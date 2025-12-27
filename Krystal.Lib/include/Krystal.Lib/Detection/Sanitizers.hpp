#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"

// Check if Thread Sanitizer is enabled.
#ifdef __SANITIZE_THREAD__
  #define KRYS_TSAN_ENABLED 1
#else
  #define KRYS_TSAN_ENABLED KRYS_COMPILER_FEATURE(thread_sanitizer)
#endif

// Suppress Thread Sanitizer for the annotated function.
#if KRYS_TSAN_ENABLED && KRYS_COMPILER_ATTRIBUTE(no_sanitize_thread)
  #define KRYS_SUPPRESS_TSAN __attribute__((no_sanitize_thread))
#else
  #define KRYS_SUPPRESS_TSAN
#endif

// Check if Undefined Behavior Sanitizer is enabled.
#define KRYS_COVERAGE_ENABLED KRYS_COMPILER_FEATURE(coverage_sanitizer)

// Suppress Undefined Behavior Sanitizer for the annotated function.
#if KRYS_COVERAGE_ENABLED && KRYS_COMPILER_ATTRIBUTE(no_sanitize)
  #define KRYS_SUPPRESS_COVERAGE __attribute__((no_sanitize("coverage")))
#else
  #define KRYS_SUPPRESS_COVERAGE
#endif