#include "Krystal.Booey/Platform/DragImage.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Graphics/StringTruncator.hpp"
#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Page/FrameSnapshotting.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/URL/URL.hpp"

namespace krys::boo
{
  IntSize DragImageSize(const DragImageRef &image) noexcept
  {
    if (image)
    {
      return image->BackendSize();
    }
    return {0, 0};
  }

  DragImageRef FitDragImageToMaxSize(DragImageRef image, const IntSize &layoutSize,
                                     const IntSize &maxSize) noexcept
  {
    float heightResizeRatio = 0.0f;
    float widthResizeRatio = 0.0f;
    float resizeRatio = -1.0f;
    IntSize originalSize = DragImageSize(image);

    if (layoutSize.Width() > maxSize.Width())
    {
      widthResizeRatio = maxSize.Width() / static_cast<float>(layoutSize.Width());
      resizeRatio = widthResizeRatio;
    }

    if (layoutSize.Height() > maxSize.Height())
    {
      heightResizeRatio = maxSize.Height() / static_cast<float>(layoutSize.Height());
      if ((resizeRatio < 0.0f) || (resizeRatio > heightResizeRatio))
      {
        resizeRatio = heightResizeRatio;
      }
    }

    if (layoutSize == originalSize)
    {
      return resizeRatio > 0.0f ? ScaleDragImage(krys::move(image), FloatSize(resizeRatio, resizeRatio))
                                : image;
    }

    // The image was scaled in the webpage so at minimum we must account for that scaling.
    float scaleX = layoutSize.Width() / static_cast<float>(originalSize.Width());
    float scaleY = layoutSize.Height() / static_cast<float>(originalSize.Height());
    if (resizeRatio > 0.0f)
    {
      scaleX *= resizeRatio;
      scaleY *= resizeRatio;
    }

    return ScaleDragImage(krys::move(image), FloatSize(scaleX, scaleY));
  }

  // Drag images record display lists, so scaling one redraws it at the new size rather than resampling it.
  DragImageRef ScaleDragImage(DragImageRef image, FloatSize scale) noexcept
  {
    if (!image)
    {
      return nullptr;
    }

    IntSize imageSize = DragImageSize(image);
    IntSize scaledSize(imageSize);
    scaledSize.Scale(scale.Width(), scale.Height());
    if (imageSize == scaledSize)
    {
      return image;
    }

    auto scaledImage = ImageBuffer::Create(FloatSize(scaledSize), 1);
    if (!scaledImage)
    {
      return nullptr;
    }
    scaledImage->Context().DrawImageBuffer(*image, FloatRect(FloatPoint(), FloatSize(scaledSize)));
    return scaledImage;
  }

  DragImageRef PlatformAdjustDragImageForDeviceScaleFactor(DragImageRef image,
                                                           float deviceScaleFactor) noexcept
  {
    // Later code expects the drag image to be scaled by device's scale factor.
    return ScaleDragImage(krys::move(image), {deviceScaleFactor, deviceScaleFactor});
  }

  DragImageRef DissolveDragImageToFraction(DragImageRef image, float fraction) noexcept
  {
    if (!image)
    {
      return nullptr;
    }

    auto dissolvedImage = ImageBuffer::Create(image->LogicalSize(), image->ResolutionScale());
    if (!dissolvedImage)
    {
      return nullptr;
    }
    auto &context = dissolvedImage->Context();
    context.SetAlpha(fraction);
    context.DrawImageBuffer(*image, FloatRect(FloatPoint(), image->LogicalSize()));
    return dissolvedImage;
  }

  DragImageRef CreateDragImageFromImage(Image *image, ImageOrientation orientation) noexcept
  {
    if (!image)
    {
      return nullptr;
    }

    auto size = image->Size(orientation);
    auto dragImage = ImageBuffer::Create(size, 1);
    if (!dragImage)
    {
      return nullptr;
    }
    dragImage->Context().DrawImage(*image, FloatRect(FloatPoint(), size), ImagePaintingOptions {orientation});
    return dragImage;
  }

  DragImageRef CreateDragImageIconForCachedImageFilename(utf8_stringview) noexcept
  {
    return nullptr;
  }

  namespace
  {
    struct ScopedNodeDragEnabler
    {
      KRYS_NON_COPY_MOVABLE(ScopedNodeDragEnabler);

      // Elements have no drag state to set while they're painted: no pseudo-class matches dragged elements.
      ScopedNodeDragEnabler(LocalFrame &frame, dom::Node &) noexcept
      {
        (void)frame.Document()->UpdateLayout();
      }
    };

    // The snapshot is the drag image: it isn't turned into an image, so it isn't oriented.
    KRYS_NODISCARD DragImageRef CreateDragImageFromSnapshot(RefPtr<ImageBuffer> snapshot,
                                                            dom::Node *node) noexcept
    {
      if (!snapshot)
      {
        return nullptr;
      }

      if (node)
      {
        if (!DynamicDowncast<RenderElement>(node->Renderer()))
        {
          return nullptr;
        }
      }

      return snapshot;
    }
  }

  DragImageRef CreateDragImageForNode(LocalFrame &frame, dom::Node &node) noexcept
  {
    ScopedNodeDragEnabler enableDrag(frame, node);
    SnapshotOptions options {{SnapshotFlags::DraggableElement}};
    return CreateDragImageFromSnapshot(SnapshotNode(frame, node, krys::move(options)), &node);
  }

  DragImageData CreateDragImageForSelection(LocalFrame &frame, bool forceBlackText) noexcept
  {
    SnapshotOptions options;
    if (forceBlackText)
    {
      options.Flags.Add(SnapshotFlags::ForceBlackText);
    }
    return {CreateDragImageFromSnapshot(SnapshotSelection(frame, krys::move(options)), nullptr)};
  }

  namespace
  {
    constexpr float DragLabelBorderX = 4;
    // Keep border_y in synch with DragController::LinkDragBorderInset.
    constexpr float DragLabelBorderY = 2;
    constexpr float DragLabelRadius = 5;
    constexpr float LabelBorderYOffset = 2;

    constexpr float MaxDragLabelWidth = 200;
    constexpr float MaxDragLabelStringWidth = (MaxDragLabelWidth - 2 * DragLabelBorderX);

    constexpr float DragLinkLabelFontsize = 11;
    constexpr float DragLinkUrlFontSize = 10;

    /// @brief The system's small caption font at @p size.
    KRYS_NODISCARD FontCascade DragLabelFont(float size, bool bold) noexcept
    {
      FontCascadeDescription description;
      description.SetWeight(bold ? BoldWeightValue() : NormalWeightValue());
      description.SetOneFamily(SystemFontDatabase::Singleton().SystemFontShorthandFamily(
        SystemFontDatabase::FontShorthand::SmallCaption));
      description.SetSpecifiedSize(size);
      description.SetComputedSize(size);
      FontCascade result(krys::move(description));
      result.Update();
      return result;
    }

    void DrawTextAtPoint(GraphicsContext &context, utf8_stringview text, const IntPoint &point,
                         const FontCascade &font, const Color &color) noexcept
    {
      TextRun run(text);

      context.SetFillColor(color);
      context.DrawBidiText(font, run, FloatPoint(point));
    }

    void DrawDoubledTextAtPoint(GraphicsContext &context, utf8_stringview text, const IntPoint &point,
                                const FontCascade &font, const Color &topColor,
                                const Color &bottomColor) noexcept
    {
      GraphicsContextStateSaver stateSaver(context);

      IntPoint textPos = point;

      DrawTextAtPoint(context, text, textPos, font, bottomColor);
      textPos.Move(0, -1);
      DrawTextAtPoint(context, text, textPos, font, topColor);
    }
  }

  DragImageData CreateDragImageForLink(dom::Element &, url::URL &url, utf8_stringview inLabel, float) noexcept
  {
    const FontCascade labelFont = DragLabelFont(DragLinkLabelFontsize, true);
    const FontCascade urlFont = DragLabelFont(DragLinkUrlFontSize, false);

    bool drawURLString = true;
    bool clipURLString = false;
    bool clipLabelString = false;

    utf8_string urlString = url.String();
    utf8_string label(inLabel);
    if (label.empty())
    {
      drawURLString = false;
      label = urlString;
    }

    // First step in drawing the link drag image width.
    TextRun labelRun(label);
    TextRun urlRun(urlString);
    IntSize labelSize(static_cast<int32>(labelFont.Width(labelRun)),
                      labelFont.MetricsOfPrimaryFont().IntAscent()
                        + labelFont.MetricsOfPrimaryFont().IntDescent());

    if (labelSize.Width() > MaxDragLabelStringWidth)
    {
      labelSize.SetWidth(static_cast<int32>(MaxDragLabelStringWidth));
      clipLabelString = true;
    }

    IntSize urlStringSize;
    IntSize imageSize(static_cast<int32>(labelSize.Width() + DragLabelBorderX * 2),
                      static_cast<int32>(labelSize.Height() + DragLabelBorderY * 2));

    if (drawURLString)
    {
      urlStringSize.SetWidth(static_cast<int32>(urlFont.Width(urlRun)));
      urlStringSize.SetHeight(urlFont.MetricsOfPrimaryFont().IntAscent()
                              + urlFont.MetricsOfPrimaryFont().IntDescent());
      imageSize.SetHeight(imageSize.Height() + urlStringSize.Height());
      if (urlStringSize.Width() > MaxDragLabelStringWidth)
      {
        imageSize.SetWidth(static_cast<int32>(MaxDragLabelWidth));
        clipURLString = true;
      }
      else
      {
        imageSize.SetWidth(
          static_cast<int32>(std::max(labelSize.Width(), urlStringSize.Width()) + DragLabelBorderX * 2));
      }
    }

    // We now know how big the image needs to be, so we create and
    // fill the background
    auto image = ImageBuffer::Create(FloatSize(imageSize), 1);
    if (!image)
    {
      return {nullptr};
    }

    auto &context = image->Context();
    constexpr auto backgroundColor = SRGBA<uint8> {140, 140, 140};
    const FloatSize radii(DragLabelRadius, DragLabelRadius);
    IntRect rect(0, 0, imageSize.Width(), imageSize.Height());
    context.FillRoundedRect(FloatRoundedRect(FloatRect(rect), radii, radii, radii, radii), backgroundColor);

    // Draw the text
    constexpr auto topColor = Color::black;                         // original alpha = 0.75
    constexpr auto bottomColor = SRGBA<uint8> {255, 255, 255, 127}; // original alpha = 0.5
    if (drawURLString)
    {
      if (clipURLString)
      {
        urlString =
          StringTruncator::RightTruncate(urlString, imageSize.Width() - (DragLabelBorderX * 2.0f), urlFont);
      }
      IntPoint textPos(
        static_cast<int32>(DragLabelBorderX),
        static_cast<int32>(imageSize.Height()
                           - (LabelBorderYOffset + urlFont.MetricsOfPrimaryFont().IntDescent())));
      DrawDoubledTextAtPoint(context, urlString, textPos, urlFont, topColor, bottomColor);
    }

    if (clipLabelString)
    {
      label = StringTruncator::RightTruncate(label, imageSize.Width() - (DragLabelBorderX * 2.0f), labelFont);
    }

    IntPoint textPos(static_cast<int32>(DragLabelBorderX),
                     static_cast<int32>(DragLabelBorderY + labelFont.Size()));
    DrawDoubledTextAtPoint(context, label, textPos, labelFont, topColor, bottomColor);

    return {image};
  }

  namespace
  {
    constexpr int32 LinkDragBorderInset = 2;
  }

  IntPoint DragOffsetForLinkDragImage(const DragImageRef &dragImage) noexcept
  {
    IntSize size = DragImageSize(dragImage);
    return {-size.Width() / 2, -LinkDragBorderInset};
  }

  FloatPoint AnchorPointForLinkDragImage(const DragImageRef &dragImage) noexcept
  {
    IntSize size = DragImageSize(dragImage);
    if (!size.Height())
    {
      return {0.5, 0};
    }
    return {0.5, static_cast<float>(size.Height() - LinkDragBorderInset) / static_cast<float>(size.Height())};
  }
}
