#pragma once

#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/SVG/SVGTextPositioningElement.hpp"

namespace krys::boo
{
  /// @brief The renderer of a `<tspan>`.
  class RenderSVGTSpan final : public RenderSVGInline
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGTSpan);

  public:
    RenderSVGTSpan(svg::SVGTextPositioningElement &element, RenderStyle &&style) noexcept
        : RenderSVGInline(Type::SVGTSpan, element, krys::move(style))
    {
      krys_debug_assert(IsRenderSVGTSpan());
    }

  private:
    void GraphicsElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGTSpan";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGTSpan)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGTSpan();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
