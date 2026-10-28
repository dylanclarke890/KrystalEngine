#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"
#include "Krystal.Text/HarfBuzz.hpp"

namespace krys::boo
{
  /// @brief A face at a size: what a font is made from. Whether bold and oblique are synthesized, the
  /// orientation, the width variant (for `text-combine-upright`) and the text rendering mode come with it.
  class FontPlatformData
  {
  private:
    RefPtr<text::FreeTypeFace> _face;
    float _size {0};
    FontOrientation _orientation {FontOrientation::Horizontal};
    FontWidthVariant _widthVariant {FontWidthVariant::RegularWidth};
    TextRenderingMode _textRenderingMode {TextRenderingMode::Auto};
    bool _syntheticBold {false};
    bool _syntheticOblique {false};
    bool _isColorBitmapFont {false};
    bool _isHashTableDeletedValue {false};

  public:
    explicit FontPlatformData(HashTableDeletedValueTag) noexcept;
    FontPlatformData() noexcept;
    FontPlatformData(Ref<text::FreeTypeFace> face, float size, bool syntheticBold, bool syntheticOblique,
                     FontOrientation orientation = FontOrientation::Horizontal,
                     FontWidthVariant widthVariant = FontWidthVariant::RegularWidth,
                     TextRenderingMode textRenderingMode = TextRenderingMode::Auto) noexcept;

    FontPlatformData(const FontPlatformData &) noexcept;
    FontPlatformData &operator=(const FontPlatformData &) noexcept;
    ~FontPlatformData() noexcept;

    KRYS_NODISCARD static FontPlatformData CloneWithOrientation(const FontPlatformData &source,
                                                                FontOrientation orientation) noexcept;
    KRYS_NODISCARD static FontPlatformData CloneWithSyntheticOblique(const FontPlatformData &source,
                                                                     bool syntheticOblique) noexcept;
    KRYS_NODISCARD static FontPlatformData CloneWithSize(const FontPlatformData &source, float size) noexcept;

    /// @brief The face, or null for an empty or deleted value.
    KRYS_NODISCARD text::FreeTypeFace *Face() const noexcept
    {
      return _face.get();
    }

    KRYS_NODISCARD bool HasVariations() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool IsFixedPitch() const noexcept;

    KRYS_NODISCARD float Size() const noexcept
    {
      return _size;
    }

    KRYS_NODISCARD bool SyntheticBold() const noexcept
    {
      return _syntheticBold;
    }

    KRYS_NODISCARD bool SyntheticOblique() const noexcept
    {
      return _syntheticOblique;
    }

    KRYS_NODISCARD bool IsColorBitmapFont() const noexcept
    {
      return _isColorBitmapFont;
    }

    KRYS_NODISCARD FontOrientation Orientation() const noexcept
    {
      return _orientation;
    }

    KRYS_NODISCARD FontWidthVariant WidthVariant() const noexcept
    {
      return _widthVariant;
    }

    KRYS_NODISCARD TextRenderingMode GetTextRenderingMode() const noexcept
    {
      return _textRenderingMode;
    }

    // Keep in sync with callers of FontDescription::SetWidthVariant().
    KRYS_NODISCARD bool IsForTextCombine() const noexcept
    {
      return WidthVariant() != FontWidthVariant::RegularWidth;
    }

    KRYS_NODISCARD utf8_string FamilyName() const noexcept;

    KRYS_NODISCARD size_t Hash() const noexcept;

    KRYS_NODISCARD bool operator==(const FontPlatformData &other) const noexcept;

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return _isHashTableDeletedValue;
    }

    KRYS_NODISCARD bool IsEmoji() const noexcept
    {
      return false;
    }

    /// @brief A HarfBuzz font in the face's units for reading its MATH table, or null if it doesn't have one.
    KRYS_NODISCARD text::HbUniquePtr<hb_font_t> CreateOpenTypeMathHarfBuzzFont() const noexcept;

    /// @brief The contents of the OpenType table @p table, or null if the font doesn't have it.
    KRYS_NODISCARD Maybe<List<byte>> OpenTypeTable(uint32 table) const noexcept;

    KRYS_NODISCARD utf8_string Description() const noexcept;

  private:
    // updateSize to be implemented by each platform since it needs to re-instantiate the platform font
    // object.
    void UpdateSize(float size) noexcept;
  };
}
