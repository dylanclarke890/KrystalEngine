#include "Krystal.Booey/Graphics/FontPlatformData.hpp"
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

  size_t FontPlatformData::Hash() const noexcept
  {
    return Hash::Combine(static_cast<const void *>(_face.get()), _size, static_cast<uint8>(_orientation),
                         static_cast<uint8>(_widthVariant), static_cast<uint8>(_textRenderingMode),
                         _syntheticBold, _syntheticOblique, _isHashTableDeletedValue);
  }

  bool FontPlatformData::operator==(const FontPlatformData &other) const noexcept
  {
    return _face == other._face && _isHashTableDeletedValue == other._isHashTableDeletedValue
           && _size == other._size && _syntheticBold == other._syntheticBold
           && _syntheticOblique == other._syntheticOblique && _isColorBitmapFont == other._isColorBitmapFont
           && _orientation == other._orientation && _widthVariant == other._widthVariant
           && _textRenderingMode == other._textRenderingMode;
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

    return text::HbUniquePtr<hb_font_t>(hb_font_create(face));
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
