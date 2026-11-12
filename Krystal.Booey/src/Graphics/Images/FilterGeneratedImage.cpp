#include "Krystal.Booey/Graphics/Images/FilterGeneratedImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"

namespace krys::boo
{
  FilterGeneratedImage::FilterGeneratedImage(Image &image, Filter &filter, const FloatSize &size) noexcept
      : _image(ShareRef(image)), _filter(ShareRef(filter))
  {
    SetContainerSize(size);
  }

  void FilterGeneratedImage::DrawFiltered(GraphicsContext &context) noexcept
  {
    FloatRect sourceImageRect {FloatPoint(), Size()};
    auto switcher = GraphicsContextSwitcher::Create(context, sourceImageRect, RefPtr<Filter>(_filter));
    if (!switcher)
    {
      return;
    }

    switcher->BeginDrawSourceImage(context);
    if (auto *drawingContext = switcher->DrawingContext(context))
    {
      (void)drawingContext->DrawImage(*_image, sourceImageRect);
    }
    switcher->EndDrawSourceImage(context);
  }

  ImageDrawResult FilterGeneratedImage::Draw(GraphicsContext &context, const FloatRect &dstRect,
                                             const FloatRect &srcRect, ImagePaintingOptions options) noexcept
  {
    GraphicsContextStateSaver stateSaver(context);
    context.SetCompositeOperation(options.GetCompositeOperator(), options.GetBlendMode());
    context.Clip(dstRect);
    context.Translate(dstRect.Location());
    if (dstRect.Size() != srcRect.Size())
    {
      context.Scale(dstRect.Size() / srcRect.Size());
    }
    context.Translate(-srcRect.Location());

    DrawFiltered(context);
    return ImageDrawResult::DidDraw;
  }

  void FilterGeneratedImage::DrawPattern(GraphicsContext &context, const FloatRect &dstRect,
                                         const FloatRect &srcRect, const AffineTransform &patternTransform,
                                         const FloatPoint &phase, const FloatSize &spacing,
                                         ImagePaintingOptions options) noexcept
  {
    auto imageBuffer = context.CreateImageBuffer(Size());
    if (!imageBuffer)
    {
      return;
    }

    DrawFiltered(imageBuffer->Context());
    context.DrawPattern(*imageBuffer, dstRect, srcRect, patternTransform, phase, spacing, options);
  }
}
