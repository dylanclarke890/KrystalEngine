#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::svg
{
  class SVGImageElement;
}

namespace krys::boo
{
  class RenderImageResource;

  /// @brief The renderer of `<image>`: draws its image into its rectangle, fitted by `preserveAspectRatio`.
  class LegacyRenderSVGImage final : public LegacyRenderSVGModelObject
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGImage);

  private:
    bool _needsBoundariesUpdate : 1 {true};
    bool _needsTransformUpdate : 1 {true};
    AffineTransform _localTransform;
    FloatRect _objectBoundingBox;
    FloatRect _repaintBoundingBox;
    const UniquePtr<RenderImageResource> _imageResource;
    // FIXME: The foreground buffered for buffered-rendering: static (m_bufferedForeground), once there are
    // image buffers.

  public:
    LegacyRenderSVGImage(svg::SVGImageElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGImage() noexcept override;

    KRYS_NODISCARD svg::SVGImageElement &ImageElement() const noexcept;

    bool UpdateImageViewport() noexcept;
    void SetNeedsBoundariesUpdate() noexcept override
    {
      _needsBoundariesUpdate = true;
    }
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }

    KRYS_NODISCARD RenderImageResource &ImageResource() noexcept
    {
      return *_imageResource;
    }
    KRYS_NODISCARD const RenderImageResource &ImageResource() const noexcept
    {
      return *_imageResource;
    }

    // Note: Assumes the PaintInfo context has had all local transforms applied.
    void PaintForeground(PaintInfo &) noexcept;

    KRYS_NODISCARD bool IsObjectBoundingBoxValid() const noexcept
    {
      return !_objectBoundingBox.IsEmpty();
    }

  private:
    void WillBeDestroyed() noexcept override;

    void GetElement() const = delete;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGImage";
    }
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localTransform;
    }

    void NotifyFinished(CachedResource &newImage) noexcept override;

    KRYS_NODISCARD FloatRect CalculateObjectBoundingBox() const noexcept;
    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept override
    {
      return _objectBoundingBox;
    }
    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept override
    {
      return _objectBoundingBox;
    }
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept override
    {
      return _repaintBoundingBox;
    }
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept override
    {
      return _objectBoundingBox;
    }

    void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                           const RenderLayerModelObject *paintContainer = nullptr) const noexcept override;

    void ImageChanged(WrappedImagePtr, const IntRect * = nullptr) noexcept override;

    void Layout() noexcept override;
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &pointInParent,
                          HitTestAction) noexcept override;

    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept override
    {
      return _localTransform;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
