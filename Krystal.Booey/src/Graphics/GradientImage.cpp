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
    FillMapped(destContext, destRect, srcRect);
    return ImageDrawResult::DidDraw;
  }

  void GradientImage::FillMapped(GraphicsContext &destContext, const FloatRect &destRect,
                                 const FloatRect &srcRect) noexcept
  {
    destContext.Translate(destRect.Location());
    if (destRect.Size() != srcRect.Size())
    {
      destContext.Scale(destRect.Size() / srcRect.Size());
    }
    destContext.Translate(-srcRect.Location());
    destContext.FillRect(FloatRect(FloatPoint(), Size()), *_gradient);
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

    // The phase is where the whole image's origin goes, so the tile starts at its source rect's scaled
    // location from there.
    const FloatPoint tileOrigin(phase.X() + srcRect.X() * static_cast<float>(patternTransform.A()),
                                phase.Y() + srcRect.Y() * static_cast<float>(patternTransform.D()));
    const float startX =
      tileOrigin.X() + std::floor((destRect.X() - tileOrigin.X()) / step.Width()) * step.Width();
    const float startY =
      tileOrigin.Y() + std::floor((destRect.Y() - tileOrigin.Y()) / step.Height()) * step.Height();
    const double columns = std::ceil((destRect.MaxX() - startX) / step.Width());
    const double rows = std::ceil((destRect.MaxY() - startY) / step.Height());
    if (columns * rows > static_cast<double>(MaxTiles))
    {
      return;
    }

    // Each tile is clipped to its rect with the edges rounded to whole units, so neighbouring tiles share an
    // edge rather than both antialiasing it (which would let what's beneath show through as a seam).
    auto snappedTileRect = [](float x, float y, const FloatSize &size)
    {
      const float left = std::round(x);
      const float top = std::round(y);
      return FloatRect(left, top, std::round(x + size.Width()) - left, std::round(y + size.Height()) - top);
    };

    GraphicsContextStateSaver stateSaver(destContext);
    destContext.Clip(destRect);
    destContext.SetCompositeOperation(options.GetCompositeOperator(), options.GetBlendMode());
    for (float y = startY; y < destRect.MaxY(); y += step.Height())
    {
      for (float x = startX; x < destRect.MaxX(); x += step.Width())
      {
        GraphicsContextStateSaver tileStateSaver(destContext);
        destContext.Clip(snappedTileRect(x, y, tileSize));
        FillMapped(destContext, FloatRect(FloatPoint(x, y), tileSize), srcRect);
      }
    }
  }
}
