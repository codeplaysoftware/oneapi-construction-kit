; Copyright (C) Codeplay Software Limited. All Rights Reserved.

; RUN: %llvm-as -o %t.bc %s
; RUN: %veczc -o %t.vecz.bc %t.bc -k main -w 4
;
; This test contains no CHECK directives as success is indicated simply by
; successful vectorization through veczc. A non-zero return code indicates
; failure and so the test will fail.

; ModuleID = 'I16toF16.bc'
source_filename = "I16toF16.dxil"

target triple = "dxil-ms-dx"

%types.in = type i16
%types.out = type half

define void @main(%types.in addrspace(0)* %ptr) {
  %id = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)
  %gep = getelementptr %types.in, %types.in addrspace(0)* %ptr, i32 %id
  %x = load %types.in, %types.in addrspace(0)* %gep
  %bc = call %types.out @dx.op.bitcastI16toF16(i32 124, %types.in %x)
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32) #1

; Function Attrs: nounwind readonly
declare %types.out @dx.op.bitcastI16toF16(i32, %types.in) #0