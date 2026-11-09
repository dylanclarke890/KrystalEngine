/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontFeatureValues.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontPaletteValues.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontTaggedSettings.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief What a font face says about the fonts made from it, when it says anything: its features,
  /// palette, feature values and size adjustment.
  class FontCreationContextRareData : public RefCounted<FontCreationContextRareData>
  {
  private:
    FontFeatureSettings _fontFaceFeatures;
    FontPaletteValues _fontPaletteValues;
    RefPtr<FontFeatureValues> _fontFeatureValues;
    float _sizeAdjust;

    FontCreationContextRareData(const FontFeatureSettings &fontFaceFeatures,
                                const FontPaletteValues &fontPaletteValues,
                                RefPtr<FontFeatureValues> fontFeatureValues, float sizeAdjust) noexcept
        : _fontFaceFeatures(fontFaceFeatures), _fontPaletteValues(fontPaletteValues),
          _fontFeatureValues(krys::move(fontFeatureValues)), _sizeAdjust(sizeAdjust)
    {
    }

  public:
    KRYS_NODISCARD static Ref<FontCreationContextRareData> Create(const FontFeatureSettings &fontFaceFeatures,
                                                                  const FontPaletteValues &fontPaletteValues,
                                                                  RefPtr<FontFeatureValues> fontFeatureValues,
                                                                  float sizeAdjust) noexcept
    {
      return AdoptRef(*new FontCreationContextRareData(fontFaceFeatures, fontPaletteValues,
                                                       krys::move(fontFeatureValues), sizeAdjust));
    }

    KRYS_NODISCARD const FontFeatureSettings &FontFaceFeatures() const noexcept
    {
      return _fontFaceFeatures;
    }

    KRYS_NODISCARD float SizeAdjust() const noexcept
    {
      return _sizeAdjust;
    }

    KRYS_NODISCARD const FontPaletteValues &GetFontPaletteValues() const noexcept
    {
      return _fontPaletteValues;
    }

    KRYS_NODISCARD RefPtr<FontFeatureValues> GetFontFeatureValues() const noexcept
    {
      return _fontFeatureValues;
    }

    KRYS_NODISCARD bool operator==(const FontCreationContextRareData &other) const noexcept
    {
      return _fontFaceFeatures == other._fontFaceFeatures && _fontPaletteValues == other._fontPaletteValues
             && _fontFeatureValues.get() == other._fontFeatureValues.get()
             && _sizeAdjust == other._sizeAdjust;
    }
  };

  /// @brief How fonts are made from a font face: its capabilities, and the rest of its descriptors.
  class FontCreationContext
  {
  private:
    FontSelectionSpecifiedCapabilities _fontFaceCapabilities;
    RefPtr<FontCreationContextRareData> _rareData;

  public:
    FontCreationContext() noexcept = default;

    FontCreationContext(const FontFeatureSettings &fontFaceFeatures,
                        const FontSelectionSpecifiedCapabilities &fontFaceCapabilities,
                        const FontPaletteValues &fontPaletteValues,
                        RefPtr<FontFeatureValues> fontFeatureValues, float sizeAdjust) noexcept
        : _fontFaceCapabilities(fontFaceCapabilities)
    {
      if (!fontFaceFeatures.isEmpty() || fontPaletteValues
          || (fontFeatureValues && !fontFeatureValues->IsEmpty()) || sizeAdjust != 1.0f)
      {
        _rareData = FontCreationContextRareData::Create(fontFaceFeatures, fontPaletteValues,
                                                        krys::move(fontFeatureValues), sizeAdjust);
      }
    }

    KRYS_NODISCARD const FontFeatureSettings *FontFaceFeatures() const noexcept
    {
      return _rareData ? &_rareData->FontFaceFeatures() : nullptr;
    }

    KRYS_NODISCARD float SizeAdjust() const noexcept
    {
      return _rareData ? _rareData->SizeAdjust() : 1.0f;
    }

    KRYS_NODISCARD const FontSelectionSpecifiedCapabilities &FontFaceCapabilities() const noexcept
    {
      return _fontFaceCapabilities;
    }

    KRYS_NODISCARD const FontPaletteValues *GetFontPaletteValues() const noexcept
    {
      return _rareData ? &_rareData->GetFontPaletteValues() : nullptr;
    }

    KRYS_NODISCARD RefPtr<FontFeatureValues> GetFontFeatureValues() const noexcept
    {
      return _rareData ? _rareData->GetFontFeatureValues() : nullptr;
    }

    KRYS_NODISCARD bool operator==(const FontCreationContext &other) const noexcept
    {
      if (_fontFaceCapabilities != other._fontFaceCapabilities)
      {
        return false;
      }
      if (!_rareData || !other._rareData)
      {
        return !_rareData && !other._rareData;
      }
      return *_rareData == *other._rareData;
    }
  };
}
