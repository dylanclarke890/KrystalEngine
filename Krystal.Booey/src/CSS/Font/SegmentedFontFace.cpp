/*
 * Copyright (C) 2008-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Font/SegmentedFontFace.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontFeatureValues.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  CSSSegmentedFontFace::~CSSSegmentedFontFace() noexcept
  {
    for (auto &face : _fontFaces)
    {
      face->RemoveClient(*this);
    }
  }

  void CSSSegmentedFontFace::AppendFontFace(Ref<CSSFontFace> &&fontFace) noexcept
  {
    _cache.clear();
    fontFace->AddClient(*this);
    _fontFaces.push_back(krys::move(fontFace));
  }

  void CSSSegmentedFontFace::FontLoaded(CSSFontFace &) noexcept
  {
    _cache.clear();
  }

  namespace
  {
    class CSSFontAccessor final : public FontAccessor
    {
    private:
      mutable Maybe<RefPtr<Font>> _result; // Caches nullptr too
      mutable Ref<CSSFontFace> _fontFace;
      FontDescription _fontDescription;
      RefPtr<FontFeatureValues> _fontFeatureValues;
      FontPaletteValues _fontPaletteValues;
      bool _syntheticBold;
      bool _syntheticItalic;

      CSSFontAccessor(CSSFontFace &fontFace, const FontDescription &fontDescription,
                      const FontPaletteValues &fontPaletteValues, RefPtr<FontFeatureValues> fontFeatureValues,
                      bool syntheticBold, bool syntheticItalic) noexcept
          : _fontFace(ShareRef(fontFace)), _fontDescription(fontDescription),
            _fontFeatureValues(krys::move(fontFeatureValues)), _fontPaletteValues(fontPaletteValues),
            _syntheticBold(syntheticBold), _syntheticItalic(syntheticItalic)
      {
      }

    public:
      KRYS_NODISCARD static Ref<CSSFontAccessor> Create(CSSFontFace &fontFace,
                                                        const FontDescription &fontDescription,
                                                        const FontPaletteValues &fontPaletteValues,
                                                        RefPtr<FontFeatureValues> fontFeatureValues,
                                                        bool syntheticBold, bool syntheticItalic) noexcept
      {
        return AdoptRef(*new CSSFontAccessor(fontFace, fontDescription, fontPaletteValues,
                                             krys::move(fontFeatureValues), syntheticBold, syntheticItalic));
      }

      KRYS_NODISCARD const Font *GetFont(ExternalResourceDownloadPolicy policy) const noexcept final
      {
        auto status = _fontFace->GetStatus();
        if (!_result
            || (policy == ExternalResourceDownloadPolicy::Allow
                && (status == CSSFontFace::Status::Pending || status == CSSFontFace::Status::Loading
                    || status == CSSFontFace::Status::TimedOut)))
        {
          auto result = _fontFace->GetFont(_fontDescription, _syntheticBold, _syntheticItalic, policy,
                                           _fontPaletteValues, _fontFeatureValues);
          if (!_result)
          {
            _result = krys::move(result);
          }
        }
        return _result.value().get();
      }

    private:
      KRYS_NODISCARD bool IsLoading() const noexcept final
      {
        return _result && _result.value() && _result.value()->IsInterstitial();
      }
    };

    void AppendFont(FontRanges &ranges, Ref<FontAccessor> &&fontAccessor,
                    Span<const CSSFontFace::UnicodeRange> unicodeRanges) noexcept
    {
      if (unicodeRanges.empty())
      {
        ranges.AppendRange({0, 0x7F'FF'FF'FF, krys::move(fontAccessor)});
        return;
      }

      for (auto &range : unicodeRanges)
      {
        ranges.AppendRange({range.From, range.To, Ref<FontAccessor>(fontAccessor)});
      }
    }
  }

  FontRanges CSSSegmentedFontFace::GetFontRanges(const FontDescription &fontDescription,
                                                 const FontPaletteValues &fontPaletteValues,
                                                 RefPtr<FontFeatureValues> fontFeatureValues) noexcept
  {
    auto [iterator, isNewEntry] =
      _cache.try_emplace(CacheKey {FontDescriptionKey(fontDescription), fontPaletteValues}, FontRanges());
    auto &ranges = iterator->second;
    if (!isNewEntry)
    {
      return ranges;
    }

    auto desiredRequest = fontDescription.GetFontSelectionRequest();

    for (auto &face : _fontFaces)
    {
      if (face->ComputeFailureState())
      {
        continue;
      }

      auto selectionCapabilities = face->GetFontSelectionCapabilities();

      bool syntheticBold = fontDescription.HasAutoFontSynthesisWeight()
                           && !IsFontWeightBold(selectionCapabilities.Weight.Maximum)
                           && IsFontWeightBold(desiredRequest.Weight);
      bool syntheticItalic = fontDescription.HasAutoFontSynthesisStyle()
                             && !IsItalic(selectionCapabilities.Slope.Maximum)
                             && IsItalic(desiredRequest.Slope);

      // Metrics used for layout come from FontRanges::fontForFirstRange(), which assumes that the first font
      // is non-null.
      auto fontAccessor = CSSFontAccessor::Create(*face, fontDescription, fontPaletteValues,
                                                  fontFeatureValues, syntheticBold, syntheticItalic);
      if (ranges.IsNull() && !fontAccessor->GetFont(ExternalResourceDownloadPolicy::Forbid))
      {
        continue;
      }

      AppendFont(ranges, krys::move(fontAccessor), face->Ranges());
    }

    return ranges;
  }
}
