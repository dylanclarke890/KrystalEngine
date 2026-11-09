#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGraphicsElement.hpp"
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
        _filter {other._filter}, _cssFilter {krys::move(other._cssFilter)},
        _cssFilterSwitcher {krys::move(other._cssFilterSwitcher)}, _savedPaintRect {other._savedPaintRect},
        _renderingFlags {std::exchange(other._renderingFlags, {})},
        _pathClippingIsEntirelyWithinRendererContents {other._pathClippingIsEntirelyWithinRendererContents}
  {
  }

  SVGRenderingContext::~SVGRenderingContext() noexcept
  {
    // Fast path if we don't need to restore anything.
    if (!_renderingFlags.ContainsAny({RenderingFlag::RestoreGraphicsContext, RenderingFlag::EndOpacityLayer,
                                      RenderingFlag::EndFilterLayer, RenderingFlag::EndCSSFilterLayer}))
    {
      return;
    }

    krys_debug_assert(_renderer && _paintInfo);

    if (_renderingFlags.Contains(RenderingFlag::EndFilterLayer))
    {
      krys_debug_assert(_filter);
      GraphicsContext *contextPtr = &_paintInfo->Context();
      _filter->PostApplyResource(*_renderer, contextPtr, {}, nullptr, nullptr);
      _paintInfo->SetContext(*_savedContext);
      _paintInfo->Rect = _savedPaintRect;
    }

    if (_renderingFlags.Contains(RenderingFlag::EndCSSFilterLayer))
    {
      krys_debug_assert(_cssFilterSwitcher && _savedContext);
      _paintInfo->SetContext(*_savedContext);
      _paintInfo->Rect = _savedPaintRect;
      _cssFilterSwitcher->EndDrawSourceImage(*_savedContext);
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

      if (!isRenderingMask)
      {
        BeginCSSFilter();
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

    if (!isRenderingMask)
    {
      _filter = resources->Filter();
      if (_filter && !_filter->IsIdentity())
      {
        _savedContext = &_paintInfo->Context();
        _savedPaintRect = _paintInfo->Rect;
        // Return with false here may mean that we don't need to draw the content
        // (because it was either drawn before or empty) but we still need to apply the filter.
        _renderingFlags.Add(RenderingFlag::EndFilterLayer);
        GraphicsContext *contextPtr = &_paintInfo->Context();
        auto result = _filter->ApplyResource(*_renderer, style, contextPtr, {});
        _paintInfo->SetContext(*contextPtr);
        if (!ResourceWasApplied(result))
        {
          return;
        }

        // Since we're caching the resulting bitmap and do not invalidate it on repaint rect
        // changes, we need to paint the whole filter region. Otherwise, elements not visible
        // at the time of the initial paint (due to scrolling, window size, etc.) will never
        // be drawn.
        _paintInfo->Rect = LayoutRect(IntRect(_filter->DrawingRegion(*_renderer)));
      }
      else if (!_filter)
      {
        BeginCSSFilter();
      }
    }

    _renderingFlags.Add(RenderingFlag::RenderingPrepared);
  }

  IntOutsets SVGRenderingContext::CSSFilterOutsets(const RenderElement &renderer) noexcept
  {
    // The root's filter is its layer's.
    const auto &style = renderer.Style();
    if (!style.HasFilter() || style.GetFilter().IsReferenceFilter() || renderer.IsLegacyRenderSVGRoot())
    {
      return {};
    }
    auto &element = const_cast<RenderElement &>(renderer);
    return CSSFilterRenderer::CalculateOutsets(element, style.GetFilter(), renderer.ObjectBoundingBox());
  }

  void SVGRenderingContext::BeginCSSFilter() noexcept
  {
    // Filter functions on an SVG element (Filter Effects 1 applies `filter` to them all): drawn as the
    // filter resource of a `url()` is, after the mask and clip, with the object bounding box as the reference
    // box and the stroke bounding box, past which the functions draw by their outsets, as the source.
    auto &renderer = *_renderer;
    const auto &style = renderer.Style();
    if (!style.HasFilter() || style.GetFilter().IsReferenceFilter() || renderer.IsLegacyRenderSVGRoot()
        || CSSFilterRenderer::IsIdentity(renderer, style.GetFilter()))
    {
      return;
    }
    auto *page = renderer.GetDocument().GetPage();
    if (!page)
    {
      return;
    }

    auto absoluteTransform = CalculateTransformationToOutermostCoordinateSystem(renderer);
    if (!absoluteTransform.IsInvertible())
    {
      return;
    }
    FloatSize scale(static_cast<float>(absoluteTransform.XScale()),
                    static_cast<float>(absoluteTransform.YScale()));

    auto sourceRect = renderer.StrokeBoundingBox();
    auto outsets = CSSFilterOutsets(renderer);
    auto filterRegion = sourceRect;
    filterRegion.Move(static_cast<float>(-outsets.Left()), static_cast<float>(-outsets.Top()));
    filterRegion.Expand(static_cast<float>(outsets.Left() + outsets.Right()),
                        static_cast<float>(outsets.Top() + outsets.Bottom()));
    if (filterRegion.IsEmpty())
    {
      return;
    }
    (void)ImageBuffer::SizeNeedsClamping(sourceRect.Size(), scale);

    auto &context = _paintInfo->Context();
    _cssFilter = CSSFilterRenderer::Create(
      renderer, style.GetFilter(),
      {.ReferenceBox = renderer.ObjectBoundingBox(), .FilterRegion = filterRegion, .Scale = scale},
      page->PreferredFilterRenderingModes(context), context);
    if (!_cssFilter)
    {
      return;
    }
    _cssFilterSwitcher = GraphicsContextSwitcher::Create(context, sourceRect, RefPtr<Filter>(_cssFilter));
    if (!_cssFilterSwitcher)
    {
      return;
    }

    _cssFilterSwitcher->BeginDrawSourceImage(context);
    _savedContext = &context;
    _savedPaintRect = _paintInfo->Rect;
    _paintInfo->SetContext(*_cssFilterSwitcher->DrawingContext(context));
    _paintInfo->Rect = LayoutRect(EnclosingIntRect(filterRegion));
    _renderingFlags.Add(RenderingFlag::EndCSSFilterLayer);
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
