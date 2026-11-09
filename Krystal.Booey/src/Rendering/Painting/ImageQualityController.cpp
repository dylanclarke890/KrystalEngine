/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Painting/ImageQualityController.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  Maybe<InterpolationQuality>
    ImageQualityController::InterpolationQualityFromStyle(const RenderStyle &style) noexcept
  {
    switch (style.GetImageRendering())
    {
      case ImageRendering::OptimizeSpeed:   return InterpolationQuality::Low;
      case ImageRendering::CrispEdges:
      case ImageRendering::Pixelated:       return InterpolationQuality::DoNotInterpolate;
      // FIXME: CSS 3 Images says that optimizeQuality should behave like 'auto', but that prevents authors
      // from overriding this low quality rendering behavior.
      case ImageRendering::OptimizeQuality: return InterpolationQuality::Default;
      case ImageRendering::Auto:            break;
    }
    return null;
  }

  InterpolationQuality ImageQualityController::ChooseInterpolationQualityForSVG(
    GraphicsContext &context, const RenderElement &renderElement, Image &image) noexcept
  {
    return ChooseInterpolationQuality(context, renderElement, image);
  }

  InterpolationQuality ImageQualityController::ChooseInterpolationQuality(GraphicsContext &context,
                                                                          const RenderElement &renderer,
                                                                          Image &image) noexcept
  {
    // If the image is not a bitmap image, then none of this is relevant and we just paint at high quality.
    if (!image.IsBitmapImage() || context.PaintingDisabled())
    {
      return InterpolationQuality::Default;
    }

    if (auto styleInterpolation = InterpolationQualityFromStyle(renderer.Style()))
    {
      return *styleInterpolation;
    }

    return InterpolationQuality::Default;
  }
}
