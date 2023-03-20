// Copyright (C) Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Tracer profiling implementation.
///
/// @copyright Copyright (C) Codeplay Software Limited. All Rights Reserved.

#ifndef TRACER_H_INCLUDED
#define TRACER_H_INCLUDED

#include <cstdint>

namespace tracer {
/// @addtogroup tracer
/// @{

/// @brief recordTrace records an event.
/// @param name usually the function name, or event you wish to record.
/// @param cat the category of the trace (eg Microseconds).
/// @param start the start timestamp (eg Microseconds).
/// @param end the end timestamp.
///
/// This function is the real meat of tracer - it'll record a trace event to the
/// .trace file for this execution of the process. The trace produced is
/// viewable via chrome's tracing mode:
/// * open chrome and go to chrome://tracing
/// * open the tracing file
///   * on Windows these are located at
///   `%APPDATA%\\.ComputeAortaTracer\\*.trace`
///   * on Linux these are located at `$HOME/.ComputeAortaTracer/*.trace`
/// * enjoy the tracing information produced!
void recordTrace(const char* name, const char* cat, uint64_t start,
                 uint64_t end);

/// @return Returns the current time stamp in Microseconds.
uint64_t getCurrentTimestamp();

// Tracer configs defined here so we don't need various defines to be
// defined in just to be able to use the functionality.

#ifndef CA_TRACE_CL
#define CA_TRACE_CL 0
#endif

#ifndef CA_TRACE_CORE
#define CA_TRACE_CORE 0
#endif

#ifndef CA_TRACE_MUX
#define CA_TRACE_MUX 0
#endif

#ifndef CA_TRACE_IMPLEMENTATION
#define CA_TRACE_IMPLEMENTATION 0
#endif

/// @brief Benchmark base class for the Bench object.
template<bool enable>
struct BenchmarkCategory {
  constexpr static bool enabled = enable;
};

template<class T> inline const char *getCategoryName();

/// @brief Helper to generate the types and category names
#define TRACER_GUARD_CATEGORY(type, enabled)                             \
  struct type : public BenchmarkCategory<static_cast<bool>(enabled)> {}; \
  template <>                                                            \
  inline const char* getCategoryName<type>() {                           \
    return #type;                                                        \
  }

TRACER_GUARD_CATEGORY(OpenCL, CA_TRACE_CL);
TRACER_GUARD_CATEGORY(Core, CA_TRACE_CORE);
TRACER_GUARD_CATEGORY(Mux, CA_TRACE_MUX);
TRACER_GUARD_CATEGORY(Impl, CA_TRACE_IMPLEMENTATION);

#undef TRACER_GUARD_CATEGORY

/// @brief A scoped timer. Construct the TracerGuard object with one of the
/// category types. eg: tracer::TraceGuard<OpenCL>("function");
template <typename Category>
struct TraceGuard {
  TraceGuard(const char *name) : trace_name(nullptr), start_time(0) {
    if (Category::enabled) {
      trace_name = name;
      start_time = getCurrentTimestamp();
    }
  };

  ~TraceGuard() {
    if (Category::enabled) {
      uint64_t end_time = getCurrentTimestamp();
      const char *cat_name = getCategoryName<Category>();
      recordTrace(trace_name, cat_name, start_time, end_time);
    }
  }

  const char *trace_name;
  uint64_t start_time;
};

/// @}
}  // tracer

#endif  // TRACER_H_INCLUDED