#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  using FloatBoxExtent = RectEdges<float>;
  using IntBoxExtent = RectEdges<int>;
  using IntOutsets = IntBoxExtent;
  using LayoutBoxExtent = RectEdges<LayoutUnit>;
  using LayoutOptionalOutsets = RectEdges<Maybe<LayoutUnit>>;

  KRYS_NODISCARD inline LayoutBoxExtent ToLayoutBoxExtent(const IntBoxExtent &extent) noexcept
  {
    return {LayoutUnit(extent.Top()), LayoutUnit(extent.Right()), LayoutUnit(extent.Bottom()),
            LayoutUnit(extent.Left())};
  }

  KRYS_NODISCARD inline FloatBoxExtent ToFloatBoxExtent(const IntBoxExtent &extent) noexcept
  {
    return {static_cast<float>(extent.Top()), static_cast<float>(extent.Right()),
            static_cast<float>(extent.Bottom()), static_cast<float>(extent.Left())};
  }
}
