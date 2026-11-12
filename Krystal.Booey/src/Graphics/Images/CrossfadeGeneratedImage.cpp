/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/CrossfadeGeneratedImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"

namespace krys::boo
{
  CrossfadeGeneratedImage::CrossfadeGeneratedImage(Image &fromImage, Image &toImage, float percentage,
                                                   const FloatSize &crossfadeSize,
                                                   const FloatSize &size) noexcept
      : _fromImage(ShareRef(fromImage)), _toImage(ShareRef(toImage)), _percentage(percentage),
        _crossfadeSize(crossfadeSize)
  {
    SetContainerSize(size);
  }

  namespace
  {
    void DrawCrossfadeSubimage(GraphicsContext &context, Image &image, CompositeOperator operation,
                               float opacity, const FloatSize &targetSize) noexcept
    {
      FloatSize imageSize = image.Size();

      // SVGImage resets the opacity when painting, so we have to use transparency layers to accurately paint
      // one at a given opacity.
      bool useTransparencyLayer = image.DrawsSVGImage();

      GraphicsContextStateSaver stateSaver(context);

      ImagePaintingOptions options;

      if (useTransparencyLayer)
      {
        context.SetCompositeOperation(operation);
        context.BeginTransparencyLayer(opacity);
      }
      else
      {
        context.SetAlpha(opacity);
        options = ImagePaintingOptions {operation};
      }

      if (targetSize != imageSize)
      {
        context.Scale(targetSize / imageSize);
      }

      (void)context.DrawImage(image, FloatPoint(), options);

      if (useTransparencyLayer)
      {
        context.EndTransparencyLayer();
      }
    }
  }

  void CrossfadeGeneratedImage::DrawCrossfade(GraphicsContext &context) noexcept
  {
    // Draw nothing if either of the images hasn't loaded yet.
    if (_fromImage.get() == &Image::NullImage() || _toImage.get() == &Image::NullImage())
    {
      return;
    }

    GraphicsContextStateSaver stateSaver(context);

    context.Clip(FloatRect(FloatPoint(), _crossfadeSize));
    context.BeginTransparencyLayer(1);

    DrawCrossfadeSubimage(context, *_fromImage, CompositeOperator::SourceOver, 1 - _percentage,
                          _crossfadeSize);
    DrawCrossfadeSubimage(context, *_toImage, CompositeOperator::PlusLighter, _percentage,
                          _crossfadeSize);

    context.EndTransparencyLayer();
  }

  ImageDrawResult CrossfadeGeneratedImage::Draw(GraphicsContext &context, const FloatRect &dstRect,
                                                const FloatRect &srcRect,
                                                ImagePaintingOptions options) noexcept
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

    DrawCrossfade(context);
    return ImageDrawResult::DidDraw;
  }

  void CrossfadeGeneratedImage::DrawPattern(GraphicsContext &context, const FloatRect &dstRect,
                                            const FloatRect &srcRect, const AffineTransform &patternTransform,
                                            const FloatPoint &phase, const FloatSize &spacing,
                                            ImagePaintingOptions options) noexcept
  {
    auto imageBuffer = context.CreateImageBuffer(Size());
    if (!imageBuffer)
    {
      return;
    }

    // Fill with the cross-faded image.
    DrawCrossfade(imageBuffer->Context());

    // Tile the image buffer into the context.
    context.DrawPattern(*imageBuffer, dstRect, srcRect, patternTransform, phase, spacing, options);
  }
}
