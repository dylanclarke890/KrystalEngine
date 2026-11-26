/*
 * Copyright (C) 2006 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Svg/Graphics/SVGImage.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Graphics/GraphicsContext.hpp"
#include "Krystal.Web/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Web/Loader/DocumentLoader.hpp"
#include "Krystal.Web/Loader/DocumentWriter.hpp"
#include "Krystal.Web/Loader/FrameLoader.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/FrameViewLayoutContext.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Web/Svg/Elements/SVGForeignObjectElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGSvgElement.hpp"
#include "Krystal.Web/Svg/Graphics/SVGImageClients.hpp"

namespace krys::web
{
  SVGImage::SVGImage(ImageObserver *observer) noexcept
      : Image(observer), _startAnimationTimer(*this, &SVGImage::StartAnimationTimerFired)
  {
  }

  SVGImage::~SVGImage() noexcept
  {
    // Clear _page, so that SVGImageWebClient knows we're destructed.
    _frame = nullptr;
  }

  RefPtr<SVGSvgElement> SVGImage::RootElement() const noexcept
  {
    if (!_frame)
    {
      return nullptr;
    }

    auto *document = _frame->Document();
    if (!document)
    {
      return nullptr;
    }

    auto documentElement = document->DocumentElement();
    auto *svgElement = DynamicDowncast<SVGElement>(documentElement.get());
    return svgElement ? ShareRefPtr(DynamicDowncast<SVGSvgElement>(*svgElement)) : nullptr;
  }

  bool SVGImage::RenderingTaintsOrigin() const noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement)
    {
      return false;
    }

    for (auto *node = dom::TreeQueries::Next(*rootElement, rootElement.get()); node;
         node = dom::TreeQueries::Next(*node, rootElement.get()))
    {
      auto *element = DynamicDowncast<SVGElement>(*node);
      if (element && Is<SVGForeignObjectElement>(*element))
      {
        return true;
      }
    }

    // Because SVG image rendering disallows external resources and links,
    // these images effectively are restricted to a single security origin.
    return false;
  }

  void SVGImage::SetContainerSize(const SizeF &size) noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement || !rootElement->Renderer() || !rootElement->Renderer()->IsLegacyRenderSVGRoot())
    {
      return;
    }

    RefPtr<web::FrameView> view = ShareRefPtr(FrameView());
    view->Resize(ContainerSize());

    Downcast<LegacyRenderSVGRoot>(*rootElement->Renderer()).SetContainerSize(SizeI(size));
  }

  SizeI SVGImage::ContainerSize() const noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement || !rootElement->Renderer() || !rootElement->Renderer()->IsLegacyRenderSVGRoot())
    {
      return {};
    }

    // If a container size is available it has precedence.
    auto containerSize = Downcast<LegacyRenderSVGRoot>(*rootElement->Renderer()).ContainerSize();
    if (!containerSize.IsEmpty())
    {
      return containerSize;
    }

    // Assure that a container size is always given for a non-identity zoom level.
    krys_debug_assert(rootElement->Renderer()->Style().UsedZoom() == 1);

    SizeF currentSize;
    if (rootElement->HasIntrinsicWidth() && rootElement->HasIntrinsicHeight())
    {
      currentSize = rootElement->CurrentViewportSizeExcludingZoom();
    }
    else
    {
      currentSize = rootElement->CurrentViewBoxRect().Size();
    }

    // Use the default CSS intrinsic size if the above failed.
    if (currentSize.IsEmpty())
    {
      return SizeI(300, 150);
    }

    return SizeI(currentSize);
  }

  ImageDrawResult SVGImage::DrawForContainer(GraphicsContext &context, SizeF containerSize,
                                             float containerZoom, const net::Url &initialFragmentUrl,
                                             const RectF &dstRect, const RectF &srcRect,
                                             ImagePaintingOptions options) noexcept
  {
    if (!_frame)
    {
      return ImageDrawResult::DidNothing;
    }

    RefPtr<ImageObserver> observer = GetImageObserver();

    // Temporarily reset image observer, we don't want to receive any ChangedInRect() calls due to this
    // relayout.
    SetImageObserver(nullptr);

    SizeI roundedContainerSize = RoundedSizeI(containerSize);
    SetContainerSize(SizeF(roundedContainerSize));

    RectF scaledSrc = srcRect;
    scaledSrc.Scale(1 / containerZoom);

    // Compensate for the container size rounding by adjusting the source rect.
    SizeF adjustedSrcSize = scaledSrc.Size();
    adjustedSrcSize.Scale(static_cast<float>(roundedContainerSize.Width()) / containerSize.Width(),
                          static_cast<float>(roundedContainerSize.Height()) / containerSize.Height());
    scaledSrc.SetSize(adjustedSrcSize);

    RefPtr<web::FrameView> view = ShareRefPtr(FrameView());
    (void)view->ScrollToFragment(initialFragmentUrl);

    ImageDrawResult result = Draw(context, dstRect, scaledSrc, options);

    SetImageObserver(krys::move(observer));
    return result;
  }

  void SVGImage::DrawPatternForContainer(GraphicsContext &context, const SizeF &containerSize,
                                         float containerZoom, const net::Url &initialFragmentUrl,
                                         const RectF &srcRect, const AffineTransform &patternTransform,
                                         const PointF &phase, const SizeF &spacing, const RectF &dstRect,
                                         ImagePaintingOptions options) noexcept
  {
    RectF zoomedContainerRect = RectF(PointF(), containerSize);
    zoomedContainerRect.Scale(containerZoom);

    // The ImageBuffer size needs to be scaled to match the final resolution.
    AffineTransform transform = context.GetCTM();
    SizeF imageBufferScale =
      SizeF(static_cast<float>(transform.XScale()), static_cast<float>(transform.YScale()));
    krys_debug_assert(imageBufferScale.Width());
    krys_debug_assert(imageBufferScale.Height());

    RectF imageBufferSize = zoomedContainerRect;
    imageBufferSize.Scale(imageBufferScale.Width(), imageBufferScale.Height());

    RefPtr<ImageBuffer> buffer = context.CreateImageBuffer(SizeF(ExpandedSizeI(imageBufferSize.Size())));
    if (!buffer)
    {
      return;
    }

    DrawForContainer(buffer->Context(), containerSize, containerZoom, initialFragmentUrl, imageBufferSize,
                     zoomedContainerRect);
    if (context.DrawLuminanceMask())
    {
      buffer->ConvertToLuminanceMask();
    }

    // Adjust the source rect and transform due to the image buffer's scaling.
    RectF scaledSrcRect = srcRect;
    scaledSrcRect.Scale(imageBufferScale.Width(), imageBufferScale.Height());
    AffineTransform unscaledPatternTransform(patternTransform);
    unscaledPatternTransform.Scale(1 / imageBufferScale.Width(), 1 / imageBufferScale.Height());

    context.SetDrawLuminanceMask(false);
    context.DrawPattern(*buffer, dstRect, scaledSrcRect, unscaledPatternTransform, phase, spacing, options);
  }

  ImageDrawResult SVGImage::Draw(GraphicsContext &context, const RectF &destination, const RectF &source,
                                 ImagePaintingOptions options) noexcept
  {
    if (!_frame)
    {
      return ImageDrawResult::DidNothing;
    }

    RefPtr<web::FrameView> view = ShareRefPtr(FrameView());
    krys_debug_assert(view);

    GraphicsContextStateSaver stateSaver(context);
    context.SetCompositeOperation(options.GetCompositeOperator(), options.GetBlendMode());
    context.Clip(RectF(EnclosingRectI(destination)));

    float alpha = context.Alpha();
    bool compositingRequiresTransparencyLayer =
      options.GetCompositeOperator() != CompositeOperator::SourceOver
      || options.GetBlendMode() != BlendMode::Normal || alpha < 1;
    if (compositingRequiresTransparencyLayer)
    {
      context.BeginTransparencyLayer(alpha);
      context.SetCompositeOperation(CompositeOperator::SourceOver, BlendMode::Normal);
    }

    auto orientation = options.Orientation();
    // SVG images don't have intrinsic orientation metadata like EXIF, so FromImage defaults to None.
    if (orientation == img::ImageOrientation::Orientation::FromImage)
    {
      orientation = img::ImageOrientation::Orientation::None;
    }

    SizeF scale(destination.Width() / source.Width(), destination.Height() / source.Height());

    // We can only draw the entire frame, clipped to the rect we want. So compute where the top left
    // of the image would be if we were drawing without clipping, and translate accordingly.
    SizeF topLeftOffset(source.Location().X() * scale.Width(), source.Location().Y() * scale.Height());
    PointF destOffset = destination.Location() - topLeftOffset;

    context.Translate(destOffset);
    context.Scale(scale);

    // Apply orientation transformation if needed.
    if (orientation != img::ImageOrientation::Orientation::None)
    {
      auto containerSizeForTransform = ContainerSize();
      auto orientationTransform =
        img::ImageOrientation(orientation).TransformFromDefault(SizeF(containerSizeForTransform));
      context.ConcatCTM(orientationTransform);
    }

    view->Resize(ContainerSize());

    if (view->LayoutContext().NeedsLayout())
    {
      view->LayoutContext().Layout();
    }

    view->Paint(context, Intersection(context.ClipBounds(), EnclosingRectI(source)));

    if (compositingRequiresTransparencyLayer)
    {
      context.EndTransparencyLayer();
    }

    stateSaver.Restore();

    if (RefPtr<ImageObserver> observer = GetImageObserver())
    {
      observer->DidDraw(*this);
    }

    return ImageDrawResult::DidDraw;
  }

  RenderBox *SVGImage::EmbeddedContentBox() const noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement)
    {
      return nullptr;
    }
    return DynamicDowncast<RenderBox>(rootElement->Renderer());
  }

  web::FrameView *SVGImage::FrameView() const noexcept
  {
    if (!_frame)
    {
      return nullptr;
    }

    return _frame->View();
  }

  bool SVGImage::HasRelativeWidth() const noexcept
  {
    // FIXME: This seems wrong.
    return false;
  }

  bool SVGImage::HasRelativeHeight() const noexcept
  {
    // FIXME: This seems wrong.
    return false;
  }

  void SVGImage::ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                            SizeF &intrinsicRatio) noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement)
    {
      return;
    }

    intrinsicWidth = rootElement->IntrinsicWidth();
    intrinsicHeight = rootElement->IntrinsicHeight();

    if (rootElement->PreserveAspectRatio().Align()
        == SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_NONE)
    {
      return;
    }

    intrinsicRatio = rootElement->ViewBox().Size();
    if (intrinsicRatio.IsEmpty())
    {
      intrinsicRatio = SizeF {intrinsicWidth, intrinsicHeight};
    }
  }

  void SVGImage::StartAnimationTimerFired() noexcept
  {
    StartAnimation();
  }

  void SVGImage::ScheduleStartAnimation() noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement || !rootElement->AnimationsPaused())
    {
      return;
    }
    _startAnimationTimer.StartOneShot({});
  }

  void SVGImage::StartAnimation() noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement || !rootElement->AnimationsPaused())
    {
      return;
    }
    rootElement->UnpauseAnimations();
    rootElement->SetCurrentTime(0);
  }

  void SVGImage::ResumeAnimation() noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement || !rootElement->AnimationsPaused())
    {
      return;
    }
    rootElement->UnpauseAnimations();
  }

  void SVGImage::StopAnimation() noexcept
  {
    _startAnimationTimer.Stop();
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement)
    {
      return;
    }
    rootElement->PauseAnimations();
  }

  void SVGImage::ResetAnimation() noexcept
  {
    StopAnimation();
  }

  bool SVGImage::IsAnimating() const noexcept
  {
    RefPtr<SVGSvgElement> rootElement = RootElement();
    if (!rootElement)
    {
      return false;
    }
    return rootElement->HasActiveAnimation();
  }

  img::EncodedDataStatus SVGImage::DataChanged(bool allDataReceived) noexcept
  {
    // Don't do anything; it is an empty image.
    if (!Data()->Size())
    {
      return img::EncodedDataStatus::Complete;
    }

    if (allDataReceived)
    {
      // FIXME: If this SVG ends up loading itself, we might leak the world.
      // The Cache code does not know about CachedImages holding Frames and
      // won't know to break the cycle.
      // This will become an issue when SVGImage will be able to load other
      // SVGImage objects, but we're safe now, because SVGImage can only be
      // loaded by a top-level document.
      _frame = Frame::Create(CreateUnique<SVGImageWebClient>(this));

      // The image's document takes the settings of the document that loaded it, and the appearance of what
      // shows it.
      auto observer = GetImageObserver();
      RefPtr<Settings> settings = observer ? ShareRefPtr(observer->GetSettings()) : nullptr;

      auto &frame = *_frame;
      frame.CreateView({});
      if (observer && observer->UseSystemDarkAppearance())
      {
        _frame->SetUseDarkAppearance(true);
      }
      auto &loader = frame.Loader();
      loader.Init();

      RefPtr<web::FrameView> frameView = ShareRefPtr(frame.View());
      // SVG Images will always synthesize a viewBox, if it's not available, and thus never see scrollbars.
      frameView->SetScrollbarModes(ScrollbarMode::AlwaysOff, ScrollbarMode::AlwaysOff);
      frameView->SetTransparent(true); // SVG Images are transparent.

      RefPtr<DocumentLoader> activeDocumentLoader = ShareRefPtr(loader.GetDocumentLoader());
      krys_debug_assert(activeDocumentLoader); // The DocumentLoader should have been created by Init().
      activeDocumentLoader->Writer().SetMimeType(u8"image/svg+xml");
      if (activeDocumentLoader->Writer().Begin(net::Url())) // create the empty document
      {
        if (auto *document = frame.Document(); document && settings)
        {
          Ref<Settings> documentSettings = settings;
          document->SetSettings(krys::move(documentSettings));
        }
        activeDocumentLoader->Writer().AddData(Data()->GetSpan());
        activeDocumentLoader->Writer().End();
      }

      if (auto *document = frame.Document())
      {
        document->UpdateLayoutIgnorePendingStylesheets();
      }

      // Set the intrinsic size before a container size is available.
      _intrinsicSize = SizeF(ContainerSize());
    }

    return _frame ? img::EncodedDataStatus::Complete : img::EncodedDataStatus::Unknown;
  }

  utf8_string SVGImage::FilenameExtension() const noexcept
  {
    return u8"svg";
  }

  bool svg::IsInSVGImage(const Element *element) noexcept
  {
    krys_debug_assert(element);

    auto *frame = element->NodeDocument().Frame();
    if (!frame)
    {
      return false;
    }

    return frame->GetWebClient().IsSVGImageWebClient();
  }
}
