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

#if KRYS_COMPILER(MSVC)
  #define KRYS_FUNCTION_SIGNATURE __FUNCSIG__
#elif KRYS_COMPILER(GCC) || KRYS_COMPILER(CLANG)
  #define KRYS_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#else
  #warning "KRYS_FUNCTION_SIGNATURE unknown!"
  #define KRYS_FUNCTION_SIGNATURE "Unknown Function Signature"
#endif

#if KRYS_COMPILER(MSVC)
  #define KRYS_DISABLE_WARNING_PUSH() __pragma(warning(push))
  #define KRYS_DISABLE_WARNING_POP() __pragma(warning(pop))
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName) __pragma(warning(disable : msvcWarningCode))
#elif KRYS_COMPILER(GCC) || KRYS_COMPILER(CLANG) // GCC or Clang
  #define KRYS_DISABLE_WARNING_PUSH() _Pragma("GCC diagnostic push")
  #define KRYS_DISABLE_WARNING_POP() _Pragma("GCC diagnostic pop")
  #if KRYS_COMPILER(CLANG)
    #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)                                            \
      _Pragma("clang diagnostic ignored \"" gccWarningName "\"")
  #else
    #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)                                            \
      _Pragma("GCC diagnostic ignored \"" gccWarningName "\"")
  #endif
#else
  #warning "Compiler not supported for warning suppression macros."
  #define KRYS_DISABLE_WARNING_PUSH()
  #define KRYS_DISABLE_WARNING_POP()
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName)
#endif