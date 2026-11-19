#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"

namespace krys
{
  namespace boo
  {
    class LayoutUnit;
  }

  /// @brief `LayoutUnit` is fixed point, with an infinite rect just inside its range (defined in
  /// `LayoutUnit.hpp`).
  template <>
  struct GeometryTraits<boo::LayoutUnit>
  {
    constexpr static bool IsFixedPoint = true;

    KRYS_NODISCARD constexpr static boo::LayoutUnit InfiniteRectMin() noexcept;

    KRYS_NODISCARD constexpr static boo::LayoutUnit InfiniteRectExtent() noexcept;
  };
}

namespace krys::boo
{
  using LayoutPoint = Point<LayoutUnit>;
  using LayoutRect = Rect<LayoutUnit>;
  using LayoutSize = Size<LayoutUnit>;
}
