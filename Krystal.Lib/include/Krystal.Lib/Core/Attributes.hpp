#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"

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

#if !defined(KRYS_LIFETIME_BOUND)
  #if KRYS_COMPILER_CPP_ATTRIBUTE(clang::lifetimebound)
    #define KRYS_LIFETIME_BOUND [[clang::lifetimebound]]
  #elif KRYS_COMPILER_CPP_ATTRIBUTE(msvc::lifetimebound)
    #define KRYS_LIFETIME_BOUND [[msvc::lifetimebound]]
  #elif KRYS_COMPILER_CPP_ATTRIBUTE(lifetimebound)
    #define KRYS_LIFETIME_BOUND [[lifetimebound]]
  #else
    #define KRYS_LIFETIME_BOUND
  #endif
#endif

#if !defined(KRYS_ALWAYS_INLINE)
  #if KRYS_COMPILER(MSVC)
    #define KRYS_ALWAYS_INLINE __forceinline
  #elif KRYS_COMPILER(CLANG) || KRYS_COMPILER(GCC)
    #define KRYS_ALWAYS_INLINE [[gnu::always_inline]] inline
  #else
    #define KRYS_ALWAYS_INLINE
  #endif
#endif

#if !defined(KRYS_NEVER_INLINE)
  #if KRYS_COMPILER(MSVC)
    #define KRYS_NEVER_INLINE __declspec(noinline)
  #elif KRYS_COMPILER(CLANG) || KRYS_COMPILER(GCC)
    #define KRYS_NEVER_INLINE [[gnu::noinline]]
  #else
    #define KRYS_NEVER_INLINE
  #endif
#endif

#if !defined(KRYS_TRIVIAL_ABI)
  #if KRYS_COMPILER(CLANG)
    #define KRYS_TRIVIAL_ABI [[clang::trivial_abi]]
  #else
    #define KRYS_TRIVIAL_ABI
  #endif
#endif

#if !defined(KRYS_RETURNS_NONNULL)
  #if KRYS_COMPILER(CLANG)
    #define KRYS_RETURNS_NONNULL [[clang::returns_nonnull]]
  #else
    #define KRYS_RETURNS_NONNULL
  #endif
#endif

#if !defined(KRYS_UNUSED_TYPE_ALIAS)
  #define KRYS_UNUSED_TYPE_ALIAS __attribute__((unused))
#endif

#if !defined(KRYS_EMPTY_BASE_CLASS)
  #if KRYS_COMPILER(MSVC)
    // https://github.com/MicrosoftDocs/cpp-docs/blob/main/docs/cpp/empty-bases.md
    #define KRYS_EMPTY_BASE_CLASS __declspec(empty_bases)
  #else
    #define KRYS_EMPTY_BASE_CLASS
  #endif
#endif
