/*
 * Copyright (C) 2006-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2008 Torch Mobile Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/Glyph.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"

namespace krys::boo
{
  // When you write code that interacts with these types, use the functions below!
  using GlyphBufferGlyph = Glyph;
  using GlyphBufferAdvance = FloatSize;
  using GlyphBufferOrigin = FloatPoint;
  /// @brief A byte offset into the run's UTF-8 text.
  using GlyphBufferStringOffset = uint32;

  KRYS_NODISCARD constexpr GlyphBufferAdvance MakeGlyphBufferAdvance(const FloatSize &size) noexcept
  {
    return size;
  }

  KRYS_NODISCARD constexpr GlyphBufferAdvance MakeGlyphBufferAdvance(float width = 0,
                                                                     float height = 0) noexcept
  {
    return FloatSize(width, height);
  }

  KRYS_NODISCARD constexpr FloatSize Size(const GlyphBufferAdvance &advance) noexcept
  {
    return advance;
  }

  constexpr void SetWidth(GlyphBufferAdvance &advance, float width) noexcept
  {
    advance.SetWidth(width);
  }

  constexpr void SetHeight(GlyphBufferAdvance &advance, float height) noexcept
  {
    advance.SetHeight(height);
  }

  KRYS_NODISCARD constexpr float Width(const GlyphBufferAdvance &advance) noexcept
  {
    return advance.Width();
  }

  KRYS_NODISCARD constexpr float Height(const GlyphBufferAdvance &advance) noexcept
  {
    return advance.Height();
  }

  KRYS_NODISCARD constexpr GlyphBufferOrigin MakeGlyphBufferOrigin(const FloatPoint &point) noexcept
  {
    return point;
  }

  KRYS_NODISCARD constexpr GlyphBufferOrigin MakeGlyphBufferOrigin(float x = 0, float y = 0) noexcept
  {
    return FloatPoint(x, y);
  }

  KRYS_NODISCARD constexpr FloatPoint Point(const GlyphBufferOrigin &origin) noexcept
  {
    return origin;
  }

  constexpr void SetX(GlyphBufferOrigin &origin, float x) noexcept
  {
    origin.SetX(x);
  }

  constexpr void SetY(GlyphBufferOrigin &origin, float y) noexcept
  {
    origin.SetY(y);
  }

  KRYS_NODISCARD constexpr float X(const GlyphBufferOrigin &origin) noexcept
  {
    return origin.X();
  }

  KRYS_NODISCARD constexpr float Y(const GlyphBufferOrigin &origin) noexcept
  {
    return origin.Y();
  }
}
