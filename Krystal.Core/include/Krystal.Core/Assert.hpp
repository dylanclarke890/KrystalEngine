#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Debug.hpp"

#include <stacktrace>
#include <string_view>
#include <utility>
#if !defined(KRYS_DISABLE_ASSERTS)
  #include <cstdlib>
  #include <format>
  #include <print>
#endif

namespace krys
{
  using AssertionFailHandler = void (*)(std::string_view expression, const std::stacktrace &stackTrace,
                                        std::string_view msg) noexcept;

  class AssertionHandler
  {
    static void DefaultHandler(std::string_view expression, const std::stacktrace &stackTrace,
                               std::string_view msg) noexcept
    {
#if !defined(KRYS_DISABLE_ASSERTS)
      std::print(stderr, "Assertion failed({}) - {}:\n{}", expression, msg, stackTrace);
      KRYS_DEBUG_BREAK();
      std::abort();
#endif
    }

    static inline AssertionFailHandler _handler = DefaultHandler;

  public:
    static void SetHandler(AssertionFailHandler handler) noexcept
    {
      _handler = handler;
    }

    KRYS_NODISCARD static AssertionFailHandler GetHandler() noexcept
    {
      return _handler;
    }

    static bool CallHandler(std::string_view expression, const std::stacktrace &stackTrace,
                            std::string_view msg = {}) noexcept
    {
      if (_handler)
      {
        _handler(expression, stackTrace, msg);
      }

      return false;
    }
  };

#if defined(KRYS_DISABLE_ASSERTS)
  #define krys_assert(cond) ((void)0)
  #define krys_assert_msg(cond, msg) ((void)0)
#else
  #define krys_assert(cond)                                                                                  \
    ((cond) || ::krys::AssertionHandler::CallHandler(#cond, std::stacktrace::current()))

  #define krys_assert_msg(cond, msg)                                                                         \
    ((cond) || ::krys::AssertionHandler::CallHandler(#cond, std::stacktrace::current(), msg))
#endif

#if KRYS_DEBUG
  #define krys_debug_assert(cond) krys_assert(cond)
  #define krys_debug_assert_msg(cond, msg) krys_assert_msg(cond, msg)
#else
  #define krys_debug_assert(cond) ((void)0)
  #define krys_debug_assert_msg(cond, msg) ((void)0)
#endif

#define krys_unreachable()                                                                                   \
  krys_assert_msg(false, "Unreachable code reached");                                                        \
  std::unreachable()

#define krys_not_implemented()                                                                               \
  krys_assert_msg(false, "Not implemented");                                                                 \
  std::unreachable()
}