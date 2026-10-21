#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/ImagePaintingOptions.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderImageResourceStyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/TextPainter.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    // If we'll be displaying either alt text or an image, add some padding.
    constexpr uint16 PaddingWidth = 4;
    constexpr uint16 PaddingHeight = 4;

    // Alt text is restricted to this maximum size, in pixels. These are
    // signed integers because they are compared with other signed values.
    constexpr float MaxAltTextWidth = 1'024;
    constexpr int32 MaxAltTextHeight = 256;

    KRYS_NODISCARD UniquePtr<RenderImageResource> CreateImageResource(StyleImage *styleImage) noexcept
    {
      if (styleImage)
      {
        return CreateUnique<RenderImageResourceStyleImage>(*styleImage);
      }
      return CreateUnique<RenderImageResource>();
    }
  }

  RenderImage::RenderImage(Type type, dom::Element &element, RenderStyle &&style, StyleImage *styleImage,
                           float imageDevicePixelRatio) noexcept
      : RenderReplaced(type, element, krys::move(style), LayoutSize(), ReplacedFlag::IsImage),
        _imageResource(CreateImageResource(styleImage)), _imageDevicePixelRatio(imageDevicePixelRatio)
  {
    UpdateAltText();
  }

  RenderImage::RenderImage(Type type, dom::Document &document, RenderStyle &&style,
                           StyleImage *styleImage) noexcept
      : RenderReplaced(type, document, krys::move(style), LayoutSize(), ReplacedFlag::IsImage),
        _imageResource(CreateImageResource(styleImage))
  {
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderImage::~RenderImage() noexcept = default;

  void RenderImage::WillBeDestroyed() noexcept
  {
    ImageResource().Shutdown();
    RenderReplaced::WillBeDestroyed();
  }

  IntSize RenderImage::ImageSizeForError(CachedImage *newImage) const noexcept
  {
    krys_debug_assert(newImage);
    krys_debug_assert(newImage->ImageForRenderer(this));

    FloatSize imageSize;
    if (newImage->WillPaintBrokenImage())
    {
      auto [brokenImage, imageScaleFactor] = newImage->BrokenImage(GetDocument().DeviceScaleFactor());
      imageSize = brokenImage->Size();
      imageSize.Scale(1 / imageScaleFactor);
    }
    else
    {
      imageSize = newImage->ImageForRenderer(this)->Size();
    }

    // imageSize() returns 0 for the error image. We need the true size of the
    // error image, so we have to get it by grabbing image() directly.
    return IntSize(static_cast<int32>(PaddingWidth + imageSize.Width() * Style().UsedZoom()),
                   static_cast<int32>(PaddingHeight + imageSize.Height() * Style().UsedZoom()));
  }

  // Sets the image height and width to fit the alt text. Returns true if the
  // image size changed.
  ImageSizeChangeType RenderImage::SetImageSizeForAltText(CachedImage *newImage) noexcept
  {
    IntSize imageSize;
    if (newImage && newImage->ImageForRenderer(this))
    {
      imageSize = ImageSizeForError(newImage);
    }
    else if (!_altText.empty() || newImage)
    {
      // If we'll be displaying either text or an image, add a little padding.
      imageSize = IntSize(PaddingWidth, PaddingHeight);
    }

    // we have an alt and the user meant it (its not a text we invented)
    if (!_altText.empty())
    {
      const FontCascade &font = Style().GetFontCascade();
      IntSize paddedTextSize(
        PaddingWidth
          + static_cast<int32>(std::min(
            std::ceil(font.Width(RenderBlock::ConstructTextRun(_altText, Style()))), MaxAltTextWidth)),
        PaddingHeight + std::min(font.MetricsOfPrimaryFont().IntHeight(), MaxAltTextHeight));
      imageSize = imageSize.ExpandedTo(paddedTextSize);
    }

    if (LayoutSize(imageSize) == IntrinsicSize())
    {
      return ImageSizeChangeType::None;
    }

    SetIntrinsicSize(LayoutSize(imageSize));
    return ImageSizeChangeType::ForAltText;
  }

  void RenderImage::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    if (!HasInitializedStyle())
    {
      ImageResource().Initialize(*this);
    }
    RenderReplaced::StyleWillChange(diff, newStyle);
  }

  void RenderImage::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderReplaced::StyleDidChange(diff, oldStyle);
    if (_needsToSetSizeForAltText)
    {
      if (!_altText.empty() && SetImageSizeForAltText(GetCachedImage()) != ImageSizeChangeType::None)
      {
        RepaintOrMarkForLayout(ImageSizeChangeType::ForAltText);
      }
      _needsToSetSizeForAltText = false;
    }

    if (oldStyle && diff == style::DifferenceResult::Layout)
    {
      if (oldStyle->GetImageOrientation() != Style().GetImageOrientation())
      {
        RepaintOrMarkForLayout(ImageSizeChangeType::None);
        return;
      }
    }
  }

  bool RenderImage::ShouldCollapseToEmpty() const noexcept
  {
    auto imageRepresentsNothing = [&]
    {
      if (!GetElement()->HasAttribute(u8"alt"))
      {
        return false;
      }
      return ImageResource().ErrorOccurred() && _altText.empty();
    };
    if (!GetElement())
    {
      // Images with no associated elements do not fall under the category of unwanted content.
      return false;
    }
    if (!IsInline())
    {
      return false;
    }
    if (!imageRepresentsNothing())
    {
      return false;
    }
    return GetDocument().InNoQuirksMode()
           || (Style().LogicalWidth().IsAuto() && Style().LogicalHeight().IsAuto());
  }

  LayoutUnit
    RenderImage::ComputeReplacedLogicalWidth(ShouldComputePreferred shouldComputePreferred) const noexcept
  {
    if (ShouldCollapseToEmpty())
    {
      return {};
    }
    return RenderReplaced::ComputeReplacedLogicalWidth(shouldComputePreferred);
  }

  LayoutUnit RenderImage::ComputeReplacedLogicalHeight(Maybe<LayoutUnit> estimatedUsedWidth) const noexcept
  {
    if (ShouldCollapseToEmpty())
    {
      return {};
    }
    return RenderReplaced::ComputeReplacedLogicalHeight(estimatedUsedWidth);
  }

  void RenderImage::ImageChanged(WrappedImagePtr newImage, const IntRect *rect) noexcept
  {
    if (RenderTreeBeingDestroyed())
    {
      return;
    }

    if (HasVisibleBoxDecorations() || HasMask() || HasShapeOutside())
    {
      RenderReplaced::ImageChanged(newImage, rect);
    }

    if (ShouldCollapseToEmpty())
    {
      // Image might need resizing when we are at the final state.
      SetNeedsLayout();
    }

    if (newImage != ImageResource().ImagePtr() || !newImage)
    {
      return;
    }

    // FIXME: Count visually non-empty pixels (incrementVisuallyNonEmptyPixelCountIfNeeded), once there's
    // paint milestone tracking.

    ImageSizeChangeType imageSizeChange = ImageSizeChangeType::None;

    // Set image dimensions, taking into account the size of the alt text.
    if (ImageResource().ErrorOccurred())
    {
      if (!_altText.empty() && GetDocument().HasPendingStyleRecalc())
      {
        krys_debug_assert(GetElement());
        if (auto *element = GetElement())
        {
          _needsToSetSizeForAltText = true;
          element->InvalidateStyle();
        }
        return;
      }
      imageSizeChange = SetImageSizeForAltText(GetCachedImage());
    }
    RepaintOrMarkForLayout(imageSizeChange, rect);
    // FIXME: Tell the accessibility cache and didLoadImage (for largest contentful paint), once they exist.
  }

  void RenderImage::UpdateIntrinsicSizeIfNeeded(const LayoutSize &newSize) noexcept
  {
    if (ImageResource().ErrorOccurred() || !_imageResource->GetCachedImage())
    {
      return;
    }
    SetIntrinsicSize(newSize);
  }

  void RenderImage::UpdateInnerContentRect() noexcept
  {
    // Propagate container size to image resource.
    IntSize containerSize(ReplacedContentRect().Size());
    if (!containerSize.IsEmpty())
    {
      url::URL imageSourceURL;
      if (auto *imageElement = DynamicDowncast<html::HTMLImageElement>(GetElement()))
      {
        imageSourceURL = imageElement->CurrentURL();
      }
      ImageResource().SetContainerContext(containerSize, imageSourceURL);
    }
  }

  void RenderImage::RepaintOrMarkForLayout(ImageSizeChangeType imageSizeChange, const IntRect *rect) noexcept
  {
    LayoutSize newIntrinsicSize = ImageResource().IntrinsicSize(Style().UsedZoom());
    LayoutSize oldIntrinsicSize = IntrinsicSize();

    UpdateIntrinsicSizeIfNeeded(newIntrinsicSize);

    // In the case of generated image content using :before/:after/content, we might not be
    // in the render tree yet. In that case, we just need to update our intrinsic size.
    // layout() will be called after we are inserted in the tree which will take care of
    // what we are doing here.
    if (!ContainingBlock())
    {
      return;
    }

    bool imageSourceHasChangedSize =
      oldIntrinsicSize != newIntrinsicSize || imageSizeChange != ImageSizeChangeType::None;

    if (imageSourceHasChangedSize && SetNeedsLayoutIfNeededAfterIntrinsicSizeChange())
    {
      return;
    }

    if (EverHadLayout() && !SelfNeedsLayout())
    {
      // The inner content rectangle is calculated during layout, but may need an update now
      // (unless the box has already been scheduled for layout). In order to calculate it, we
      // may need values from the containing block, though, so make sure that we're not too
      // early. It may be that layout hasn't even taken place once yet.

      // FIXME: we should not have to trigger another call to setContainerContextForRenderer()
      // from here, since it's already being done during layout.
      UpdateInnerContentRect();
    }

    if (Parent())
    {
      auto repaintRect = ContentBoxRect();
      if (rect)
      {
        // The image changed rect is in source image coordinates (pre-zooming),
        // so map from the bounds of the image to the contentsBox.
        if (auto srcImg = ImageResource().GetImage(FlooredIntSize(ContentBoxRect().Size())))
        {
          FloatSize sourceSize = srcImg->Size() / Style().UsedZoom();
          repaintRect.Intersect(LayoutRect(EnclosingIntRect(
            MapRect(FloatRect(*rect), FloatRect(FloatPoint(), sourceSize), FloatRect(repaintRect)))));
        }
      }
      RepaintRectangle(repaintRect);
    }

    // Tell any potential compositing layers that the image needs updating.
    ContentChanged(ContentChangeType::Image);
  }

  void RenderImage::NotifyFinished(CachedResource &newImage) noexcept
  {
    if (RenderTreeBeingDestroyed())
    {
      return;
    }

    InvalidateBackgroundObscurationStatus();

    if (&newImage == GetCachedImage())
    {
      // tell any potential compositing layers
      // that the image is done and they can reference it directly.
      ContentChanged(ContentChangeType::Image);
    }

    RenderReplaced::NotifyFinished(newImage);
  }

  void RenderImage::SetImageDevicePixelRatio(float factor) noexcept
  {
    if (_imageDevicePixelRatio == factor)
    {
      return;
    }

    _imageDevicePixelRatio = factor;
    IntrinsicSizeChanged();
  }

  bool RenderImage::IsShowingMissingOrImageError() const noexcept
  {
    return !ImageResource().GetCachedImage() || ImageResource().ErrorOccurred();
  }

  bool RenderImage::IsShowingAltText() const noexcept
  {
    return IsShowingMissingOrImageError() && !_altText.empty();
  }

  bool RenderImage::ShouldDisplayBrokenImageIcon() const noexcept
  {
    return ImageResource().ErrorOccurred();
  }

  // Per CSSWG resolution, we should respect 0px inline sizes.
  // See: https://github.com/w3c/csswg-drafts/issues/11236#issuecomment-2718502765
  bool RenderImage::ShouldRespectZeroIntrinsicWidth() const noexcept
  {
    // FIXME: SVG images whose root has an intrinsic width, once there are SVG images.
    return false;
  }

  void RenderImage::PaintIncompleteImageOutline(PaintInfo &paintInfo, LayoutPoint paintOffset,
                                                LayoutUnit borderWidth) const noexcept
  {
    auto contentSize = ContentBoxSize();
    if (contentSize.Width() <= 2 || contentSize.Height() <= 2)
    {
      return;
    }

    auto leftBorder = BorderLeft();
    auto topBorder = BorderTop();
    auto leftPadding = PaddingLeft();
    auto topPadding = PaddingTop();

    // Draw an outline rect where the image should be.
    GraphicsContext &context = paintInfo.Context();
    context.SetStrokeStyle(StrokeStyle::SolidStroke);
    context.SetStrokeColor(Color::lightGray);
    context.SetFillColor(Color::transparentBlack);
    context.DrawRect(SnapRectToDevicePixels(LayoutRect({paintOffset.X() + leftBorder + leftPadding,
                                                        paintOffset.Y() + topBorder + topPadding},
                                                       contentSize),
                                            GetDocument().DeviceScaleFactor()),
                     borderWidth);
  }

  void RenderImage::PaintReplaced(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    GraphicsContext &context = paintInfo.Context();
    // FIXME: Invalidating images with asynchronous decodes and detecting contentful paint, once there are
    // asynchronous decodes and paint milestones.

    auto contentSize = ContentBoxSize();
    float deviceScaleFactor = GetDocument().DeviceScaleFactor();
    LayoutUnit missingImageBorderWidth(1 / deviceScaleFactor);

    // FIXME: Deferred (lazily loaded) images paint nothing, once there's lazy loading.

    if (!ImageResource().GetCachedImage() || ShouldDisplayBrokenImageIcon())
    {
      if (paintInfo.Phase == PaintPhase::Selection)
      {
        return;
      }

      PaintIncompleteImageOutline(paintInfo, paintOffset, missingImageBorderWidth);

      if (contentSize.Width() > 2 && contentSize.Height() > 2)
      {
        LayoutUnit leftBorder = BorderLeft();
        LayoutUnit topBorder = BorderTop();
        LayoutUnit leftPadding = PaddingLeft();
        LayoutUnit topPadding = PaddingTop();

        // When calculating the usable dimensions, exclude the pixels of
        // the outline rect so the error image/alt text doesn't draw on it.
        LayoutSize usableSize =
          contentSize - LayoutSize(2 * missingImageBorderWidth, 2 * missingImageBorderWidth);

        // FIXME: Draw the broken image icon, centred in the content box, once there's one
        // (CachedImage::BrokenImage).

        if (!_altText.empty())
        {
          auto &style = Style();
          auto &fontCascade = style.GetFontCascade();
          auto &fontMetrics = fontCascade.MetricsOfPrimaryFont();
          auto isHorizontal = GetWritingMode().IsHorizontal();
          // FIXME: Bidi alt text (drawBidiText, respecting direction overrides), and the display string
          // modified by the document's encoding.
          auto textRun = RenderBlock::ConstructTextRun(_altText, style);
          auto textWidth = LayoutUnit {fontCascade.Width(textRun)};

          auto hasRoomForAltText = [&]
          {
            // Only draw the alt text if it fits within the content box (content width) and above the error
            // image (content height). Error picture is always visually below the text regardless of writing
            // direction.
            auto availableLogicalWidth = isHorizontal ? usableSize.Width() : usableSize.Height();
            if (availableLogicalWidth < textWidth)
            {
              return false;
            }
            auto availableLogicalHeight = isHorizontal ? usableSize.Height() : usableSize.Width();
            return availableLogicalHeight >= fontMetrics.IntHeight();
          };
          if (hasRoomForAltText())
          {
            context.SetFillColor(style.VisitedDependentColorWithColorFilter(css::PropertyId::Color));
            if (isHorizontal)
            {
              auto contentHorizontalOffset =
                LayoutUnit {leftBorder + leftPadding + (PaddingWidth / 2) - missingImageBorderWidth};
              auto contentVerticalOffset = LayoutUnit {topBorder + topPadding + fontMetrics.IntAscent()
                                                       + (PaddingHeight / 2) - missingImageBorderWidth};
              if (!style.GetWritingMode().IsInlineLeftToRight())
              {
                contentHorizontalOffset += contentSize.Width() - textWidth;
              }
              (void)context.DrawText(
                fontCascade, textRun,
                FloatPoint(paintOffset + LayoutPoint {contentHorizontalOffset, contentVerticalOffset}));
            }
            else
            {
              // FIXME: TextBoxPainter has this logic already, maybe we should transition to some painter
              // class.
              auto contentLogicalHeight = fontMetrics.IntHeight();
              auto adjustedPaintOffset =
                LayoutPoint {paintOffset.X(), paintOffset.Y() - contentLogicalHeight};

              auto visualLeft = Size().Width() / 2 - contentLogicalHeight / 2;
              auto visualRight = visualLeft + contentLogicalHeight;
              if (GetWritingMode().IsBlockFlipped())
              {
                visualLeft = Size().Width() - visualRight;
              }
              visualLeft += adjustedPaintOffset.X();

              auto rotationRect = FloatRect(LayoutRect {visualLeft, adjustedPaintOffset.Y(), textWidth,
                                                        LayoutUnit(contentLogicalHeight)});
              context.ConcatCTM(Rotation(rotationRect, RotationDirection::Clockwise));
              auto textOrigin = LayoutPoint {visualLeft, adjustedPaintOffset.Y() + fontMetrics.IntAscent()};
              (void)context.DrawText(fontCascade, textRun, FloatPoint(textOrigin));
              context.ConcatCTM(Rotation(rotationRect, RotationDirection::Counterclockwise));
            }
          }
        }
      }
      return;
    }

    if (contentSize.IsEmpty())
    {
      return;
    }

    // FIXME: The incomplete image border setting (incompleteImageBorderEnabled), and recording unpainted and
    // repainted objects for paint milestones.

    RefPtr<Image> img = ImageResource().GetImage(FlooredIntSize(contentSize));
    if (!img || img->IsNull())
    {
      return;
    }

    LayoutRect contentBoxRect = ContentBoxRect();
    contentBoxRect.MoveBy(paintOffset);
    LayoutRect replacedContentRect = ReplacedContentRect();
    replacedContentRect.MoveBy(paintOffset);
    bool clip = !contentBoxRect.Contains(replacedContentRect);
    GraphicsContextStateSaver stateSaver(context, clip);
    if (clip)
    {
      context.Clip(FloatRect(contentBoxRect));
    }

    (void)PaintIntoRect(paintInfo, SnapRectToDevicePixels(replacedContentRect, deviceScaleFactor));
  }

  ImageDrawResult RenderImage::PaintIntoRect(PaintInfo &paintInfo, const FloatRect &rect) noexcept
  {
    if (!ImageResource().GetCachedImage() || ImageResource().ErrorOccurred() || rect.Width() <= 0
        || rect.Height() <= 0)
    {
      return ImageDrawResult::DidNothing;
    }

    RefPtr<Image> img = ImageResource().GetImage(FlooredIntSize(rect.Size()));
    if (!img || img->IsNull())
    {
      return ImageDrawResult::DidNothing;
    }

    // FIXME: Document when image != img.get().
    RefPtr image = ImageResource().GetImage();

    // FIXME: The decoding mode, subsampling, debug backgrounds and HDR options, once they exist.
    ImagePaintingOptions options = {
      CompositeOperator::SourceOver, GetImageOrientation(),
      image ? ChooseInterpolationQuality(paintInfo.Context(), *image, image.get(), LayoutSize(rect.Size()))
            : InterpolationQuality::Default};

    auto drawResult = paintInfo.Context().DrawImage(*img, rect, options);

    // FIXME: Wait for asynchronous decoding (addClientWaitingForAsyncDecoding), and remember that the element
    // painted images (setHasEverPaintedImages), once there are asynchronous decodes.

    return drawResult;
  }

  LayoutUnit RenderImage::MinimumReplacedHeight() const noexcept
  {
    return ImageResource().ErrorOccurred() ? IntrinsicSize().Height() : 0_lu;
  }

  void RenderImage::UpdateAltText() noexcept
  {
    if (!GetElement())
    {
      return;
    }

    Maybe<utf8_string> altText;
    // FIXME: <input type=image>'s alt text, once it has a renderer.
    if (auto *image = DynamicDowncast<html::HTMLImageElement>(*GetElement()))
    {
      altText = image->AltText();
    }

    // We check for null and not empty because we don't want to override empty-string
    // alt text provided by either of the above branches.
    _altText = altText ? krys::move(*altText) : utf8_string(Style().AltFromContent());
  }

  void RenderImage::Layout() noexcept
  {
    // Recomputing overflow is required only when child content is present.
    if (NeedsSimplifiedNormalFlowLayoutOnly())
    {
      ClearNeedsLayout();
      return;
    }

    RenderReplaced::Layout();

    UpdateInnerContentRect();
  }

  FloatSize RenderImage::ComputeIntrinsicSize() const noexcept
  {
    krys_debug_assert(!ShouldApplySizeContainment());
    auto intrinsicSize = RenderReplaced::ComputeIntrinsicSize();

    // Our intrinsicSize is empty if we're rendering generated images with relative width/height. Figure out
    // the right intrinsic size to use.
    if (intrinsicSize.IsEmpty()
        && (ImageResource().ImageHasRelativeWidth() || ImageResource().ImageHasRelativeHeight()))
    {
      RenderObject *containingBlock = IsOutOfFlowPositioned() ? Container() : ContainingBlock();
      if (auto *box = DynamicDowncast<RenderBox>(containingBlock))
      {
        intrinsicSize.SetWidth(box->ContentBoxLogicalWidth());
        intrinsicSize.SetHeight(
          box->AvailableLogicalHeight(AvailableLogicalHeightType::IncludeMarginBorderPadding));
      }
    }

    return intrinsicSize;
  }

  FloatSize RenderImage::PreferredAspectRatio() const noexcept
  {
    krys_debug_assert(!ShouldApplySizeContainment());

    // Don't compute an intrinsic ratio to preserve historical behavior if we're painting alt text
    // and/or a broken image.
    if (ShouldDisplayBrokenImageIcon())
    {
      if (Style().GetAspectRatio().IsAutoAndRatio() && !IsShowingAltText())
      {
        return FloatSize::NarrowPrecision(Style().GetAspectRatio().Width().Value,
                                          Style().GetAspectRatio().Height().Value);
      }
      return {1.0f, 1.0f};
    }

    return RenderReplaced::PreferredAspectRatio();
  }

  bool RenderImage::ShouldInvalidatePreferredWidths() const noexcept
  {
    // FIXME: SVG images with an embedded content box, once there are SVG images.
    return RenderReplaced::ShouldInvalidatePreferredWidths();
  }
}
