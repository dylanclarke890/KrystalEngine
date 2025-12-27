#pragma once

#include "Krystal.Lib/Detection/Compiler.hpp"

namespace Krys
{
#if KRYS_COMPILER(MSVC)
  #define KRYS_FUNC_SIG __FUNCSIG__
#elif KRYS_COMPILER(GCC) || KRYS_COMPILER(CLANG)
  #define KRYS_FUNC_SIG __PRETTY_FUNCTION__
#else
  #warning "KRYS_FUNC_SIG unknown!"
  #define KRYS_FUNC_SIG "Unknown Function Signature"
#endif

#if KRYS_COMPILER(MSVC)
  #define KRYS_DISABLE_WARNING_PUSH() __pragma(warning(push))
  #define KRYS_DISABLE_WARNING_POP() __pragma(warning(pop))
  #define KRYS_DISABLE_WARNING(msvcWarningCode, gccWarningName) __pragma(warning(disable : msvcWarningCode))
#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
  #define KRYS_DISABLE_WARNING_PUSH() _Pragma("GCC diagnostic push")
  #define KRYS_DISABLE_WARNING_POP() _Pragma("GCC diagnostic pop")
  #if defined(__clang__)
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

#define NO_COPY(ClassName)                                                                                   \
  ClassName(const ClassName &) = delete;                                                                     \
  ClassName &operator=(const ClassName &) = delete;

#define NO_MOVE(ClassName)                                                                                   \
  ClassName(ClassName &&) = delete;                                                                          \
  ClassName &operator=(ClassName &&) = delete;

#define NO_COPY_MOVE(ClassName)                                                                              \
  NO_COPY(ClassName)                                                                                         \
  NO_MOVE(ClassName)

#define STATIC_CLASS(ClassName)                                                                              \
  ClassName() = delete;                                                                                      \
  ~ClassName() = delete;                                                                                     \
  NO_COPY_MOVE(ClassName)

#define MOVE_SWAP(ClassName)                                                                                 \
  ClassName(ClassName &&other) noexcept                                                                      \
  {                                                                                                          \
    Swap(other);                                                                                             \
  }                                                                                                          \
                                                                                                             \
  ClassName &operator=(ClassName &&other) noexcept                                                           \
  {                                                                                                          \
    if (this != &other)                                                                                      \
    {                                                                                                        \
      Swap(other);                                                                                           \
    }                                                                                                        \
    return *this;                                                                                            \
  }                                                                                                          \
  void swap(ClassName &other) noexcept                                                                       \
  {                                                                                                          \
    Swap(other);                                                                                             \
  }

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define KRYS_CONCAT_IMPL(x, y) x##y
#define KRYS_CONCAT(x, y) KRYS_CONCAT_IMPL(x, y)

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)
}