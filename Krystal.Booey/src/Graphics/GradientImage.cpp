#include "Krystal.Booey/Graphics/GradientImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include <cmath>

namespace krys::boo
{
  GradientImage::GradientImage(boo::Gradient &generator, const FloatSize &size) noexcept
      : _gradient(ShareRef(generator))
  {
    SetContainerSize(size);
  }

  GradientImage::~GradientImage() noexcept = default;

  ImageDrawResult GradientImage::Draw(GraphicsContext &destContext, const FloatRect &destRect,
                                      const FloatRect &srcRect, ImagePaintingOptions options) noexcept
  {
    GraphicsContextStateSaver stateSaver(destContext);
    destContext.SetCompositeOperation(options.GetCompositeOperator(), options.GetBlendMode());
    destContext.Clip(destRect);
    destContext.Translate(destRect.Location());
    if (destRect.Size() != srcRect.Size())
    {
      destContext.Scale(destRect.Size() / srcRect.Size());
    }
    destContext.Translate(-srcRect.Location());
    destContext.FillRect(FloatRect(FloatPoint(), Size()), *_gradient);
    return ImageDrawResult::DidDraw;
  }

  void GradientImage::DrawPattern(GraphicsContext &destContext, const FloatRect &destRect,
                                  const FloatRect &srcRect, const AffineTransform &patternTransform,
                                  const FloatPoint &phase, const FloatSize &spacing,
                                  ImagePaintingOptions options) noexcept
  {
    // Each tile is the gradient drawn into its own rect, the tiles starting at the phase and repeating every
    // tile and spacing, clipped to the destination.
    const FloatSize tileSize(srcRect.Width() * static_cast<float>(patternTransform.A()),
                             srcRect.Height() * static_cast<float>(patternTransform.D()));
    const FloatSize step = tileSize + spacing;
    if (tileSize.IsEmpty() || step.Width() <= 0 || step.Height() <= 0 || destRect.IsEmpty())
    {
      return;
    }

    const float startX = phase.X() + std::floor((destRect.X() - phase.X()) / step.Width()) * step.Width();
    const float startY = phase.Y() + std::floor((destRect.Y() - phase.Y()) / step.Height()) * step.Height();
    const double columns = std::ceil((destRect.MaxX() - startX) / step.Width());
    const double rows = std::ceil((destRect.MaxY() - startY) / step.Height());
    if (columns * rows > static_cast<double>(MaxTiles))
    {
      return;
    }

    GraphicsContextStateSaver stateSaver(destContext);
    destContext.Clip(destRect);
    for (float y = startY; y < destRect.MaxY(); y += step.Height())
    {
      for (float x = startX; x < destRect.MaxX(); x += step.Width())
      {
        (void)Draw(destContext, FloatRect(FloatPoint(x, y), tileSize), srcRect, options);
      }
    }
  }
}
