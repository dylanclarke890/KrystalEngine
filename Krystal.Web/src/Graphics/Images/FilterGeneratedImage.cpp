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

#include "Krystal.Web/Graphics/Images/FilterGeneratedImage.hpp"
#include "Krystal.Web/Graphics/GraphicsContext.hpp"
#include "Krystal.Web/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Web/Graphics/ImageBuffers/ImageBuffer.hpp"

namespace krys::web
{
  FilterGeneratedImage::FilterGeneratedImage(Image &image, Filter &filter, const SizeF &size) noexcept
      : _image(ShareRef(image)), _filter(ShareRef(filter))
  {
    SetContainerSize(size);
  }

  void FilterGeneratedImage::DrawFiltered(GraphicsContext &context) noexcept
  {
    RectF sourceImageRect {PointF(), Size()};
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

  ImageDrawResult FilterGeneratedImage::Draw(GraphicsContext &context, const RectF &dstRect,
                                             const RectF &srcRect, ImagePaintingOptions options) noexcept
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

  void FilterGeneratedImage::DrawPattern(GraphicsContext &context, const RectF &dstRect, const RectF &srcRect,
                                         const AffineTransform &patternTransform, const PointF &phase,
                                         const SizeF &spacing, ImagePaintingOptions options) noexcept
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
