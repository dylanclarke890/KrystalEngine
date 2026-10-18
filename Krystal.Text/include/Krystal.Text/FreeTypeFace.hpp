#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Text/HarfBuzz.hpp"

// FreeType's own declarations of these handles.
typedef struct FT_FaceRec_ *FT_Face;

namespace krys::text
{
  /// @brief The style a face says it has.
  struct FaceStyle
  {
    /// @brief From 1 to 1,000: the OS/2 weight class, or 700 or 400 from the face's bold flag.
    float Weight {400.f};
    /// @brief As a percentage of the normal width, from the OS/2 width class.
    float Width {100.f};
    /// @brief Whether the face is italic or oblique.
    bool Italic {false};

    constexpr bool operator==(const FaceStyle &) const noexcept = default;
  };

  /// @brief The contents of a font file (a font or a collection of fonts), which FreeType reads in place.
  class FontFileData final : public RefCounted<FontFileData>
  {
    KRYS_NON_COPY_MOVABLE(FontFileData);

  private:
    List<byte> _owned;
    Span<const byte> _bytes;

    FontFileData(List<byte> &&owned) noexcept;
    explicit FontFileData(Span<const byte> external) noexcept;

  public:
    /// @brief Font data that the object owns.
    KRYS_NODISCARD static Ref<FontFileData> Create(List<byte> &&bytes) noexcept;
    /// @brief Font data that lives as long as the program (such as a bundled font).
    KRYS_NODISCARD static Ref<FontFileData> CreateStatic(Span<const byte> bytes) noexcept;

    KRYS_NODISCARD Span<const byte> Bytes() const noexcept
    {
      return _bytes;
    }
  };

  /// @brief One face of a font file, opened with FreeType: its names, its style and the FreeType face that
  /// metrics and glyphs are read from. The face has no size set: its users measure in font units, or make
  /// FreeType sizes of their own, so none changes what another reads.
  class FreeTypeFace final : public RefCounted<FreeTypeFace>
  {
    KRYS_NON_COPY_MOVABLE(FreeTypeFace);

  private:
    Ref<FontFileData> _data;
    FT_Face _face {nullptr};
    utf8_string _familyName;
    utf8_string _styleName;
    utf8_string _postScriptName;
    FaceStyle _style;
    bool _isFixedPitch {false};
    mutable HbUniquePtr<hb_face_t> _harfBuzzFace;

    FreeTypeFace(Ref<FontFileData> &&data, FT_Face face) noexcept;

  public:
    ~FreeTypeFace() noexcept;

    /// @brief Opens face @p faceIndex of @p data, or null if FreeType can't read it as a scalable font.
    KRYS_NODISCARD static RefPtr<FreeTypeFace> Create(Ref<FontFileData> data, uint32 faceIndex) noexcept;

    /// @brief The number of faces in @p data (more than one for a font collection), or 0 if FreeType can't
    /// read it.
    KRYS_NODISCARD static uint32 FaceCount(const FontFileData &data) noexcept;

    KRYS_NODISCARD FT_Face Face() const noexcept
    {
      return _face;
    }

    /// @brief The family name, as FreeType reports it.
    KRYS_NODISCARD const utf8_string &FamilyName() const noexcept
    {
      return _familyName;
    }

    KRYS_NODISCARD const utf8_string &StyleName() const noexcept
    {
      return _styleName;
    }

    KRYS_NODISCARD const utf8_string &PostScriptName() const noexcept
    {
      return _postScriptName;
    }

    KRYS_NODISCARD const FaceStyle &Style() const noexcept
    {
      return _style;
    }

    KRYS_NODISCARD bool IsFixedPitch() const noexcept
    {
      return _isFixedPitch;
    }

    /// @brief The HarfBuzz face of this face, read from the same font data.
    KRYS_NODISCARD hb_face_t *HarfBuzzFace() const noexcept;

    KRYS_NODISCARD uint32 UnitsPerEm() const noexcept;

    /// @brief The glyph of @p codePoint, or 0 if the face has none.
    KRYS_NODISCARD uint32 GlyphIndex(char32 codePoint) const noexcept;

    /// @brief The glyph of @p codePoint followed by the variation selector @p variation, or 0.
    KRYS_NODISCARD uint32 GlyphIndex(char32 codePoint, char32 variation) const noexcept;

    /// @brief The contents of the OpenType table @p tag, or null if the face doesn't have it.
    KRYS_NODISCARD Maybe<List<byte>> OpenTypeTable(uint32 tag) const noexcept;
  };
}
