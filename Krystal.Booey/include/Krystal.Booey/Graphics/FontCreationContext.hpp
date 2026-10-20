#pragma once

#include "Krystal.Booey/Graphics/FontFeatureValues.hpp"
#include "Krystal.Booey/Graphics/FontPaletteValues.hpp"
#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
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
