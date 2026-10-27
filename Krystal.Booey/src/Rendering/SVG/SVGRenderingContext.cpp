#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsRenderingMaskImage(const RenderObject &object) noexcept
    {
      return object.View().FrameView().GetPaintBehavior().Contains(PaintBehavior::RenderingSVGClipOrMask);
    }

    KRYS_NODISCARD AffineTransform &CurrentContentTransformation() noexcept
    {
      static AffineTransform currentContentTransformation;
      return currentContentTransformation;
    }
  }

  SVGRenderingContext::SVGRenderingContext(SVGRenderingContext &&other) noexcept
      : _renderer {other._renderer}, _paintInfo {other._paintInfo}, _savedContext {other._savedContext},
        _savedPaintRect {other._savedPaintRect}, _renderingFlags {std::exchange(other._renderingFlags, {})},
        _pathClippingIsEntirelyWithinRendererContents {other._pathClippingIsEntirelyWithinRendererContents}
  {
  }

  SVGRenderingContext::~SVGRenderingContext() noexcept
  {
    // Fast path if we don't need to restore anything.
    if (!_renderingFlags.ContainsAny({RenderingFlag::RestoreGraphicsContext, RenderingFlag::EndOpacityLayer,
                                      RenderingFlag::EndFilterLayer}))
    {
      return;
    }

    krys_debug_assert(_renderer && _paintInfo);

    if (_renderingFlags.Contains(RenderingFlag::EndFilterLayer))
    {
      // FIXME: Apply the filter (LegacyRenderSVGResourceFilter::postApplyResource), with filters (B11h).
      _paintInfo->SetContext(*_savedContext);
      _paintInfo->Rect = _savedPaintRect;
    }

    if (_renderingFlags.Contains(RenderingFlag::EndOpacityLayer))
    {
      _paintInfo->Context().EndTransparencyLayer();
    }

    if (_renderingFlags.Contains(RenderingFlag::RestoreGraphicsContext))
    {
      _paintInfo->Context().Restore();
    }
  }

  void
    SVGRenderingContext::PrepareToRenderSVGContent(RenderElement &renderer, PaintInfo &paintInfo,
                                                   NeedsGraphicsContextSave needsGraphicsContextSave) noexcept
  {
#ifndef NDEBUG
    // This function must not be called twice!
    krys_debug_assert(!_renderingFlags.Contains(RenderingFlag::PrepareToRenderSVGContentWasCalled));
    _renderingFlags.Add(RenderingFlag::PrepareToRenderSVGContentWasCalled);
#endif

    _renderer = &renderer;
    _paintInfo = &paintInfo;

    // We need to save / restore the context even if the initialization failed.
    if (needsGraphicsContextSave == NeedsGraphicsContextSave::SaveGraphicsContext)
    {
      _paintInfo->Context().Save();
      _renderingFlags.Add(RenderingFlag::RestoreGraphicsContext);
    }

    auto &style = _renderer->Style();

    // Setup transparency layers before setting up SVG resources!
    bool isRenderingMask = IsRenderingMaskImage(*_renderer);
    // RenderLayer takes care of root opacity.
    float opacity =
      (renderer.IsLegacyRenderSVGRoot() || isRenderingMask) ? 1 : style.GetOpacity().Value.Value;
    bool hasBlendMode = style.HasBlendMode();
    bool hasIsolation = style.HasIsolation();
    bool isolateMaskForBlending = false;

    if (style.HasPositionedMask())
    {
      if (auto *graphicsElement =
            DynamicDowncast<svg::SVGGraphicsElement>(Downcast<svg::SVGElement>(*renderer.GetElement())))
      {
        isolateMaskForBlending = graphicsElement->ShouldIsolateBlending();
      }
    }

    if (opacity < 1 || hasBlendMode || isolateMaskForBlending || hasIsolation)
    {
      FloatRect repaintRect = _renderer->RepaintRectInLocalCoordinates();
      _paintInfo->Context().Clip(repaintRect);

      if (opacity < 1 || hasBlendMode || isolateMaskForBlending || hasIsolation)
      {
        if (hasBlendMode)
        {
          _paintInfo->Context().SetCompositeOperation(_paintInfo->Context().CompositeOperation(),
                                                      style.GetBlendMode());
        }

        _paintInfo->Context().BeginTransparencyLayer(opacity);

        if (hasBlendMode)
        {
          _paintInfo->Context().SetCompositeOperation(_paintInfo->Context().CompositeOperation(),
                                                      BlendMode::Normal);
        }

        _renderingFlags.Add(RenderingFlag::EndOpacityLayer);
      }
    }

    bool hasSimpleClip = style.GetClipPath().TryBasicShape() || style.GetClipPath().TryBox();
    if (hasSimpleClip && !Is<LegacyRenderSVGRoot>(renderer))
    {
      SVGRenderSupport::ClipContextToCSSClippingArea(_paintInfo->Context(), renderer);
    }

    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*_renderer);

    if (!resources)
    {
      if (style.GetFilter().IsReferenceFilter())
      {
        return;
      }

      _renderingFlags.Add(RenderingFlag::RenderingPrepared);
      return;
    }

    if (!isRenderingMask)
    {
      if (auto *masker = resources->Masker())
      {
        GraphicsContext *contextPtr = &_paintInfo->Context();
        auto result = masker->ApplyResource(*_renderer, style, contextPtr, {});
        _paintInfo->SetContext(*contextPtr);
        if (!ResourceWasApplied(result))
        {
          return;
        }
      }
    }

    auto *clipper = resources->Clipper();
    if (!hasSimpleClip && clipper && !Is<LegacyRenderSVGRoot>(renderer))
    {
      GraphicsContext *contextPtr = &_paintInfo->Context();
      auto result = clipper->ApplyResource(*_renderer, style, contextPtr, {});
      _paintInfo->SetContext(*contextPtr);
      if (!ResourceWasApplied(result))
      {
        return;
      }

      _pathClippingIsEntirelyWithinRendererContents =
        result.Contains(LegacyRenderSVGResource::ApplyResult::ClipContainsRendererContent);
    }

    // FIXME: The filter (LegacyRenderSVGResourceFilter::applyResource and the filter's drawing region), with
    // filters (B11h).
    if (!isRenderingMask && style.GetFilter().IsReferenceFilter())
    {
      return;
    }

    _renderingFlags.Add(RenderingFlag::RenderingPrepared);
  }

  void SVGRenderingContext::ClipToImageBuffer(GraphicsContext &context, const FloatRect &targetRect,
                                              const FloatSize &scale, RefPtr<ImageBuffer> &imageBuffer,
                                              bool safeToClear) noexcept
  {
    if (!imageBuffer)
    {
      return;
    }

    AffineTransform absoluteTransform;
    absoluteTransform.Scale(scale.Width(), scale.Height());

    auto absoluteTargetRect = CalculateImageBufferRect(targetRect, absoluteTransform);

    // The mask image has been created in the absolute coordinate space, as the image should not be scaled.
    // So the actual masking process has to be done in the absolute coordinate space as well.
    context.ConcatCTM(absoluteTransform.Inverse().value_or(AffineTransform {}));
    context.ClipToImageBuffer(*imageBuffer, FloatRect(absoluteTargetRect));
    context.ConcatCTM(absoluteTransform);

    // When nesting resources, with objectBoundingBox as content unit types, there's no use in caching the
    // resulting image buffer as the parent resource already caches the result.
    if (safeToClear && !CurrentContentTransformation().IsIdentity())
    {
      imageBuffer = nullptr;
    }
  }

  float SVGRenderingContext::CalculateScreenFontSizeScalingFactor(const RenderObject &renderer) noexcept
  {
    AffineTransform ctm = CalculateTransformationToOutermostCoordinateSystem(renderer);
    return static_cast<float>(std::hypot(ctm.XScale(), ctm.YScale()) / std::numbers::sqrt2);
  }

  AffineTransform SVGRenderingContext::CalculateTransformationToOutermostCoordinateSystem(
    const RenderObject &renderer) noexcept
  {
    AffineTransform absoluteTransform = CurrentContentTransformation();

    float deviceScaleFactor = renderer.GetDocument().DeviceScaleFactor();
    // Walk up the render tree, accumulating SVG transforms.
    const RenderObject *ancestor = &renderer;
    while (ancestor)
    {
      absoluteTransform = ancestor->LocalToParentTransform() * absoluteTransform;
      if (ancestor->IsLegacyRenderSVGRoot())
      {
        break;
      }
      ancestor = ancestor->Parent();
    }

    // Continue walking up the layer tree, accumulating CSS transforms.
    RenderLayer *layer = ancestor ? ancestor->EnclosingLayer() : nullptr;
    while (layer)
    {
      if (TransformationMatrix *layerTransform = layer->Transform())
      {
        absoluteTransform = layerTransform->ToAffineTransform() * absoluteTransform;
      }

      // We can stop at compositing layers, to match the backing resolution.
      if (layer->IsComposited())
      {
        break;
      }

      layer = layer->Parent();
    }

    absoluteTransform.Scale(deviceScaleFactor);
    return absoluteTransform;
  }

  void
    SVGRenderingContext::RenderSubtreeToContext(GraphicsContext &context, RenderElement &item,
                                                const AffineTransform &subtreeContentTransformation) noexcept
  {
    // Rendering into a buffer implies we're being used for masking, clipping, patterns or filters. In each of
    // these cases we don't want to paint the selection.
    PaintInfo info(context, LayoutRect::InfiniteRect(), PaintPhase::Foreground,
                   PaintBehavior::SkipSelectionHighlight);

    AffineTransform &contentTransformation = CurrentContentTransformation();
    AffineTransform savedContentTransformation = contentTransformation;
    contentTransformation = subtreeContentTransformation * contentTransformation;

    krys_debug_assert(!item.NeedsLayout());
    item.Paint(info, {});

    contentTransformation = savedContentTransformation;
  }
}
