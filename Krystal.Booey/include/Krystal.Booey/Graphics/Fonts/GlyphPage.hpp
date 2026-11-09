/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
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
#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <bitset>

namespace krys::boo
{
  class Font;

  /// @brief Holds the glyph index and the corresponding Font information for a given character.
  struct GlyphData
  {
    boo::Glyph Glyph;
    boo::ColorGlyphType ColorGlyphType;
    WeakPtr<const boo::Font> Font;

    GlyphData(boo::Glyph glyph = 0, const boo::Font *font = nullptr,
              boo::ColorGlyphType colorGlyphType = boo::ColorGlyphType::Outline) noexcept
        : Glyph(glyph), ColorGlyphType(colorGlyphType), Font(CreateWeakPtr(font))
    {
    }

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return !!Font;
    }
  };

  /// @brief A GlyphPage contains a fixed-size set of GlyphData mappings for a contiguous range of characters
  /// in the Unicode code space. GlyphPages are indexed starting from 0 and incrementing for each "size"
  /// number of glyphs.
  class GlyphPage final : public RefCounted<GlyphPage>
  {
    KRYS_NON_COPY_MOVABLE(GlyphPage);

  public:
    static constexpr uint32 Size = 16;

  private:
    WeakPtr<const Font> _font;
    Array<Glyph, Size> _glyphs {};
    std::bitset<Size> _isColor;

    explicit GlyphPage(const Font &font) noexcept : _font(CreateWeakPtr(&font))
    {
    }

  public:
    KRYS_NODISCARD static Ref<GlyphPage> Create(const Font &font) noexcept
    {
      return AdoptRef(*new GlyphPage(font));
    }

    KRYS_NODISCARD static constexpr uint32 SizeForPageNumber(uint32) noexcept
    {
      return Size;
    }

    KRYS_NODISCARD static constexpr uint32 IndexForCodePoint(char32 c) noexcept
    {
      return c % Size;
    }

    KRYS_NODISCARD static constexpr uint32 PageNumberForCodePoint(char32 c) noexcept
    {
      return c / Size;
    }

    KRYS_NODISCARD static constexpr char32 StartingCodePointInPageNumber(uint32 pageNumber) noexcept
    {
      return pageNumber * Size;
    }

    KRYS_NODISCARD static constexpr bool PageNumberIsUsedForArabic(uint32 pageNumber) noexcept
    {
      return StartingCodePointInPageNumber(pageNumber) >= 0x600
             && StartingCodePointInPageNumber(pageNumber) + SizeForPageNumber(pageNumber) < 0x700;
    }

    KRYS_NODISCARD GlyphData GlyphDataForCharacter(char32 c) const noexcept
    {
      return GlyphDataForIndex(IndexForCodePoint(c));
    }

    KRYS_NODISCARD Glyph GlyphForCharacter(char32 c) const noexcept
    {
      return GlyphForIndex(IndexForCodePoint(c));
    }

    KRYS_NODISCARD GlyphData GlyphDataForIndex(uint32 index) const noexcept
    {
      Glyph glyph = GlyphForIndex(index);
      auto colorGlyphType = ColorGlyphTypeForIndex(index);
      return GlyphData(glyph, glyph ? _font.get() : nullptr, colorGlyphType);
    }

    KRYS_NODISCARD Glyph GlyphForIndex(uint32 index) const noexcept
    {
      krys_assert(index < Size);
      return _glyphs[index];
    }

    KRYS_NODISCARD ColorGlyphType ColorGlyphTypeForIndex(uint32 index) const noexcept
    {
      krys_assert(index < Size);
      return _isColor.test(index) ? ColorGlyphType::Color : ColorGlyphType::Outline;
    }

    // FIXME: Pages are immutable after initialization. This should be private.
    void SetGlyphForIndex(uint32 index, Glyph glyph, ColorGlyphType colorGlyphType) noexcept
    {
      krys_assert(index < Size);
      _glyphs[index] = glyph;
      _isColor.set(index, colorGlyphType == ColorGlyphType::Color);
    }

    KRYS_NODISCARD const Font &GetFont() const noexcept
    {
      return *_font;
    }

    /// @brief Fills the page with the glyphs of @p codePoints, the code points of the page in order.
    /// @return Whether the font has any of them.
    bool Fill(Span<const char32> codePoints) noexcept;
  };
}
