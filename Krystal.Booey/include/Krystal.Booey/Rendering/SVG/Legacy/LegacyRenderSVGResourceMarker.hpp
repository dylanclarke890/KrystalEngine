#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::svg
{
  class SVGMarkerElement;
  enum class SVGMarkerUnitsType : uint8;
}

namespace krys::boo
{
  /// @brief The renderer of a `<marker>`: draws its children at the vertices of the shapes that reference it.
  class LegacyRenderSVGResourceMarker final : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceMarker);

  private:
    mutable AffineTransform _localToParentTransform;
    FloatRect _viewport;

  public:
    LegacyRenderSVGResourceMarker(svg::SVGMarkerElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceMarker() noexcept override;

    KRYS_NODISCARD svg::SVGMarkerElement &MarkerElement() const noexcept;

    void RemoveAllClientsFromCache() noexcept override
    {
    }
    void RemoveClientFromCache(RenderElement &) noexcept override
    {
    }

    void Draw(PaintInfo &, const AffineTransform &) noexcept;

    // Calculates marker boundaries, mapped to the target element's coordinate space
    KRYS_NODISCARD FloatRect MarkerBoundaries(RepaintRectCalculation,
                                              const AffineTransform &markerTransformation) const noexcept;

    void ApplyViewportClip(PaintInfo &) noexcept override;
    void Layout() noexcept override;
    void CalcViewport() noexcept override;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override;
    KRYS_NODISCARD AffineTransform MarkerTransformation(const FloatPoint &origin, float angle,
                                                        float strokeWidth) const noexcept;

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override
    {
      return {};
    }
    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &,
                                                 RepaintRectCalculation) noexcept override
    {
      return FloatRect();
    }

    KRYS_NODISCARD FloatPoint ReferencePoint() const noexcept;
    KRYS_NODISCARD Maybe<float> Angle() const noexcept;
    KRYS_NODISCARD svg::SVGMarkerUnitsType MarkerUnits() const noexcept;

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::MarkerResourceType;
    }

  private:
    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceMarker";
    }

    // Generates a transformation matrix usable to render marker content. Handles scaling the marker content
    // acording to SVGs markerUnits="strokeWidth" concept, when a strokeWidth value != -1 is passed in.
    KRYS_NODISCARD AffineTransform MarkerContentTransformation(const AffineTransform &contentTransformation,
                                                               const FloatPoint &origin,
                                                               float strokeWidth = -1) const noexcept;

    KRYS_NODISCARD AffineTransform ViewportTransform() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceMarker)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceMarker();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
