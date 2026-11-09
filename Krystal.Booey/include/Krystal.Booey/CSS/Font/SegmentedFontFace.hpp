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

#pragma once

#include "Krystal.Booey/CSS/Font/FontFace.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascadeCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontPaletteValues.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontRanges.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  /// @brief The font faces of a family that match a font selection request, best first, each covering its
  /// `unicode-range`.
  class CSSSegmentedFontFace final : public RefCounted<CSSSegmentedFontFace>, public CSSFontFaceClient
  {
    KRYS_NON_COPY_MOVABLE(CSSSegmentedFontFace);

  private:
    struct CacheKey
    {
      FontDescriptionKey DescriptionKey;
      FontPaletteValues PaletteValues;

      KRYS_NODISCARD bool operator==(const CacheKey &) const noexcept = default;
    };

    struct CacheKeyHash
    {
      KRYS_NODISCARD size_t operator()(const CacheKey &key) const noexcept
      {
        return FontDescriptionKeyHash {}(key.DescriptionKey);
      }
    };

    // FIXME: Add support for font-feature-values in the key for this cache.
    HashMap<CacheKey, FontRanges, CacheKeyHash> _cache;
    SmallList<Ref<CSSFontFace>, 1> _fontFaces;

    CSSSegmentedFontFace() noexcept = default;

  public:
    KRYS_NODISCARD static Ref<CSSSegmentedFontFace> Create() noexcept
    {
      return AdoptRef(*new CSSSegmentedFontFace());
    }

    ~CSSSegmentedFontFace() noexcept override;

    void AddRef() const noexcept final
    {
      RefCounted::AddRef();
    }
    void SubRef() const noexcept final
    {
      RefCounted::SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return RefCounted::GetRefCount();
    }

    void AppendFontFace(Ref<CSSFontFace> &&fontFace) noexcept;

    KRYS_NODISCARD FontRanges GetFontRanges(const FontDescription &fontDescription,
                                            const FontPaletteValues &fontPaletteValues,
                                            RefPtr<FontFeatureValues> fontFeatureValues) noexcept;

    KRYS_NODISCARD SmallList<Ref<CSSFontFace>, 1> &ConstituentFaces() noexcept
    {
      return _fontFaces;
    }

  private:
    void FontLoaded(CSSFontFace &fontFace) noexcept final;
  };
}
