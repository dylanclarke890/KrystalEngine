#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"

namespace krys
{
  namespace web
  {
    class LayoutUnit;
  }

  /// @brief `LayoutUnit` is fixed point, with an infinite rect just inside its range (defined in
  /// `LayoutUnit.hpp`).
  template <>
  struct GeometryTraits<web::LayoutUnit>
  {
    constexpr static bool IsFixedPoint = true;

    KRYS_NODISCARD constexpr static web::LayoutUnit InfiniteRectMin() noexcept;

    KRYS_NODISCARD constexpr static web::LayoutUnit InfiniteRectExtent() noexcept;
  };
}

namespace krys::web
{
  using LayoutPoint = krys::Point<LayoutUnit>;
  using LayoutRect = krys::Rect<LayoutUnit>;
  using LayoutSize = krys::Size<LayoutUnit>;
}
