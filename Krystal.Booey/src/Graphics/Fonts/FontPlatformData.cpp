#include "Krystal.Booey/Graphics/Fonts/FontPlatformData.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCreationContext.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontDescription.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <hb-ot.h>
#include <hb.h>

namespace krys::boo
{
  FontPlatformData::FontPlatformData(HashTableDeletedValueTag) noexcept : _isHashTableDeletedValue(true)
  {
  }

  FontPlatformData::FontPlatformData() noexcept = default;

  FontPlatformData::FontPlatformData(Ref<text::FreeTypeFace> face, float size, bool syntheticBold,
                                     bool syntheticOblique, FontOrientation orientation,
                                     FontWidthVariant widthVariant,
                                     TextRenderingMode textRenderingMode) noexcept
      : _face(krys::move(face)), _size(size), _orientation(orientation), _widthVariant(widthVariant),
        _textRenderingMode(textRenderingMode), _syntheticBold(syntheticBold),
        _syntheticOblique(syntheticOblique)
  {
  }

  FontPlatformData::FontPlatformData(const FontPlatformData &) noexcept = default;
  FontPlatformData &FontPlatformData::operator=(const FontPlatformData &) noexcept = default;
  FontPlatformData::~FontPlatformData() noexcept = default;

  FontPlatformData FontPlatformData::CloneWithOrientation(const FontPlatformData &source,
                                                          FontOrientation orientation) noexcept
  {
    FontPlatformData copy(source);
    copy._orientation = orientation;
    return copy;
  }

  FontPlatformData FontPlatformData::CloneWithSyntheticOblique(const FontPlatformData &source,
                                                               bool syntheticOblique) noexcept
  {
    FontPlatformData copy(source);
    copy._syntheticOblique = syntheticOblique;
    return copy;
  }

  FontPlatformData FontPlatformData::CloneWithSize(const FontPlatformData &source, float size) noexcept
  {
    FontPlatformData copy(source);
    copy.UpdateSize(size);
    return copy;
  }

  void FontPlatformData::UpdateSizeWithFontSizeAdjust(const FontSizeAdjust &fontSizeAdjust,
                                                      float computedSize) noexcept
  {
    if (!fontSizeAdjust.Value || !_size)
    {
      return;
    }

    // The metric scales with the size, so the size that gives it the wanted value follows from its value at
    // this size.
    auto font = FontCache::Singleton().FontForPlatformData(*this);
    auto aspectValue = fontSizeAdjust.Resolve(_size, font->GetFontMetrics());
    // FIXME: The behavior for missing metrics has yet to be defined.
    // https://github.com/w3c/csswg-drafts/issues/6384
    if (!aspectValue || !*aspectValue)
    {
      return;
    }

    auto adjustedSize = computedSize * *fontSizeAdjust.Value / *aspectValue;
    if (adjustedSize != _size)
    {
      UpdateSize(std::min(adjustedSize, MaximumAllowedFontSize));
    }
  }

  void FontPlatformData::UpdateSize(float size) noexcept
  {
    // Glyphs are measured in font units and scaled, so a face has no size of its own.
    _size = size;
  }

  bool FontPlatformData::IsFixedPitch() const noexcept
  {
    return _face && _face->IsFixedPitch();
  }

  utf8_string FontPlatformData::FamilyName() const noexcept
  {
    return _face ? _face->FamilyName() : utf8_string {};
  }

  namespace
  {
    /// @brief The first palette flagged for @p flag, if any.
    KRYS_NODISCARD Maybe<uint32> FlaggedPalette(hb_face_t *face, uint32 count,
                                                hb_ot_color_palette_flags_t flag) noexcept
    {
      for (uint32 index = 0; index < count; ++index)
      {
        if (hb_ot_color_palette_get_flags(face, index) & flag)
        {
          return index;
        }
      }
      return std::nullopt;
    }
  }

  void FontPlatformData::ApplyCreationContext(const FontDescription &description,
                                              const FontCreationContext &context) noexcept
  {
    _fontFaceFeatures = context.FontFaceFeatures() ? *context.FontFaceFeatures() : FontFeatureSettings();
    _fontFeatureValues = context.GetFontFeatureValues();
    _paletteIndex = 0;
    _paletteOverrides.clear();

    auto *face = _face ? _face->HarfBuzzFace() : nullptr;
    const uint32 count = face ? hb_ot_color_palette_get_count(face) : 0u;
    if (!count)
    {
      return;
    }

    // `light` and `dark` are the first palette flagged for them, or else palette 0; a palette the font
    // doesn't have is palette 0 too (as Chrome).
    auto resolve = [&](const FontPaletteIndex &index) -> uint32
    {
      switch (index.IndexType)
      {
        case FontPaletteIndex::Type::Light:
          return FlaggedPalette(face, count, HB_OT_COLOR_PALETTE_FLAG_USABLE_WITH_LIGHT_BACKGROUND)
            .value_or(0u);
        case FontPaletteIndex::Type::Dark:
          return FlaggedPalette(face, count, HB_OT_COLOR_PALETTE_FLAG_USABLE_WITH_DARK_BACKGROUND)
            .value_or(0u);
        case FontPaletteIndex::Type::Integer: return index.Integer < count ? index.Integer : 0u;
      }
      return 0u;
    };

    switch (description.GetFontPalette().PaletteType)
    {
      case FontPalette::Type::Normal: break;
      case FontPalette::Type::Light:  _paletteIndex = resolve(FontPaletteIndex::Type::Light); break;
      case FontPalette::Type::Dark:   _paletteIndex = resolve(FontPaletteIndex::Type::Dark); break;
      case FontPalette::Type::Custom:
      {
        // A name no `@font-palette-values` rule gives this family is `normal`.
        const auto *values = context.GetFontPaletteValues();
        if (!values)
        {
          break;
        }
        if (auto base = values->BasePalette())
        {
          _paletteIndex = resolve(*base);
        }
        // Entries the palette doesn't have are ignored.
        const uint32 entries = hb_ot_color_palette_get_colors(face, _paletteIndex, 0, nullptr, nullptr);
        for (const auto &[index, colour] : values->OverrideColors())
        {
          if (index < entries)
          {
            _paletteOverrides.emplace_back(index, colour);
          }
        }
        break;
      }
    }
  }

  size_t FontPlatformData::Hash() const noexcept
  {
    return Hash::Combine(static_cast<const void *>(_face.get()), _size, static_cast<uint8>(_orientation),
                         static_cast<uint8>(_widthVariant), static_cast<uint8>(_textRenderingMode),
                         _syntheticBold, _syntheticOblique, _isHashTableDeletedValue, _paletteIndex,
                         _paletteOverrides.size(), _fontFaceFeatures.size(),
                         static_cast<const void *>(_fontFeatureValues.get()));
  }

  bool FontPlatformData::operator==(const FontPlatformData &other) const noexcept
  {
    return _face == other._face && _isHashTableDeletedValue == other._isHashTableDeletedValue
           && _size == other._size && _syntheticBold == other._syntheticBold
           && _syntheticOblique == other._syntheticOblique && _isColorBitmapFont == other._isColorBitmapFont
           && _orientation == other._orientation && _widthVariant == other._widthVariant
           && _textRenderingMode == other._textRenderingMode && _paletteIndex == other._paletteIndex
           && _paletteOverrides == other._paletteOverrides && _fontFaceFeatures == other._fontFaceFeatures
           && _fontFeatureValues == other._fontFeatureValues;
  }

  text::HbUniquePtr<hb_font_t> FontPlatformData::CreateOpenTypeMathHarfBuzzFont() const noexcept
  {
    if (!_face)
    {
      return nullptr;
    }

    auto *face = _face->HarfBuzzFace();
    if (!face || !hb_ot_math_has_data(face))
    {
      return nullptr;
    }

    text::HbUniquePtr<hb_font_t> font(hb_font_create(face));
    _face->ApplyVariations(font.get());
    return font;
  }

  Maybe<List<byte>> FontPlatformData::OpenTypeTable(uint32 table) const noexcept
  {
    if (!_face)
    {
      return null;
    }
    return _face->OpenTypeTable(table);
  }

  utf8_string FontPlatformData::Description() const noexcept
  {
    StringBuilder description;
    description.Append(FamilyName(), u8' ', _size, u8"px");
    if (_syntheticBold)
    {
      description.Append(u8" synthetic-bold");
    }
    if (_syntheticOblique)
    {
      description.Append(u8" synthetic-oblique");
    }
    return krys::move(description).ToString();
  }
}
