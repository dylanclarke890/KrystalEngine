#pragma once

#include <version>

/// @brief Check if a particular compiler is being used.
#define KRYS_COMPILER(COMPILER) (defined KRYS_COMPILER_##COMPILER && KRYS_COMPILER_##COMPILER)

/// @brief Check if the current compiler supports the given feature.
#define KRYS_COMPILER_SUPPORTS(FEAT) (defined KRYS_COMPILER_SUPPORTS_##FEAT && KRYS_COMPILER_SUPPORTS_##FEAT)

/// @brief Check if the current compiler requires a given quirk.
#define KRYS_COMPILER_QUIRK(QUIRK) (defined KRYS_COMPILER_QUIRK_##QUIRK && KRYS_COMPILER_QUIRK_##QUIRK)

/// @brief Check if the current compiler supports a particular language (C|CPP).
#define KRYS_COMPILER_LANGUAGE(LANG) (defined KRYS_COMPILER_LANGUAGE_##LANG && KRYS_COMPILER_LANGUAGE_##LANG)

#define KRYS_COMPILER_STL(STL) (defined KRYS_COMPILER_STL_##STL && KRYS_COMPILER_STL_##STL)

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

#ifdef __has_include
  /// @brief Check if the current compiler supports a particular include file.
  #define KRYS_COMPILER_INCLUDE(x) __has_include(x))
#else
  #define KRYS_COMPILER_INCLUDE(x) 0
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
  #define KRYS_EXTERN_C extern "C"
  #define KRYS_EXTERN_C_BEGIN                                                                                \
    KRYS_EXTERN_C                                                                                            \
    {
  #define KRYS_EXTERN_C_END }
#else
  #define KRYS_COMPILER_LANG_C 1
  #define KRYS_COMPILER_LANG_CPP 0
  #define KRYS_EXTERN_C
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

/// KRYS_COMPILER_STL(MSVC) - Using the Microsoft STL implementation
#if defined(_MSVC_STL_VERSION)
  #define KRYS_COMPILER_STL_MSVC 1
#endif

/// KRYS_COMPILER_STL(GCC) - Using the GNU STL implementation
#if defined(__GLIBCXX__)
  #define KRYS_COMPILER_STL_GCC 1
#endif

/// KRYS_COMPILER_STL(CLANG) - Using the LLVM STL implementation
#if defined(_LIBCPP_VERSION)
  #define KRYS_COMPILER_STL_CLANG 1
#endif

/// KRYS_FUNCTION_SIGNATURE - Macro that expands to the current function signature as a string
#if KRYS_COMPILER(MSVC)
  #define KRYS_FUNCTION_SIGNATURE __FUNCSIG__
#elif KRYS_COMPILER(GCC) || KRYS_COMPILER(CLANG)
  #define KRYS_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#else
  #warning "KRYS_FUNCTION_SIGNATURE unknown!"
  #define KRYS_FUNCTION_SIGNATURE "Unknown Function Signature"
#endif

/// KRYS_COMPILER_PRAGMA - Macro to emit a compiler pragma
#if KRYS_COMPILER(MSVC)
  #define KRYS_COMPILER_PRAGMA(PRAGMA) __pragma(PRAGMA)
#elif KRYS_COMPILER(GCC)
  #define KRYS_COMPILER_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#else
  #warning "KRYS_FUNCTION_SIGNATURE unknown!"
  #define KRYS_COMPILER_PRAGMA "Unknown Function Signature"
#endif

/// @brief Macros to disable compiler warnings for a section of code.
#if KRYS_COMPILER(MSVC)
  #define KRYS_DISABLE_WARNING_PUSH() KRYS_COMPILER_PRAGMA(warning(push))
  #define KRYS_DISABLE_WARNING_POP() KRYS_COMPILER_PRAGMA(warning(pop))
  #define KRYS_DISABLE_WARNING(WARNING_CODE) KRYS_COMPILER_PRAGMA(warning(disable : WARNING_CODE))
#elif KRYS_COMPILER(GCC)
  #define KRYS_DISABLE_WARNING_PUSH() KRYS_COMPILER_PRAGMA(GCC diagnostic push)
  #define KRYS_DISABLE_WARNING_POP() KRYS_COMPILER_PRAGMA(GCC diagnostic pop)
  #define KRYS_DISABLE_WARNING(WARNING_NAME) _Pragma(GCC diagnostic ignored #WARNING_NAME)
#elif KRYS_COMPILER(CLANG)
  #define KRYS_DISABLE_WARNING_PUSH() KRYS_COMPILER_PRAGMA(clang diagnostic push)
  #define KRYS_DISABLE_WARNING_POP() KRYS_COMPILER_PRAGMA(clang diagnostic pop)
  #define KRYS_DISABLE_WARNING(WARNING_NAME) _Pragma(clang diagnostic ignored #WARNING_NAME)
#else
  #warning "Compiler not supported for warning suppression macros."
  #define KRYS_DISABLE_WARNING_PUSH()
  #define KRYS_DISABLE_WARNING_POP()
  #define KRYS_DISABLE_WARNING(WARNING)
#endif

// Now we define specific warning disable macros for common warnings.
#if KRYS_COMPILER(MSVC)
  #define KRYS_DISABLE_WARNING_UNUSED_VARIABLE() KRYS_DISABLE_WARNING(4'101)
  #define KRYS_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER() KRYS_DISABLE_WARNING(4'100)
  #define KRYS_DISABLE_WARNING_SIGN_CONVERSION() KRYS_DISABLE_WARNING(4'244)
  #define KRYS_DISABLE_WARNING_DEPRECATED_DECLARATIONS() KRYS_DISABLE_WARNING(4'996)
  #define KRYS_DISABLE_WARNING_FALLTHROUGH() KRYS_DISABLE_WARNING(4'762)
  #define KRYS_DISABLE_WARNING_USE_AFTER_FREE() // not applicable in MSVC
  #define KRYS_DISABLE_WARNING_SHADOWED_VARIABLE() KRYS_DISABLE_WARNING(44'456)
#elif KRYS_COMPILER(GCC) || KRYS_COMPILER(CLANG) // clang-format off
  #define KRYS_DISABLE_WARNING_UNUSED_VARIABLE() KRYS_DISABLE_WARNING(-Wunused-variable)
  #define KRYS_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER() KRYS_DISABLE_WARNING(-Wunused-parameter)
  #define KRYS_DISABLE_WARNING_SIGN_CONVERSION() KRYS_DISABLE_WARNING(-Wsign-conversion)
  #define KRYS_DISABLE_WARNING_DEPRECATED_DECLARATIONS() KRYS_DISABLE_WARNING(-Wdeprecated-declarations)
  #define KRYS_DISABLE_WARNING_FALLTHROUGH() KRYS_DISABLE_WARNING(-Wimplicit-fallthrough)
  #define KRYS_DISABLE_WARNING_USE_AFTER_FREE() KRYS_DISABLE_WARNING(-Wuse-after-free)
  #define KRYS_DISABLE_WARNING_SHADOWED_VARIABLE() KRYS_DISABLE_WARNING(-Wshadow)
#else // clang-format on
  #define KRYS_DISABLE_WARNING_UNUSED_VARIABLE()
  #define KRYS_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER()
  #define KRYS_DISABLE_WARNING_SIGN_CONVERSION()
  #define KRYS_DISABLE_WARNING_DEPRECATED_DECLARATIONS()
  #define KRYS_DISABLE_WARNING_FALLTHROUGH()
  #define KRYS_DISABLE_WARNING_USE_AFTER_FREE()
  #define KRYS_DISABLE_WARNING_SHADOWED_VARIABLE()
#endif
