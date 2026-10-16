#include "Krystal.Booey/Graphics/FreeTypeFace.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

namespace krys::boo
{
  /// @brief The FreeType library every face is opened with. It's never closed.
  KRYS_NODISCARD static FT_Library Library() noexcept
  {
    static FT_Library library = []
    {
      FT_Library result = nullptr;
      [[maybe_unused]] auto error = FT_Init_FreeType(&result);
      krys_assert_msg(!error, "FreeType couldn't be initialised");
      return result;
    }();
    return library;
  }

  // MARK: - FontFileData

  FontFileData::FontFileData(List<byte> &&owned) noexcept : _owned(krys::move(owned)), _bytes(_owned)
  {
  }

  FontFileData::FontFileData(Span<const byte> external) noexcept : _bytes(external)
  {
  }

  Ref<FontFileData> FontFileData::Create(List<byte> &&bytes) noexcept
  {
    return AdoptRef(*new FontFileData(krys::move(bytes)));
  }

  Ref<FontFileData> FontFileData::CreateStatic(Span<const byte> bytes) noexcept
  {
    return AdoptRef(*new FontFileData(bytes));
  }

  // MARK: - FreeTypeFace

  KRYS_NODISCARD static FT_Face OpenFace(const FontFileData &data, FT_Long faceIndex) noexcept
  {
    auto bytes = data.Bytes();
    FT_Face face = nullptr;
    if (FT_New_Memory_Face(Library(), reinterpret_cast<const FT_Byte *>(bytes.data()),
                           static_cast<FT_Long>(bytes.size()), faceIndex, &face))
    {
      return nullptr;
    }
    return face;
  }

  /// @brief The font-stretch percentage of an OS/2 width class.
  /// @see https://learn.microsoft.com/typography/opentype/spec/os2#uswidthclass
  KRYS_NODISCARD static float WidthForWidthClass(FT_UShort widthClass) noexcept
  {
    switch (widthClass)
    {
      case 1:  return 50;
      case 2:  return 62.5f;
      case 3:  return 75;
      case 4:  return 87.5f;
      case 6:  return 112.5f;
      case 7:  return 125;
      case 8:  return 150;
      case 9:  return 200;
      default: return 100;
    }
  }

  KRYS_NODISCARD static FontSelectionCapabilities CapabilitiesOfFace(FT_Face face) noexcept
  {
    constexpr FT_UShort italicMask = 1 << 0;
    constexpr FT_UShort obliqueMask = 1 << 9;

    bool isItalic = face->style_flags & FT_STYLE_FLAG_ITALIC;
    auto weight = face->style_flags & FT_STYLE_FLAG_BOLD ? BoldWeightValue() : NormalWeightValue();
    auto width = NormalWidthValue();
    if (auto *os2 = static_cast<TT_OS2 *>(FT_Get_Sfnt_Table(face, FT_SFNT_OS2));
        os2 && os2->version != 0xFFFF)
    {
      if (os2->usWeightClass)
      {
        weight = FontSelectionValue {static_cast<float>(std::clamp<FT_UShort>(os2->usWeightClass, 1, 1'000))};
      }
      width = FontSelectionValue {WidthForWidthClass(os2->usWidthClass)};
      isItalic = isItalic || (os2->fsSelection & (italicMask | obliqueMask));
    }

    FontSelectionCapabilities capabilities;
    capabilities.Weight = FontSelectionRange {weight};
    capabilities.Width = FontSelectionRange {width};
    capabilities.Slope = FontSelectionRange {isItalic ? ItalicValue() : NormalItalicValue()};
    return capabilities;
  }

  KRYS_NODISCARD static utf8_string ToString(const char *name) noexcept
  {
    if (!name)
    {
      return {};
    }
    return utf8_string {reinterpret_cast<const char8 *>(name)};
  }

  FreeTypeFace::FreeTypeFace(Ref<FontFileData> &&data, FT_Face face) noexcept
      : _data(krys::move(data)), _face(face), _familyName(ToString(face->family_name)),
        _styleName(ToString(face->style_name)), _postScriptName(ToString(FT_Get_Postscript_Name(face))),
        _capabilities(CapabilitiesOfFace(face)), _isFixedPitch(FT_IS_FIXED_WIDTH(face))
  {
  }

  FreeTypeFace::~FreeTypeFace() noexcept
  {
    FT_Done_Face(_face);
  }

  RefPtr<FreeTypeFace> FreeTypeFace::Create(Ref<FontFileData> data, uint32 faceIndex) noexcept
  {
    auto *face = OpenFace(*data, static_cast<FT_Long>(faceIndex));
    if (!face)
    {
      return nullptr;
    }
    // Only scalable fonts can be laid out at any size.
    if (!FT_IS_SCALABLE(face) || !face->units_per_EM)
    {
      FT_Done_Face(face);
      return nullptr;
    }
    // Glyphs are looked up by Unicode code point.
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE))
    {
      FT_Done_Face(face);
      return nullptr;
    }
    return AdoptRef(*new FreeTypeFace(krys::move(data), face));
  }

  uint32 FreeTypeFace::FaceCount(const FontFileData &data) noexcept
  {
    // Face index -1 only reads the number of faces.
    auto *face = OpenFace(data, -1);
    if (!face)
    {
      return 0;
    }
    auto count = static_cast<uint32>(face->num_faces);
    FT_Done_Face(face);
    return count;
  }

  uint32 FreeTypeFace::UnitsPerEm() const noexcept
  {
    return _face->units_per_EM;
  }

  uint32 FreeTypeFace::GlyphIndex(char32 codePoint) const noexcept
  {
    return FT_Get_Char_Index(_face, static_cast<FT_ULong>(codePoint));
  }

  uint32 FreeTypeFace::GlyphIndex(char32 codePoint, char32 variation) const noexcept
  {
    return FT_Face_GetCharVariantIndex(_face, static_cast<FT_ULong>(codePoint),
                                       static_cast<FT_ULong>(variation));
  }

  Maybe<List<byte>> FreeTypeFace::OpenTypeTable(uint32 tag) const noexcept
  {
    FT_ULong length = 0;
    if (FT_Load_Sfnt_Table(_face, tag, 0, nullptr, &length) || !length)
    {
      return null;
    }
    List<byte> table(length);
    if (FT_Load_Sfnt_Table(_face, tag, 0, reinterpret_cast<FT_Byte *>(table.data()), &length))
    {
      return null;
    }
    return table;
  }
}
