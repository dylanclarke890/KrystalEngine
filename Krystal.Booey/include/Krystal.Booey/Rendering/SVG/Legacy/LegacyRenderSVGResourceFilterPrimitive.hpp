#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"

namespace krys::boo::svg
{
  class SVGFilterPrimitiveStandardAttributes;
}

namespace krys::boo
{
  class FilterEffect;

  /// @brief The renderer of a filter primitive: it holds the primitive's style, and tells its filter when the
  /// primitive changes.
  class LegacyRenderSVGResourceFilterPrimitive final : public LegacyRenderSVGHiddenContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceFilterPrimitive);

  public:
    LegacyRenderSVGResourceFilterPrimitive(svg::SVGFilterPrimitiveStandardAttributes &,
                                           RenderStyle &&) noexcept;

    KRYS_NODISCARD svg::SVGFilterPrimitiveStandardAttributes &FilterPrimitiveElement() const noexcept;

    void StyleDidChange(style::Difference, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceFilterPrimitive";
    }

    void MarkFilterEffectForRepaint(FilterEffect *) noexcept;
    void MarkFilterEffectForRebuild() noexcept;

  private:
    void GetElement() const = delete;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceFilterPrimitive)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceFilterPrimitive();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
