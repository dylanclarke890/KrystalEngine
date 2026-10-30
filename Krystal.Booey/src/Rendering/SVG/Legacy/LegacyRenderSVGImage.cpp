#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/ImageQualityController.hpp"
#include "Krystal.Booey/Rendering/LayoutRepainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/PointerEventsHitRules.hpp"
#include "Krystal.Booey/Rendering/RenderImageResource.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResource.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGVisitedRendererTracking.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGImage::LegacyRenderSVGImage(svg::SVGImageElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGModelObject(Type::LegacySVGImage, element, krys::move(style),
                                   {SVGModelObjectFlag::UsesBoundaryCaching}),
        _imageResource(CreateUnique<RenderImageResource>())
  {
    ImageResource().Initialize(*this);
    krys_debug_assert(IsLegacyRenderSVGImage());
  }

  LegacyRenderSVGImage::~LegacyRenderSVGImage() noexcept = default;

  void LegacyRenderSVGImage::NotifyFinished(CachedResource &newImage) noexcept
  {
    if (RenderTreeBeingDestroyed())
    {
      return;
    }

    if (auto *page = ImageElement().NodeDocument().GetPage())
    {
      page->GetChromeClient().DidFinishLoadingImageForSVGImage(ImageElement());
    }

    LegacyRenderSVGModelObject::NotifyFinished(newImage);
  }

  void LegacyRenderSVGImage::WillBeDestroyed() noexcept
  {
    ImageResource().Shutdown();
    LegacyRenderSVGModelObject::WillBeDestroyed();
  }

  svg::SVGImageElement &LegacyRenderSVGImage::ImageElement() const noexcept
  {
    return Downcast<svg::SVGImageElement>(LegacyRenderSVGModelObject::GetElement());
  }

  FloatRect LegacyRenderSVGImage::CalculateObjectBoundingBox() const noexcept
  {
    LayoutSize intrinsicSize;
    if (auto *cachedImage = ImageResource().GetCachedImage())
    {
      intrinsicSize = cachedImage->ImageSizeForRenderer(nullptr, Style().UsedZoom());
    }

    auto &imageElement = ImageElement();
    svg::SVGLengthContext lengthContext(&imageElement);

    auto &style = Style();
    auto &width = style.Width();
    auto &height = style.Height();
    auto usedZoom = style.UsedZoomForLength();

    float concreteWidth;
    if (!width.IsAuto())
    {
      concreteWidth = lengthContext.ValueForLength(width, usedZoom, svg::SVGLengthMode::Width);
    }
    else if (!height.IsAuto() && !intrinsicSize.IsEmpty())
    {
      concreteWidth = lengthContext.ValueForLength(height, usedZoom, svg::SVGLengthMode::Height)
                      * intrinsicSize.Width() / intrinsicSize.Height();
    }
    else
    {
      concreteWidth = intrinsicSize.Width();
    }

    float concreteHeight;
    if (!height.IsAuto())
    {
      concreteHeight = lengthContext.ValueForLength(height, usedZoom, svg::SVGLengthMode::Height);
    }
    else if (!width.IsAuto() && !intrinsicSize.IsEmpty())
    {
      concreteHeight = lengthContext.ValueForLength(width, usedZoom, svg::SVGLengthMode::Width)
                       * intrinsicSize.Height() / intrinsicSize.Width();
    }
    else
    {
      concreteHeight = intrinsicSize.Height();
    }

    return {imageElement.X().Value(lengthContext), imageElement.Y().Value(lengthContext), concreteWidth,
            concreteHeight};
  }

  bool LegacyRenderSVGImage::UpdateImageViewport() noexcept
  {
    FloatRect oldBoundaries = _objectBoundingBox;
    _objectBoundingBox = CalculateObjectBoundingBox();

    bool updatedViewport = false;
    auto imageSourceURL =
      GetDocument().CompleteURL(ImageElement().ImageSourceURL().value_or(dom::DOMString {}));

    // Images with preserveAspectRatio=none should force non-uniform scaling. This can be achieved
    // by setting the image's container size to its intrinsic size.
    // See: http://www.w3.org/TR/SVG/single-page.html, 7.8 The ‘preserveAspectRatio’ attribute.
    if (ImageElement().PreserveAspectRatio().Align()
        == svg::SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_NONE)
    {
      if (auto *cachedImage = ImageResource().GetCachedImage())
      {
        LayoutSize intrinsicSize = cachedImage->ImageSizeForRenderer(nullptr, Style().UsedZoom());
        if (intrinsicSize != ImageResource().ImageSize(Style().UsedZoom()))
        {
          ImageResource().SetContainerContext(RoundedIntSize(intrinsicSize), imageSourceURL);
          updatedViewport = true;
        }
      }
    }

    if (oldBoundaries != _objectBoundingBox)
    {
      if (!updatedViewport)
      {
        ImageResource().SetContainerContext(EnclosingIntRect(_objectBoundingBox).Size(), imageSourceURL);
      }
      updatedViewport = true;
      _needsBoundariesUpdate = true;
    }

    return updatedViewport;
  }

  void LegacyRenderSVGImage::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    auto checkForRepaintOverride = !SelfNeedsLayout()
                                     ? LayoutRepainter::CheckForRepaint::No
                                     : SVGRenderSupport::CheckForSVGRepaintDuringLayout(*this);
    LayoutRepainter repainter(*this, checkForRepaintOverride, {}, RepaintOutlineBounds::No);
    (void)UpdateImageViewport();

    bool transformOrBoundariesUpdate = _needsTransformUpdate || _needsBoundariesUpdate;
    if (_needsTransformUpdate)
    {
      _localTransform = ImageElement().AnimatedLocalTransform();
      _needsTransformUpdate = false;
    }

    if (_needsBoundariesUpdate)
    {
      _repaintBoundingBox = _objectBoundingBox;
      SVGRenderSupport::IntersectRepaintRectWithResources(*this, _repaintBoundingBox);
      _needsBoundariesUpdate = false;
    }

    // Invalidate all resources of this client if our layout changed.
    SVGResourcesCache::ClientLayoutChanged(*this);

    // If our bounds changed, notify the parents.
    if (transformOrBoundariesUpdate)
    {
      if (auto *parent = Parent())
      {
        parent->InvalidateCachedBoundaries();
      }
    }

    repainter.RepaintAfterLayout();
    ClearNeedsLayout();
  }

  void LegacyRenderSVGImage::Paint(PaintInfo &paintInfo, const LayoutPoint &) noexcept
  {
    if (paintInfo.Context().PaintingDisabled() || paintInfo.Phase != PaintPhase::Foreground
        || Style().UsedVisibility() == Visibility::Hidden || !ImageResource().GetCachedImage())
    {
      return;
    }

    FloatRect boundingBox = RepaintRectInLocalCoordinates();
    if (!SVGRenderSupport::PaintInfoIntersectsRepaintRect(boundingBox, _localTransform, paintInfo))
    {
      return;
    }

    PaintInfo childPaintInfo(paintInfo);
    GraphicsContextStateSaver stateSaver(childPaintInfo.Context());
    childPaintInfo.ApplyTransform(_localTransform);

    if (childPaintInfo.Phase == PaintPhase::Foreground)
    {
      SVGRenderingContext renderingContext(*this, childPaintInfo);

      if (renderingContext.IsRenderingPrepared())
      {
        // FIXME: Paint the foreground buffered for buffered-rendering: static (bufferForeground).
        PaintForeground(childPaintInfo);
      }
    }

    if (style::Evaluate<float>(Style().OutlineWidth(), style::ZoomNeeded {}))
    {
      PaintOutline(childPaintInfo, LayoutRect(IntRect(boundingBox)));
    }
  }

  void LegacyRenderSVGImage::PaintForeground(PaintInfo &paintInfo) noexcept
  {
    RefPtr<Image> image = ImageResource().GetImage();
    if (!image)
    {
      return;
    }

    FloatRect destRect = _objectBoundingBox;
    FloatRect srcRect(0, 0, image->Width(), image->Height());

    ImageElement().PreserveAspectRatio().TransformRect(destRect, srcRect);

    // FIXME: The HDR content and dynamic range limit options, once there are any.
    ImagePaintingOptions options = {
      GetImageOrientation(),
      ImageQualityController::ChooseInterpolationQualityForSVG(paintInfo.Context(), *this, *image)};

    auto &context = paintInfo.Context();
    (void)context.DrawImage(*image, destRect, srcRect, options);

    // FIXME: Tell the document the element painted an image (didPaintImage), for largest contentful paint,
    // once there's one.
  }

  bool LegacyRenderSVGImage::NodeAtFloatPoint(const HitTestRequest &request, HitTestResult &result,
                                              const FloatPoint &pointInParent,
                                              HitTestAction hitTestAction) noexcept
  {
    // We only draw in the forground phase, so we only hit-test then.
    if (hitTestAction != HitTestForeground)
    {
      return false;
    }

    PointerEventsHitRules hitRules(PointerEventsHitRules::HitTestingTargetType::SVGImage, request,
                                   Style().UsedPointerEvents());
    if (VisibleToHitTesting(&request) || !hitRules.RequireVisible)
    {
      static SVGVisitedRendererTracking::VisitedSet visitedSet;

      SVGVisitedRendererTracking recursionTracking(visitedSet);
      if (recursionTracking.IsVisiting(*this))
      {
        return false;
      }

      SVGVisitedRendererTracking::Scope recursionScope(recursionTracking, *this);

      FloatPoint localPoint =
        LocalToParentTransform().Inverse().value_or(AffineTransform {}).MapPoint(pointInParent);

      if (!SVGRenderSupport::PointInClippingArea(*this, localPoint))
      {
        return false;
      }

      if (hitRules.CanHitFill)
      {
        if (_objectBoundingBox.Contains(localPoint))
        {
          UpdateHitTestResult(result, LayoutPoint(localPoint));
          if (result.AddNodeToListBasedTestResult(NodeForHitTest(), request,
                                                  HitTestLocation(FlooredLayoutPoint(localPoint)))
              == HitTestProgress::Stop)
          {
            return true;
          }
        }
      }
    }

    return false;
  }

  void LegacyRenderSVGImage::ImageChanged(WrappedImagePtr, const IntRect *) noexcept
  {
    if (!Parent())
    {
      return;
    }

    // The image resource defaults to nullImage until the resource arrives.
    // This empty image may be cached by SVG resources which must be invalidated.
    if (auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*this))
    {
      resources->RemoveClientFromCacheAndMarkForInvalidation(*this);
    }

    // Eventually notify parent resources, that we've changed.
    LegacyRenderSVGResource::MarkForLayoutAndParentResourceInvalidation(*this, false);

    // Update the SVGImageCache sizeAndScales entry in case image loading finished after layout.
    // (https://bugs.webkit.org/show_bug.cgi?id=99489)
    _objectBoundingBox = FloatRect();
    if (UpdateImageViewport())
    {
      SetNeedsLayout();
    }

    Repaint();

    // FIXME: Tell the document the element loaded an image (didLoadImage), for largest contentful paint, once
    // there's one.
  }

  void LegacyRenderSVGImage::AddFocusRingRects(List<LayoutRect> &rects, const LayoutPoint &,
                                               const RenderLayerModelObject *) const noexcept
  {
    // this is called from paint() after the localTransform has already been applied
    LayoutRect contentRect = LayoutRect(RepaintRectInLocalCoordinates());
    if (!contentRect.IsEmpty())
    {
      rects.push_back(contentRect);
    }
  }
}
