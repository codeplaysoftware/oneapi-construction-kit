; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %pp-llvm-ver -o %t < %s --llvm-ver %LLVMVER
; RUN: %veczc -k entry -w 2 -vecz-handle-declaration-only-calls -vecz-passes=cfg-convert,packetizer -S < %s | %filecheck %t

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

@.str.1 = private unnamed_addr addrspace(2) constant [10 x i8] c"Test %ld\0A\00", align 1
@.str.2 = private unnamed_addr addrspace(2) constant [6 x i8] c"Test\0A\00", align 1

define spir_kernel void @entry(i64* %input, i64* %output) {
entry:
  %gid = call i64 @_Z12get_local_idj(i32 0)
  %i1ptr = getelementptr i64, i64* %output, i64 %gid
  call spir_func void @_Z9mem_fencej(i32 1)
  %ii = call i64 @functionD(i64* %input)
  %ib = trunc i64 %ii to i1
  call void @functionA(i64* %i1ptr, i1 %ib)
  %i1 = load i64, i64* %i1ptr
  %i2ptr = getelementptr i64, i64* %input, i64 %gid
  %i2 = load i64, i64* %i2ptr
  %cond = icmp eq i64 %i1, %i2
  br i1 %cond, label %middle, label %end

middle:
  %ci3ptr = getelementptr i64, i64* %output, i64 %gid
  %ci3 = load i64, i64* %ci3ptr
  %fc = call i64 @functionB(i64* %ci3ptr, i64 %ci3, i32 16, i1 false)
  %call2 = call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* getelementptr inbounds ([10 x i8], [10 x i8] addrspace(2)* @.str.1, i64 0, i64 0), i64 %ci3)
  br label %end

end:
  %rr = phi i64 [42, %entry], [%fc, %middle]
  call void @functionC(i64 %rr)
  %nah = call i64 @functionB(i64* %i2ptr, i64 %rr, i32 8, i1 true)
  %call3 = call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(2)* @.str.2, i64 0, i64 0))
  ret void
}

declare void @functionA(i64*, i1)

declare i64 @functionB(i64*, i64, i32, i1)

declare void @functionC(i64)

define i64 @functionD(i64* %input) {
entry:
  %r = load i64, i64* %input
  ret i64 %r
}

declare spir_func void @_Z9mem_fencej(i32)

declare extern_weak spir_func i32 @printf(i8 addrspace(2)*, ...)

declare i64 @_Z12get_local_idj(i32)

; CHECK: define spir_kernel void @__vecz_v[[WIDTH:[0-9]+]]_entry
; CHECK: entry:
; Check that we didn't mask the get_local_id call
; CHECK: %gid = call i64 @_Z12get_local_idj(i32 0)
; Check that we didn't mask the mem_fence call
; CHECK: call spir_func void @_Z9mem_fencej(i32 1)
; Check that we instantiated functionA without a mask
; CHECK-GE15: call void @functionA(ptr {{.+}}, i1 %ib)
; CHECK-LT15: call void @functionA(i64* {{.+}}, i1 %ib)
; CHECK-GE15: call void @functionA(ptr {{.+}}, i1 %ib)
; CHECK-LT15: call void @functionA(i64* {{.+}}, i1 %ib)

; Get the condition -- Also works as a sanity check for this test
; CHECK: [[COND:%cond.*]] = icmp eq <[[WIDTH]] x i64>

; Check if we instatiated functionB with a mask
; CHECK: [[COND1:%[0-9]+]] = extractelement <[[WIDTH]] x i1> [[COND]], {{(i32|i64)}} 0
; CHECK: [[COND2:%[0-9]+]] = extractelement <[[WIDTH]] x i1> [[COND]], {{(i32|i64)}} 1
; CHECK-GE15: {{.+}} = call i64 @__vecz_b_masked_functionB(ptr {{(nonnull )?}}{{%[0-9]+}}, i64 {{%[0-9]+}}, i32 16, i1 false, i1 [[COND1]])
; CHECK-LT15: {{.+}} = call i64 @__vecz_b_masked_functionB(i64* {{(nonnull )?}}{{%[0-9]+}}, i64 {{%[0-9]+}}, i32 16, i1 false, i1 [[COND1]])
; CHECK-GE15: {{.+}} = call i64 @__vecz_b_masked_functionB(ptr {{(nonnull )?}}{{%[0-9]+}}, i64 {{%[0-9]+}}, i32 16, i1 false, i1 [[COND2]])
; CHECK-LT15: {{.+}} = call i64 @__vecz_b_masked_functionB(i64* {{(nonnull )?}}{{%[0-9]+}}, i64 {{%[0-9]+}}, i32 16, i1 false, i1 [[COND2]])
; CHECK-GE15: call spir_func i32 @__vecz_b_masked_printf_u3ptrU3AS2mb(ptr addrspace(2) @.str.1, i64 {{%[0-9]+}}, i1 [[COND1]])
; CHECK-LT15: call spir_func i32 @__vecz_b_masked_printf_PU3AS2hmb(i8 addrspace(2)* getelementptr inbounds ([10 x i8], [10 x i8] addrspace(2)* @.str.1, i64 0, i64 0), i64 {{%[0-9]+}}, i1 [[COND1]])
; CHECK-GE15: call spir_func i32 @__vecz_b_masked_printf_u3ptrU3AS2mb(ptr addrspace(2) @.str.1, i64 {{%[0-9]+}}, i1 [[COND2]])
; CHECK-LT15: call spir_func i32 @__vecz_b_masked_printf_PU3AS2hmb(i8 addrspace(2)* getelementptr inbounds ([10 x i8], [10 x i8] addrspace(2)* @.str.1, i64 0, i64 0), i64 {{%[0-9]+}}, i1 [[COND2]])

; The following checks check the generated functionB masked function
; CHECK-GE15: define private i64 @__vecz_b_masked_functionB(ptr{{( %0)?}}, i64{{( %1)?}}, i32{{( %2)?}}, i1{{( %3)?}}, i1{{( %4)?}}) {
; CHECK-LT15: define private i64 @__vecz_b_masked_functionB(i64*{{( %0)?}}, i64{{( %1)?}}, i32{{( %2)?}}, i1{{( %3)?}}, i1{{( %4)?}}) {
; CHECK: entry:
; CHECK: br i1 %4, label %active, label %exit
; CHECK: active:
; CHECK-GE15: [[RES:%[0-9]+]] = call i64 @functionB(ptr {{(nonnull )?}}%0, i64 %1, i32 %2, i1 %3)
; CHECK-LT15: [[RES:%[0-9]+]] = call i64 @functionB(i64* {{(nonnull )?}}%0, i64 %1, i32 %2, i1 %3)
; CHECK: br label %exit
; CHECK: exit:
; CHECK: [[RET:%[0-9]+]] = phi i64 [ [[RES]], %active ], [ 0, %entry ]
; CHECK: ret i64 [[RET]]

; The following checks check the generated printf masked function
; CHECK-GE15: define private spir_func i32 @__vecz_b_masked_printf_u3ptrU3AS2mb(ptr addrspace(2){{( %0)?}}, i64{{( %1)?}}, i1{{( %2)?}}) {
; CHECK-LT15: define private spir_func i32 @__vecz_b_masked_printf_PU3AS2hmb(i8 addrspace(2)*{{( %0)?}}, i64{{( %1)?}}, i1{{( %2)?}}) {
; CHECK: entry:
; CHECK: br i1 %2, label %active, label %exit
; CHECK: active:
; CHECK-GE15: [[RES:%[0-9]+]] = call spir_func i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) %0, i64 %1)
; CHECK-LT15: [[RES:%[0-9]+]] = call spir_func i32 (i8 addrspace(2)*, ...) @printf(i8 addrspace(2)* %0, i64 %1)
; CHECK: br label %exit
; CHECK: exit:
; CHECK: [[RET:%[0-9]+]] = phi i32 [ [[RES]], %active ], [ 0, %entry ]
; CHECK: ret i32 [[RET]]