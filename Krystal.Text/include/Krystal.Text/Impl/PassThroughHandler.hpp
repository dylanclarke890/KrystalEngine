#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"

namespace Krys::Impl
{
  template <bool AssumeValidity = false>
  class PassThroughHandlerWith
  {
  public:
    /// @brief A type that is true when calling code can not call this function and ignore it, and false
    /// when it cannot ignore it. AssumeValidHandler for details.
    using assumeTValid = IntegralConstant<bool, AssumeValidity>;

    /// @brief A handler for either decode or encode results that simply passes the result type back
    /// through with no changes made.
    /// @param[in] result The current state of the encode operation to pass through.
    template <typename TEncoding, typename TResult, typename TInputProgress, typename TOutputProgress>
    constexpr auto operator()(const TEncoding &, TResult result, const TInputProgress &,
                              const TOutputProgress &) const
    {
      return result;
    }
  };

  using PassThroughHandler = PassThroughHandlerWith<false>;
}
