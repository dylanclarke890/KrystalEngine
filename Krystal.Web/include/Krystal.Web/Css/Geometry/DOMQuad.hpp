#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Geometry/DOMPoint.hpp"
#include "Krystal.Web/Css/Geometry/DOMRectReadOnly.hpp"

namespace krys::web
{
  class DOMRect;

  /// @see https://drafts.csswg.org/geometry-1/#dictdef-domquadinit
  struct DOMQuadInit
  {
    DOMPointInit P1;
    DOMPointInit P2;
    DOMPointInit P3;
    DOMPointInit P4;
  };

  /// @brief A quadrilateral, as four points that can each be changed.
  /// @see https://drafts.csswg.org/geometry-1/#DOMQuad
  class DOMQuad final : public RefCounted<DOMQuad>
  {
  private:
    Ref<DOMPoint> _point1;
    Ref<DOMPoint> _point2;
    Ref<DOMPoint> _point3;
    Ref<DOMPoint> _point4;

    DOMQuad(Ref<DOMPoint> point1, Ref<DOMPoint> point2, Ref<DOMPoint> point3, Ref<DOMPoint> point4) noexcept;

  public:
#pragma region DOMQuad - https://drafts.csswg.org/geometry-1/#domquad

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-domquad
    KRYS_NODISCARD static Ref<DOMQuad> Create(const DOMPointInit &p1 = {}, const DOMPointInit &p2 = {},
                                              const DOMPointInit &p3 = {},
                                              const DOMPointInit &p4 = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-fromrect
    KRYS_NODISCARD static Ref<DOMQuad> FromRect(const DOMRectInit &other = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-fromquad
    KRYS_NODISCARD static Ref<DOMQuad> FromQuad(const DOMQuadInit &other = {}) noexcept;

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-p1
    KRYS_NODISCARD DOMPoint &P1() const noexcept
    {
      return *_point1;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-p2
    KRYS_NODISCARD DOMPoint &P2() const noexcept
    {
      return *_point2;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-p3
    KRYS_NODISCARD DOMPoint &P3() const noexcept
    {
      return *_point3;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-p4
    KRYS_NODISCARD DOMPoint &P4() const noexcept
    {
      return *_point4;
    }

    /// @see https://drafts.csswg.org/geometry-1/#dom-domquad-getbounds
    KRYS_NODISCARD Ref<DOMRect> GetBounds() const noexcept;

#pragma endregion
  };
}
