/*
 * Copyright (C) 2008, 2009, 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Fonts/FontRanges.hpp"
#include "Krystal.Booey/Text/CharacterProperties.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  const Font *FontRanges::Range::GetFont(ExternalResourceDownloadPolicy policy) const noexcept
  {
    return _fontAccessor->GetFont(policy);
  }

  FontRanges::FontRanges(FontRanges &&other, boo::IsGenericFontFamily isGenericFontFamily) noexcept
      : _ranges {krys::move(other._ranges)}, _isGenericFontFamily {isGenericFontFamily}
  {
  }

  /// @brief Gives a font that's always available.
  class TrivialFontAccessor final : public FontAccessor
  {
  private:
    RefPtr<Font> _font;

    explicit TrivialFontAccessor(RefPtr<Font> &&font) noexcept : _font(krys::move(font))
    {
    }

  public:
    KRYS_NODISCARD static Ref<TrivialFontAccessor> Create(Ref<Font> &&font) noexcept
    {
      return AdoptRef(*new TrivialFontAccessor(krys::move(font)));
    }

    KRYS_NODISCARD const Font *GetFont(ExternalResourceDownloadPolicy) const noexcept final
    {
      return _font.get();
    }

    KRYS_NODISCARD bool IsLoading() const noexcept final
    {
      return _font->IsInterstitial();
    }
  };

  FontRanges::FontRanges(RefPtr<Font> &&font) noexcept
  {
    if (font)
    {
      Ref<Font> nonNull = krys::move(font);
      _ranges.push_back(Range {0, 0x7F'FF'FF'FF, TrivialFontAccessor::Create(krys::move(nonNull))});
    }
  }

  FontRanges::~FontRanges() noexcept = default;

  GlyphData FontRanges::GlyphDataForCharacter(char32 character,
                                              ExternalResourceDownloadPolicy policy) const noexcept
  {
    const Font *resultFont = nullptr;
    if (IsGenericFontFamily() && IsPrivateUseAreaCharacter(character))
    {
      return GlyphData();
    }

    for (auto &range : _ranges)
    {
      if (range.From() <= character && character <= range.To())
      {
        if (auto *font = range.GetFont(policy))
        {
          if (font->IsInterstitial())
          {
            policy = ExternalResourceDownloadPolicy::Forbid;
            if (!resultFont)
            {
              resultFont = font;
            }
          }
          else
          {
            auto glyphData = font->GlyphDataForCharacter(character);
            if (glyphData.IsValid())
            {
              auto *glyphDataFont = glyphData.Font.get();
              if (glyphDataFont && glyphDataFont->GetVisibility() == FontVisibility::Visible && resultFont
                  && resultFont->GetVisibility() == FontVisibility::Invisible)
              {
                return GlyphData(glyphData.Glyph, &glyphDataFont->InvisibleFont());
              }
              return glyphData;
            }
          }
        }
      }
    }
    if (resultFont)
    {
      // We want higher-level code to be able to differentiate between
      // "The interstitial font doesn't have the character" and
      // "The real downloaded font doesn't have the character".
      GlyphData result = resultFont->GlyphDataForCharacter(character);
      if (!result.Font)
      {
        result.Font = CreateWeakPtr(resultFont);
      }
      return result;
    }
    return GlyphData();
  }

  const Font *FontRanges::FontForCharacter(char32 character) const noexcept
  {
    return GlyphDataForCharacter(character, ExternalResourceDownloadPolicy::Allow).Font.get();
  }

  const Font &FontRanges::FontForFirstRange() const noexcept
  {
    auto *font = _ranges[0].GetFont(ExternalResourceDownloadPolicy::Forbid);
    krys_debug_assert(font);
    return *font;
  }

  bool FontRanges::IsLoading() const noexcept
  {
    for (auto &range : _ranges)
    {
      if (range.GetFontAccessor().IsLoading())
      {
        return true;
      }
    }
    return false;
  }
}
