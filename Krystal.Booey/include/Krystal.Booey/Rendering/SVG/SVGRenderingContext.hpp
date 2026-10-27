#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class ImageBuffer;
  class LegacyRenderSVGResourceFilter;
  class RenderElement;
  class RenderObject;
  struct PaintInfo;

  /// @brief The steps `SVGRenderingContext` has taken, so that its destructor can undo them.
  enum class SVGRenderingContextFlag : uint8
  {
    RenderingPrepared,
    RestoreGraphicsContext,
    EndOpacityLayer,
    EndFilterLayer,
    PrepareToRenderSVGContentWasCalled
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::SVGRenderingContextFlag, 5uz);

namespace krys::boo
{
  /// @brief Applies an SVG renderer's opacity, blending, clipping, masking and filters around its painting.
  class SVGRenderingContext
  {
    KRYS_NON_COPYABLE(SVGRenderingContext);
    using RenderingFlag = SVGRenderingContextFlag;

  public:
    enum class NeedsGraphicsContextSave : bool
    {
      SaveGraphicsContext,
      DontSaveGraphicsContext
    };

  private:
    RenderElement *_renderer {nullptr};
    PaintInfo *_paintInfo {nullptr};
    GraphicsContext *_savedContext {nullptr};
    LegacyRenderSVGResourceFilter *_filter {nullptr};
    LayoutRect _savedPaintRect;
    // To properly revert partially successful initializtions in the destructor, we record all successful
    // steps.
    EnumSet<RenderingFlag> _renderingFlags;
    // True with path-based clipping is known to contrain the clipped area to within the renderer; used to
    // optimize away a context clip.
    bool _pathClippingIsEntirelyWithinRendererContents {false};

  public:
    // Does not start rendering.
    SVGRenderingContext() noexcept = default;

    SVGRenderingContext(SVGRenderingContext &&) noexcept;

    SVGRenderingContext(RenderElement &object, PaintInfo &paintinfo,
                        NeedsGraphicsContextSave needsGraphicsContextSave =
                          NeedsGraphicsContextSave::DontSaveGraphicsContext) noexcept
    {
      PrepareToRenderSVGContent(object, paintinfo, needsGraphicsContextSave);
    }

    // Automatically finishes context rendering.
    ~SVGRenderingContext() noexcept;

    // Used by all SVG renderers who apply clip/filter/etc. resources to the renderer content.
    void PrepareToRenderSVGContent(
      RenderElement &, PaintInfo &,
      NeedsGraphicsContextSave = NeedsGraphicsContextSave::DontSaveGraphicsContext) noexcept;
    KRYS_NODISCARD bool IsRenderingPrepared() const noexcept
    {
      return _renderingFlags.Contains(RenderingFlag::RenderingPrepared);
    }

    KRYS_NODISCARD bool PathClippingIsEntirelyWithinRendererContents() const noexcept
    {
      return _pathClippingIsEntirelyWithinRendererContents;
    }

    static void RenderSubtreeToContext(GraphicsContext &, RenderElement &, const AffineTransform &) noexcept;
    static void ClipToImageBuffer(GraphicsContext &, const FloatRect &targetRect, const FloatSize &scale,
                                  RefPtr<ImageBuffer> &, bool safeToClear) noexcept;

    KRYS_NODISCARD static float CalculateScreenFontSizeScalingFactor(const RenderObject &) noexcept;
    KRYS_NODISCARD static AffineTransform
      CalculateTransformationToOutermostCoordinateSystem(const RenderObject &) noexcept;

    KRYS_NODISCARD static IntRect CalculateImageBufferRect(const FloatRect &targetRect,
                                                           const AffineTransform &absoluteTransform) noexcept
    {
      return EnclosingIntRect(absoluteTransform.MapRect(targetRect));
    }

    // FIXME: Support for the buffered-rendering hint (bufferForeground), once image buffers can be drawn
    // back from the CPU.

    KRYS_NODISCARD const RenderElement *Renderer() const noexcept
    {
      return _renderer;
    }
  };
}
