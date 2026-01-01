#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include <utility>

namespace Krys::Impl
{
  template <typename TVal>
  decltype(auto) ForwardIfMoveOnly(remove_ref_t<TVal> &&val) noexcept
  {
    constexpr bool IsMoveOnly = RValueRef<TVal> && MoveConstructible<TVal> && MoveAssignable<TVal>
                                 && !CopyConstructible<TVal> && !CopyAssignable<TVal>;
    if constexpr (IsMoveOnly)
    {
      return std::forward<TVal>(val);
    }
    else
    {
      return val;
    }
  }

  template <typename TVal>
  decltype(auto) ForwardIfMoveOnly(remove_ref_t<TVal> &val) noexcept
  {
    return val;
  }
}
