/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/ImageBuffers/ImageBufferContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"

namespace krys::boo
{
  ImageBufferContextSwitcher::ImageBufferContextSwitcher(GraphicsContext &destinationContext,
                                                         const FloatRect &sourceImageRect,
                                                         RefPtr<Filter> &&filter,
                                                         FilterResults *results) noexcept
      : GraphicsContextSwitcher(krys::move(filter)), _sourceImageRect(sourceImageRect), _results(results)
  {
    if (sourceImageRect.IsEmpty())
    {
      return;
    }

    if (_filter)
    {
      _sourceImage = destinationContext.CreateScaledImageBuffer(_sourceImageRect, _filter->FilterScale());
    }
    else
    {
      _sourceImage =
        destinationContext.CreateScaledImageBuffer(_sourceImageRect, destinationContext.ScaleFactor());
    }

    if (!_sourceImage)
    {
      _filter = nullptr;
      return;
    }

    auto state = destinationContext.State();
    _sourceImage->Context().MergeAllChanges(state);
  }

  ImageBufferContextSwitcher::~ImageBufferContextSwitcher() noexcept = default;

  GraphicsContext *ImageBufferContextSwitcher::DrawingContext(GraphicsContext &context) const noexcept
  {
    return _sourceImage ? &_sourceImage->Context() : &context;
  }

  void ImageBufferContextSwitcher::BeginClipAndDrawSourceImage(GraphicsContext &destinationContext,
                                                               const FloatRect &repaintRect,
                                                               const FloatRect &) noexcept
  {
    if (auto *context = DrawingContext(destinationContext))
    {
      context->Save();
      context->ClearRect(repaintRect);
      context->Clip(repaintRect);
    }
  }

  void ImageBufferContextSwitcher::EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept
  {
    if (auto *context = DrawingContext(destinationContext))
    {
      context->Restore();
    }

    EndDrawSourceImage(destinationContext);
  }

  void ImageBufferContextSwitcher::EndDrawSourceImage(GraphicsContext &destinationContext) noexcept
  {
    if (!_filter)
    {
      if (_sourceImage)
      {
        destinationContext.DrawImageBuffer(
          *_sourceImage, _sourceImageRect,
          {destinationContext.CompositeOperation(), destinationContext.GetBlendMode()});
      }
      return;
    }

    FilterResults results;
    destinationContext.DrawFilteredImageBuffer(_sourceImage.get(), _sourceImageRect, *_filter,
                                               _results ? *_results : results);
  }
}
