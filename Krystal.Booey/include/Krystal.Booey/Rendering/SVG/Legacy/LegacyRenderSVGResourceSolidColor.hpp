#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief The paint server of a plain colour `fill` or `stroke`.
  class LegacyRenderSVGResourceSolidColor final : public LegacyRenderSVGResource
  {
  private:
    Color _color;

  public:
    LegacyRenderSVGResourceSolidColor() noexcept = default;

    void RemoveAllClientsFromCache() noexcept override
    {
    }
    void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(bool, VisitedRenderers *) noexcept override
    {
    }
    void RemoveClientFromCache(RenderElement &) noexcept override
    {
    }
    void RemoveClientFromCacheAndMarkForInvalidation(RenderElement &, bool = true) noexcept override
    {
    }

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override;
    void PostApplyResource(RenderElement &, GraphicsContext *&, EnumSet<RenderSVGResourceMode>, const Path *,
                           const RenderElement *) noexcept override;
    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &,
                                                 RepaintRectCalculation) noexcept override
    {
      return FloatRect();
    }

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::SolidColorResourceType;
    }

    KRYS_NODISCARD const Color &GetColor() const noexcept
    {
      return _color;
    }
    void SetColor(const Color &color) noexcept
    {
      _color = color;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceSolidColor)
  KRYS_NODISCARD static bool IsType(const krys::boo::LegacyRenderSVGResource &resource) noexcept
  {
    return resource.ResourceType() == krys::boo::RenderSVGResourceType::SolidColorResourceType;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
