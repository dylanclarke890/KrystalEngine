/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Graphics/Pattern.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<Pattern> Pattern::Create(SourceImage &&tileImage, const Parameters &parameters) noexcept
  {
    return AdoptRef(*new Pattern(krys::move(tileImage), parameters));
  }

  Pattern::Pattern(SourceImage &&tileImage, const Parameters &parameters) noexcept
      : _tileImage(krys::move(tileImage)), _parameters(parameters)
  {
  }

  Pattern::~Pattern() noexcept = default;

  void Pattern::SetPatternSpaceTransform(const AffineTransform &patternSpaceTransform) noexcept
  {
    _parameters.PatternSpaceTransform = patternSpaceTransform;
  }

  const SourceImage &Pattern::TileImage() const noexcept
  {
    return _tileImage;
  }

  RefPtr<NativeImage> Pattern::TileNativeImage() const noexcept
  {
    return ShareRefPtr(_tileImage.GetNativeImage());
  }

  RefPtr<ImageBuffer> Pattern::TileImageBuffer() const noexcept
  {
    return ShareRefPtr(_tileImage.GetImageBuffer());
  }

  void Pattern::SetTileImage(SourceImage &&tileImage) noexcept
  {
    _tileImage = krys::move(tileImage);
  }

  TextStream &operator<<(TextStream &ts, const Pattern &pattern) noexcept
  {
    ts.DumpProperty(u8"tile-size", pattern.TileImage().Size());
    ts.DumpProperty(u8"pattern-space-transform", pattern.PatternSpaceTransform());
    return ts;
  }
}
