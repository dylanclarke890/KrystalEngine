/*
 * Copyright (C) 2008-2016 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Images/GradientImage.hpp"
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

      if (destContext.DrawLuminanceMask())
      {
        imageBuffer->ConvertToLuminanceMask();
      }

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
