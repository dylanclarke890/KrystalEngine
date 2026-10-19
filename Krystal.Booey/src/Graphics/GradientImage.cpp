#include "Krystal.Booey/Graphics/GradientImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Core/Numeric.hpp"
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
    // Allow the generator to provide visually-equivalent tiling parameters for better performance.
    FloatSize adjustedSize = Size();
    FloatRect adjustedSrcRect = srcRect;
    _gradient->AdjustParametersForTiledDrawing(adjustedSize, adjustedSrcRect, spacing);

    // Factor in the destination context's scale to generate at the best resolution
    AffineTransform destContextCTM = destContext.GetCTM(GraphicsContext::DefinitelyIncludeDeviceScale);
    double xScale = std::abs(destContextCTM.XScale());
    double yScale = std::abs(destContextCTM.YScale());
    AffineTransform adjustedPatternCTM = patternTransform;
    adjustedPatternCTM.Scale(1.0 / xScale, 1.0 / yScale);
    adjustedSrcRect.Scale(static_cast<float>(xScale), static_cast<float>(yScale));

    uint32 generatorHash = _gradient->Hash();

    if (!_cachedImage || _cachedGeneratorHash != generatorHash || _cachedAdjustedSize != adjustedSize
        || !AreEssentiallyEqual(destContext.ScaleFactor(), _cachedScaleFactor))
    {
      auto imageBuffer = destContext.CreateAlignedImageBuffer(adjustedSize);
      if (!imageBuffer)
      {
        return;
      }

      // Fill with the generated image.
      imageBuffer->Context().FillRect(FloatRect(FloatPoint(), adjustedSize), *_gradient);

      _cachedGeneratorHash = generatorHash;
      _cachedAdjustedSize = adjustedSize;
      _cachedScaleFactor = destContext.ScaleFactor();

      // FIXME: Luminance masks (drawLuminanceMask, convertToLuminanceMask) come with masks.

      _cachedImage = krys::move(imageBuffer);
      if (!_cachedImage)
      {
        return;
      }
    }

    destContext.SetDrawLuminanceMask(false);
    destContext.DrawPattern(*_cachedImage, destRect, adjustedSrcRect, adjustedPatternCTM, phase, spacing,
                            options);
  }
}
