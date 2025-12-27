#pragma once

/// @brief Check if a particular compiler is being used.
#define KRYS_COMPILER(COMPILER) (defined KRYS_COMPILER_##COMPILER && KRYS_COMPILER_##COMPILER)

/// @brief Check if the current compiler supports the given feature.
#define KRYS_COMPILER_SUPPORTS(FEAT) (defined KRYS_COMPILER_SUPPORTS_##FEAT && KRYS_COMPILER_SUPPORTS_##FEAT)

/// @brief Check if the current compiler requires a given quirk.
#define KRYS_COMPILER_QUIRK(QUIRK) (defined KRYS_COMPILER_QUIRK_##QUIRK && KRYS_COMPILER_QUIRK_##QUIRK)

/// @brief Check if the current compiler supports a particular language (C|CPP).
#define KRYS_COMPILER_LANG(LANG) (defined KRYS_COMPILER_LANG_##LANG && KRYS_COMPILER_LANG_##LANG)

#ifdef __has_attribute
  /// @brief Check if the current compiler supports a particular attribute.
  #define KRYS_COMPILER_ATTRIBUTE(x) __has_attribute(x)
#else
  #define KRYS_COMPILER_ATTRIBUTE(x) 0
#endif

#ifdef __has_cpp_attribute
  /// @brief Check if the current compiler supports a particular C++ attribute.
  #define KRYS_COMPILER_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
  #define KRYS_COMPILER_ATTRIBUTE(x) 0
#endif

#ifdef __has_builtin
  /// @brief Check if the current compiler supports a particular builtin.
  #define KRYS_COMPILER_BUILTIN(x) __has_builtin(x))
#else
  #define KRYS_COMPILER_BUILTIN(x) 0
#endif

#ifdef __has_feature
  /// @brief Check if the current compiler supports a particular language or library feature.
  #define KRYS_COMPILER_FEATURE(x) __has_feature(x))
#else
  #define KRYS_COMPILER_FEATURE(x) 0
#endif

#ifdef __has_declspec_attribute
  /// @brief Check if the current compiler supports a Microsoft style __declspec attribute.
  #define KRYS_COMPILER_DECLSPEC(x) __has_declspec_attribute(x))
#else
  #define KRYS_COMPILER_DECLSPEC(x) 0
#endif

#ifdef __cplusplus
  #define KRYS_COMPILER_LANG_C 0
  #define KRYS_COMPILER_LANG_CPP 1
  #define KRYS_EXTERN_C_BEGIN                                                                                \
    extern "C"                                                                                               \
    {
  #define KRYS_EXTERN_C_END }
#else
  #define KRYS_COMPILER_LANG_C 1
  #define KRYS_COMPILER_LANG_CPP 0
  #define KRYS_EXTERN_C_BEGIN
  #define KRYS_EXTERN_C_END
#endif

/// KRYS_COMPILER(CLANG) - Clang/LLVM
#if defined(__clang__)
  #define KRYS_COMPILER_CLANG 1
#endif

/// KRYS_COMPILER(GCC_COMPATIBLE) - GCC or GCC-compatible compiler
#if defined(__GNUC__)
  #define KRYS_COMPILER_GCC_COMPATIBLE 1
#endif

/// KRYS_COMPILER(GCC) - GNU Compiler Collection (GCC)
#if KRYS_COMPILER(GCC_COMPATIBLE) && !KRYS_COMPILER(CLANG)
  #define KRYS_COMPILER_GCC 1
#endif

/// KRYS_COMPILER(MSVC) - Microsoft Visual C++
#if defined(_MSC_VER)
  #define KRYS_COMPILER_MSVC 1
#endif

/// KRYS_COMPILER_SUPPORTS(EABI) - Supports the Embedded ABI
#if defined(__ARM_EABI__) || defined(__EABI__)
  #define KRYS_COMPILER_SUPPORTS_EABI 1
#endif

/// KRYS_COMPILER_QUIRK(CONSIDERS_UNREACHABLE_CODE) - Code after certain constructs is considered unreachable
#if !KRYS_COMPILER(CLANG)
  #define KRYS_COMPILER_QUIRK_CONSIDERS_UNREACHABLE_CODE 1
#endif

#pragma region Sanitizers

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

#pragma endregion

#pragma region Attributes

/// @brief Mark a parameter as unused to suppress compiler warnings.
#define KRYS_UNUSED_PARAM(x) (void)(x)

#if !defined(KRYS_NODISCARD) && KRYS_COMPILER_CPP_ATTRIBUTE(nodiscard)
  /// @brief Indicate that the return value of a function should not be discarded.
  #define KRYS_NODISCARD [[nodiscard]]
#elif !defined(KRYS_NODISCARD)
  #define KRYS_NODISCARD
#endif

#if !defined(KRYS_NORETURN) && KRYS_COMPILER_CPP_ATTRIBUTE(noreturn)
  /// @brief Indicate that a function does not return to its caller.
  #define KRYS_NORETURN [[noreturn]]
#elif !defined(KRYS_NORETURN)
  #define KRYS_NORETURN
#endif

#if !defined(KRYS_FALLTHROUGH) && KRYS_COMPILER_CPP_ATTRIBUTE(fallthrough)
  /// @brief Indicate that a case in a switch statement intentionally falls through to the next case.
  #define KRYS_FALLTHROUGH [[fallthrough]]
#elif !defined(KRYS_FALLTHROUGH)
  #define KRYS_FALLTHROUGH
#endif

#if !defined(KRYS_LIKELY) && KRYS_COMPILER_CPP_ATTRIBUTE(likely)
  /// @brief Indicate that a condition is likely to be true.
  #define KRYS_LIKELY [[likely]]
#elif !defined(KRYS_LIKELY)
  #define KRYS_LIKELY
#endif

#if !defined(KRYS_UNLIKELY) && KRYS_COMPILER_CPP_ATTRIBUTE(unlikely)
  /// @brief Indicate that a condition is unlikely to be true.
  #define KRYS_UNLIKELY [[unlikely]]
#elif !defined(KRYS_UNLIKELY)
  #define KRYS_UNLIKELY
#endif

#if !defined(KRYS_MAYBE_UNUSED) && KRYS_COMPILER_CPP_ATTRIBUTE(maybe_unused)
  /// @brief Indicate that a variable, function, or parameter may be unused.
  #define KRYS_MAYBE_UNUSED [[maybe_unused]]
#elif !defined(KRYS_MAYBE_UNUSED)
  #define KRYS_MAYBE_UNUSED
#endif

#if !defined(KRYS_DEPRECATED) && KRYS_COMPILER_CPP_ATTRIBUTE(deprecated)
  /// @brief Mark a function or variable as deprecated with an optional message.
  #define KRYS_DEPRECATED(msg) [[deprecated(msg)]]
#elif !defined(KRYS_DEPRECATED)
  #define KRYS_DEPRECATED(msg)
#endif

#if !defined(KRYS_NO_UNIQUE_ADDRESS) && KRYS_COMPILER_CPP_ATTRIBUTE(no_unique_address)
  /// @brief Indicate that a non-static data member does not need to have a unique address.
  #define KRYS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#elif !defined(KRYS_NO_UNIQUE_ADDRESS)
  #define KRYS_NO_UNIQUE_ADDRESS
#endif

#pragma endregion
