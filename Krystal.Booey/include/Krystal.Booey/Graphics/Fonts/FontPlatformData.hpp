/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Michael Emmel mike.emmel@gmail.com
 * Copyright (C) 2007 Holger Hans Peter Freyther
 * Copyright (C) 2007 Pioneer Research Center USA, Inc.
 * Copyright (C) 2010, 2011 Brent Fulgham <bfulgham@webkit.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontFeatureValues.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontPaletteValues.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontTaggedSettings.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"
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
  class FontCreationContext;
  class FontDescription;
  struct FontSizeAdjust;

  /// @brief A face at a size: what a font is made from. Whether bold and oblique are synthesized, the
  /// orientation, the width variant (for `text-combine-upright`), the text rendering mode, and the features,
  /// feature values and palette of the rules that name it come with it.
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
    FontFeatureSettings _fontFaceFeatures;
    RefPtr<FontFeatureValues> _fontFeatureValues;
    uint32 _paletteIndex {0};
    List<FontPaletteValues::OverriddenColor> _paletteOverrides;

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

    /// @brief Resizes the font so that the metric @p fontSizeAdjust names is its aspect value times
    /// @p computedSize. Nothing changes without a value, or if the font lacks the metric.
    void UpdateSizeWithFontSizeAdjust(const FontSizeAdjust &fontSizeAdjust, float computedSize) noexcept;

    /// @brief The face, or null for an empty or deleted value.
    KRYS_NODISCARD text::FreeTypeFace *Face() const noexcept
    {
      return _face.get();
    }

    KRYS_NODISCARD bool HasVariations() const noexcept
    {
      return _face && !_face->Variations().empty();
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

    /// @brief Takes the features of the font's `@font-face` rule, the `@font-feature-values` of its family,
    /// and its palette: @p description's `font-palette`, with the `@font-palette-values` it names.
    /// @see https://drafts.csswg.org/css-fonts-4/#font-palette-prop
    void ApplyCreationContext(const FontDescription &description,
                              const FontCreationContext &context) noexcept;

    /// @brief The `font-feature-settings` descriptor of the font's `@font-face` rule.
    KRYS_NODISCARD const FontFeatureSettings &FontFaceFeatures() const noexcept
    {
      return _fontFaceFeatures;
    }

    /// @brief The `@font-feature-values` of the font's family, for `font-variant-alternates`.
    KRYS_NODISCARD const RefPtr<FontFeatureValues> &GetFontFeatureValues() const noexcept
    {
      return _fontFeatureValues;
    }

    /// @brief The palette (`CPAL`) colour glyphs are drawn with.
    KRYS_NODISCARD uint32 PaletteIndex() const noexcept
    {
      return _paletteIndex;
    }

    /// @brief Colours replacing entries of that palette, each an entry the palette has; later ones win.
    KRYS_NODISCARD const List<FontPaletteValues::OverriddenColor> &PaletteOverrides() const noexcept
    {
      return _paletteOverrides;
    }

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
