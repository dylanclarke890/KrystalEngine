/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/SVG/Graphics/SVGResourceImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::svg
{
  Ref<SVGResourceImage> SVGResourceImage::Create(LegacyRenderSVGResourceContainer &renderResource,
                                                 const style::URL &resourceURL) noexcept
  {
    return AdoptRef(*new SVGResourceImage(renderResource, resourceURL));
  }

  SVGResourceImage::SVGResourceImage(LegacyRenderSVGResourceContainer &renderResource,
                                     const style::URL &resourceURL) noexcept
      : _legacyRenderResource(CreateWeakPtr(&renderResource)), _resourceURL(resourceURL)
  {
  }

  SVGResourceImage::~SVGResourceImage() noexcept = default;

  ImageDrawResult SVGResourceImage::Draw(GraphicsContext &context, const FloatRect &destinationRect,
                                         const FloatRect &sourceRect, ImagePaintingOptions options) noexcept
  {
    if (auto *masker = DynamicDowncast<LegacyRenderSVGResourceMasker>(_legacyRenderResource.get()))
    {
      if (masker->DrawContentIntoContext(context, destinationRect, sourceRect, options))
      {
        return ImageDrawResult::DidDraw;
      }
    }

    return ImageDrawResult::DidNothing;
  }

  void SVGResourceImage::DrawPattern(GraphicsContext &context, const FloatRect &destinationRect,
                                     const FloatRect &sourceRect, const AffineTransform &patternTransform,
                                     const FloatPoint &phase, const FloatSize &spacing,
                                     ImagePaintingOptions options) noexcept
  {
    auto imageBuffer = context.CreateImageBuffer(Size());
    if (!imageBuffer)
    {
      return;
    }

    // Fill with the SVG resource.
    GraphicsContext &graphicsContext = imageBuffer->Context();
    (void)graphicsContext.DrawImage(*this, FloatPoint());

    // Tile the image buffer into the context.
    context.DrawPattern(*imageBuffer, destinationRect, sourceRect, patternTransform, phase, spacing, options);
  }

  void SVGResourceImage::Dump(TextStream &ts) const noexcept
  {
    GeneratedImage::Dump(ts);
    ts << _resourceURL.Resolved.String();
  }
}
