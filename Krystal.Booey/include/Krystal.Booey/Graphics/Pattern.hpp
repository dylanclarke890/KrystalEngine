/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2007-2008 Torch Mobile, Inc.
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

#pragma once

#include "Krystal.Booey/Graphics/Images/SourceImage.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief Whether a pattern repeats along each axis, and the transform from its space to the user space.
  struct PatternParameters
  {
    bool RepeatX {true};
    bool RepeatY {true};
    AffineTransform PatternSpaceTransform;
  };

  /// @brief An image tiled as a brush.
  class Pattern final : public RefCounted<Pattern>
  {
  public:
    using Parameters = PatternParameters;

  private:
    SourceImage _tileImage;
    Parameters _parameters;

  public:
    KRYS_NODISCARD static Ref<Pattern> Create(SourceImage &&tileImage, const Parameters & = {}) noexcept;
    ~Pattern() noexcept;

    KRYS_NODISCARD const SourceImage &TileImage() const noexcept;
    void SetTileImage(SourceImage &&) noexcept;

    KRYS_NODISCARD RefPtr<NativeImage> TileNativeImage() const noexcept;
    KRYS_NODISCARD RefPtr<ImageBuffer> TileImageBuffer() const noexcept;

    KRYS_NODISCARD const Parameters &GetParameters() const noexcept
    {
      return _parameters;
    }

    void SetPatternSpaceTransform(const AffineTransform &) noexcept;

    KRYS_NODISCARD const AffineTransform &PatternSpaceTransform() const noexcept
    {
      return _parameters.PatternSpaceTransform;
    }
    KRYS_NODISCARD bool RepeatX() const noexcept
    {
      return _parameters.RepeatX;
    }
    KRYS_NODISCARD bool RepeatY() const noexcept
    {
      return _parameters.RepeatY;
    }

  private:
    Pattern(SourceImage &&, const Parameters &) noexcept;
  };

  TextStream &operator<<(TextStream &, const Pattern &) noexcept;
}
