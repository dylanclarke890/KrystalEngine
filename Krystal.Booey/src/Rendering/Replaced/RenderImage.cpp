/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2006 Allan Sandfeld Jensen (kde@carewolf.com)
 *           (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011-2012. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Graphics/Images/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestLocation.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Painting/TextPainter.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImageResourceStyleImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/Text/UnicodeHelpers.hpp"
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

    KRYS_NODISCARD bool IsDeferredImage(dom::Element *element) noexcept
    {
      auto *image = DynamicDowncast<html::HTMLImageElement>(element);
      return image && image->IsDeferred();
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
      // Alt text stands in for the broken image icon, as in Chrome.
      if (!_altText.empty())
      {
        return IntSize(PaddingWidth, PaddingHeight);
      }
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
    auto *cachedImage = GetCachedImage();
    if (!cachedImage)
    {
      return false;
    }
    if (auto *svgImage = DynamicDowncast<svg::SVGImage>(cachedImage->GetImage()))
    {
      if (auto rootElement = svgImage->RootElement())
      {
        return rootElement->HasIntrinsicWidth();
      }
    }
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

  void RenderImage::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    RenderReplaced::Paint(paintInfo, paintOffset);

    if (paintInfo.Phase == PaintPhase::Outline)
    {
      PaintAreaElementFocusRing(paintInfo, paintOffset);
    }
  }

  void RenderImage::PaintAreaElementFocusRing(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    if (GetDocument().Printing() || !GetDocument().Selection().IsFocusedAndActive())
    {
      return;
    }

    if (paintInfo.Context().PaintingDisabled() && !paintInfo.Context().PerformingPaintInvalidation())
    {
      return;
    }

    RefPtr<html::HTMLAreaElement> areaElement =
      ShareRefPtr(DynamicDowncast<html::HTMLAreaElement>(GetDocument().FocusedElement()));
    if (!areaElement || areaElement->ImageElement().get() != GetElement())
    {
      return;
    }

    auto *areaElementStyle = areaElement->ComputedStyle();
    if (!areaElementStyle)
    {
      return;
    }

    auto outlineWidth = style::Evaluate<float>(areaElementStyle->OutlineWidth(), style::ZoomNeeded {});
    if (!outlineWidth)
    {
      return;
    }

    // Even if the theme handles focus ring drawing for entire elements, it won't do it for
    // an area within an image, so we don't call RenderTheme::SupportsFocusRing here.
    auto path = areaElement->ComputePathForFocusRing(Size());
    if (path.IsEmpty())
    {
      return;
    }

    AffineTransform zoomTransform;
    zoomTransform.Scale(Style().UsedZoom());
    path.Transform(zoomTransform);

    auto adjustedOffset = paintOffset;
    adjustedOffset.MoveBy(Location());
    path.Translate(ToFloatSize(adjustedOffset));

    paintInfo.Context().DrawFocusRing(
      path, outlineWidth, areaElementStyle->ColorResolvingCurrentColor(css::PropertyId::OutlineColor));
  }

  void RenderImage::AreaElementFocusChanged(html::HTMLAreaElement &element) noexcept
  {
    krys_debug_assert(element.ImageElement().get() == GetElement());
    (void)element;

    // It would be more efficient to only repaint the focus ring rectangle
    // for the passed-in area element. That would require adding functions
    // to the area element class.
    Repaint();
  }

  html::HTMLMapElement *RenderImage::ImageMap() const noexcept
  {
    auto *imageElement = DynamicDowncast<html::HTMLImageElement>(GetElement());
    return imageElement ? imageElement->AssociatedMapElement() : nullptr;
  }

  bool RenderImage::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                const HitTestLocation &locationInContainer,
                                const LayoutPoint &accumulatedOffset, HitTestAction action) noexcept
  {
    HitTestResult tempResult(result.GetHitTestLocation());
    bool inside =
      RenderReplaced::NodeAtPoint(request, tempResult, locationInContainer, accumulatedOffset, action);

    if (tempResult.InnerNode() && GetElement())
    {
      if (RefPtr<html::HTMLMapElement> map = ShareRefPtr(ImageMap()))
      {
        LayoutRect contentBox = ContentBoxRect();
        float scaleFactor = 1 / Style().UsedZoom();
        LayoutPoint mapLocation = locationInContainer.Point() - ToLayoutSize(accumulatedOffset)
                                  - LocationOffset() - ToLayoutSize(contentBox.Location());
        mapLocation.Scale(scaleFactor);

        if (map->MapMouseEvent(mapLocation, contentBox.Size(), tempResult))
        {
          tempResult.SetInnerNonSharedNode(GetElement());
        }
      }
    }

    if (!inside && request.ResultIsElementList())
    {
      result.Append(tempResult, request);
    }
    if (inside)
    {
      result = tempResult;
    }
    return inside;
  }

  void RenderImage::PaintReplaced(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    GraphicsContext &context = paintInfo.Context();
    // FIXME: Invalidating images with asynchronous decodes for a host's snapshot, and detecting contentful
    // paint, once there are snapshots and paint milestones.

    auto contentSize = ContentBoxSize();
    float deviceScaleFactor = GetDocument().DeviceScaleFactor();
    LayoutUnit missingImageBorderWidth(1 / deviceScaleFactor);

    if (IsDeferredImage(GetElement()))
    {
      return;
    }

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

        bool errorPictureDrawn = false;
        LayoutSize imageOffset;
        // When calculating the usable dimensions, exclude the pixels of
        // the outline rect so the error image/alt text doesn't draw on it.
        LayoutSize usableSize =
          contentSize - LayoutSize(2 * missingImageBorderWidth, 2 * missingImageBorderWidth);

        // As Chrome, alt text stands in for the icon.
        RefPtr<Image> image = ImageResource().GetImage();
        if (ShouldDisplayBrokenImageIcon() && _altText.empty() && image && !image->IsNull()
            && usableSize.Width() >= image->Width() && usableSize.Height() >= image->Height())
        {
          auto [brokenImage, imageScaleFactor] = GetCachedImage()->BrokenImage(deviceScaleFactor);
          FloatSize imageSize = brokenImage->Size();
          imageSize.Scale(1 / imageScaleFactor);

          // Centre the error image, accounting for border and padding.
          LayoutUnit centerX {(usableSize.Width() - imageSize.Width()) / 2};
          if (centerX < 0)
          {
            centerX = 0;
          }
          LayoutUnit centerY {(usableSize.Height() - imageSize.Height()) / 2};
          if (centerY < 0)
          {
            centerY = 0;
          }
          imageOffset = LayoutSize(leftBorder + leftPadding + centerX + missingImageBorderWidth,
                                   topBorder + topPadding + centerY + missingImageBorderWidth);

          context.DrawImage(
            *brokenImage,
            SnapRectToDevicePixels(LayoutRect(paintOffset + imageOffset, LayoutSize(imageSize)),
                                   deviceScaleFactor),
            ImagePaintingOptions {GetImageOrientation()});
          errorPictureDrawn = true;
        }

        if (!_altText.empty())
        {
          auto &style = Style();
          auto &fontCascade = style.GetFontCascade();
          auto &fontMetrics = fontCascade.MetricsOfPrimaryFont();
          auto isHorizontal = GetWritingMode().IsHorizontal();
          // FIXME: The display string modified by the document's encoding.
          auto textRun = RenderBlock::ConstructTextRun(
            _altText, style, ExpansionBehavior::DefaultBehavior(),
            {TextRunFlag::RespectDirection, TextRunFlag::RespectDirectionOverride});
          // With dir=auto, the text has the directionality of its attribute: its first strong character's, or
          // ltr. https://html.spec.whatwg.org/#directionality-of-the-attribute
          if (auto *element = GetElement();
              _altTextIsFromAttribute && element && element->HasAutoTextDirectionState())
          {
            textRun.SetDirection(DefaultWritingDirection(_altText).value_or(TextDirection::LTR));
          }
          auto textWidth = LayoutUnit {fontCascade.Width(textRun)};

          auto hasRoomForAltText = [&]
          {
            // Only draw the alt text if it fits within the content box (content width) and above the error
            // image (content height). Error picture is always visually below the text regardless of writing
            // direction.
            auto availableLogicalWidth = isHorizontal
                                           ? usableSize.Width()
                                           : (errorPictureDrawn ? imageOffset.Height() : usableSize.Height());
            if (availableLogicalWidth < textWidth)
            {
              return false;
            }
            auto availableLogicalHeight = isHorizontal
                                            ? (errorPictureDrawn ? imageOffset.Height() : usableSize.Height())
                                            : usableSize.Width();
            return availableLogicalHeight >= fontMetrics.IntHeight();
          };
          if (hasRoomForAltText())
          {
            context.SetFillColor(style.ColorResolvingCurrentColor(css::PropertyId::Color));
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
              context.DrawBidiText(
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
              context.DrawBidiText(fontCascade, textRun, FloatPoint(textOrigin));
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

    // FIXME: Recording unpainted and repainted objects for paint milestones.

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

    // FIXME: The subsampling, debug backgrounds and HDR options, once they exist.
    ImagePaintingOptions options = {
      CompositeOperator::SourceOver,
      image ? DecodingModeForImageDraw(*image, paintInfo) : DecodingMode::Synchronous, GetImageOrientation(),
      image ? ChooseInterpolationQuality(paintInfo.Context(), *image, image.get(), LayoutSize(rect.Size()))
            : InterpolationQuality::Default};

    auto drawResult = paintInfo.Context().DrawImage(*img, rect, options);

    if (drawResult == ImageDrawResult::DidRequestDecoding)
    {
      ImageResource().GetCachedImage()->AddClientWaitingForAsyncDecoding(GetCachedImageClient());
    }

    if (GetElement() && !paintInfo.Context().PaintingDisabled())
    {
      GetElement()->SetHasEverPaintedImages(true);
    }

    return drawResult;
  }

  LayoutUnit RenderImage::MinimumReplacedHeight() const noexcept
  {
    return ImageResource().ErrorOccurred() ? IntrinsicSize().Height() : 0_lu;
  }

  void RenderImage::UpdateForAltText() noexcept
  {
    UpdateAltText();
    if (GetCachedImage() && !ImageResource().ErrorOccurred())
    {
      return;
    }
    RepaintOrMarkForLayout(SetImageSizeForAltText(GetCachedImage()));
  }

  void RenderImage::UpdateAltText() noexcept
  {
    if (!GetElement())
    {
      return;
    }

    Maybe<utf8_string> altText;
    if (auto *image = DynamicDowncast<html::HTMLImageElement>(*GetElement()))
    {
      altText = image->AltText();
    }
    else if (auto *input = DynamicDowncast<html::HTMLInputElement>(*GetElement());
             input && input->IsImageButton())
    {
      altText = input->AltText();
    }

    // We check for null and not empty because we don't want to override empty-string
    // alt text provided by either of the above branches.
    _altTextIsFromAttribute = altText.has_value();
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
    if (RenderReplaced::ShouldInvalidatePreferredWidths())
    {
      return true;
    }
    return EmbeddedContentBox();
  }

  RenderBox *RenderImage::EmbeddedContentBox() const noexcept
  {
    if (auto *cachedImage = GetCachedImage())
    {
      if (auto *image = DynamicDowncast<svg::SVGImage>(cachedImage->GetImage()))
      {
        return image->EmbeddedContentBox();
      }
    }
    return nullptr;
  }
}
