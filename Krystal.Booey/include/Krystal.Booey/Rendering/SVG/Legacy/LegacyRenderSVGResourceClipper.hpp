#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <array>

namespace krys::boo::svg
{
  class SVGClipPathElement;
}

namespace krys::boo
{
  class ImageBuffer;

  /// @brief The mask a clip path drew for one client, and what it was drawn for.
  struct ClipperData
  {
    struct Inputs
    {
      KRYS_NODISCARD bool operator==(const Inputs &other) const noexcept = default;

      FloatRect ObjectBoundingBox;
      FloatRect ClippedContentBounds;
      FloatSize Scale;
      float UsedZoom = 1;
      bool PaintingDisabled {false};
    };

    bool Invalidate(const Inputs &inputs) noexcept;

    RefPtr<boo::ImageBuffer> ImageBuffer;
    Inputs LastInputs;
  };

  /// @brief The renderer of a `<clipPath>`: clips its clients to one path when it can, or else to a mask of
  /// its children.
  class LegacyRenderSVGResourceClipper final : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceClipper);

  private:
    std::array<FloatRect, 2> _clipBoundaries;
    HashMap<const RenderObject *, UniquePtr<ClipperData>> _clipperMap;

  public:
    LegacyRenderSVGResourceClipper(svg::SVGClipPathElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGResourceClipper() noexcept override;

    KRYS_NODISCARD svg::SVGClipPathElement &ClipPathElement() const noexcept;

    void RemoveAllClientsFromCache() noexcept override;
    void RemoveClientFromCache(RenderElement &) noexcept override;

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept override;
    // clipPath can be clipped too, but don't have a boundingBox or repaintRect. So we can't call
    // applyResource directly and use the rects from the object, since they are empty for RenderSVGResources
    // FIXME: We made applyClippingToContext public because we cannot call applyResource on HTML elements (it
    // asserts on RenderObject::objectBoundingBox)
    // objectBoundingBox ia used to compute clip path geometry when clipPathUnits="objectBoundingBox".
    // clippedContentBounds is the bounds of the content to which clipping is being applied.
    EnumSet<ApplyResult> ApplyClippingToContext(GraphicsContext &, RenderElement &,
                                                const FloatRect &objectBoundingBox,
                                                const FloatRect &clippedContentBounds,
                                                float usedZoom = 1) noexcept;
    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &,
                                                 RepaintRectCalculation) noexcept override;

    KRYS_NODISCARD RenderSVGResourceType ResourceType() const noexcept override
    {
      return RenderSVGResourceType::ClipperResourceType;
    }

    KRYS_NODISCARD bool HitTestClipContent(const FloatRect &, const FloatPoint &) noexcept;

    KRYS_NODISCARD svg::SVGUnitTypes::SVGUnitType ClipPathUnits() const noexcept;

  private:
    KRYS_NODISCARD bool SelfNeedsClientInvalidation() const noexcept override
    {
      return (EverHadLayout() || !_clipperMap.empty()) && SelfNeedsLayout();
    }

    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGResourceClipper";
    }

    KRYS_NODISCARD ClipperData::Inputs ComputeInputs(const GraphicsContext &, const RenderElement &,
                                                     const FloatRect &objectBoundingBox,
                                                     const FloatRect &clippedContentBounds,
                                                     float usedZoom) noexcept;
    KRYS_NODISCARD EnumSet<ApplyResult> PathOnlyClipping(GraphicsContext &, const RenderElement &,
                                                         const AffineTransform &, const FloatRect &,
                                                         float usedZoom) noexcept;
    KRYS_NODISCARD bool DrawContentIntoMaskImage(ImageBuffer &, const FloatRect &objectBoundingBox,
                                                 float usedZoom) noexcept;
    void CalculateClipContentRepaintRect(RepaintRectCalculation) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGResourceClipper)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGResourceClipper();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
