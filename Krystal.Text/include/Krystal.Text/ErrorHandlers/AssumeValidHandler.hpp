#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"

#if KRYS_ENV(DEV)
  #include <cstdlib>
#endif
#include <cassert>

namespace Krys::Handlers
{
  /// @brief An error handler that tells an encoding that it will pass through any errors, without doing any
  /// adjustment, correction or checking.
  /// @remarks Can invoke undefined behavior if the calling code does not ensure that all inputs are valid for
  /// the encoding in question.
  class AssumeValidHandler
  {
    /// @brief If true, triggers an assert when this handler is invoked.
    constexpr static bool TrapsOnInvocation =
#if KRYS_ENV(DEV)
      false;
#else
      true;
#endif

  public:
    /// @brief A type that is true when calling code can not call this function and ignore it, and false when
    /// it cannot ignore it.
    using assumeTValid = IntegralConstant<bool, TrapsOnInvocation>;

    /// @brief A handler for either decode or encode results that simply passes the result type back through
    /// with no changes made.
    /// @param[in] result The current state of the encode operation to pass through.
    template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
    KRYS_NODISCARD constexpr auto operator()(const TEncoding &, TResult result, const TInputProgress &,
                                             const TOutputProgress &) const noexcept(!TrapsOnInvocation)
    {
      if constexpr (TrapsOnInvocation)
      {
        assert(
          false
          && "You have invoked the AssumeValidHandler, and tripped undefined behavior. This means you "
             "violated the covenant between you, the compiler, this library, and every piece of code that "
             "depends on you. It is highly suggested that AssumeValidHandler is not used except for the most "
             "extremely secure cases. If you cannot properly grok your threat model or what may or may not "
             "be external data sources, please do not use this type!");
      }

      return result;
    }
  };

  /// @brief An instance of AssumeValidHandler type for ease of use.
  constexpr inline AssumeValidHandler AssumeValid = {};
}
