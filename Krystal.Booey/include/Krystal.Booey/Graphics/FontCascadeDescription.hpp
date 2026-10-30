#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  /// @brief A family a font cascade asks for. Platforms with native font family specifications (Core Text)
  /// add theirs as a second alternative.
  using FontFamilySpecification = StringAtom;

  /// @brief A font description with the family list and the specified-size state of the cascade it
  /// describes.
  class FontCascadeDescription : public FontDescription
  {
  private:
    List<StringAtom> _families {StringAtom::Null()};

    // Specified CSS value. Independent of rendering issues such as integer rounding, minimum font sizes, and
    // zooming.
    float _specifiedSize {0};
    // Whether or not CSS specified an explicit size (logical sizes like "medium" don't count).
    uint32 _isAbsoluteSize : 1 {false};
    uint32 _kerning : 2 {ToUnderlying(Kerning::Auto)};
    // We cache whether or not a font is currently represented by a CSS keyword (e.g., medium). If so, then we
    // can accurately translate across different generic families to adjust for different preference settings
    // (e.g., 13px monospace vs. 16px everything else). Sizes are 1-8 (like the HTML size values for <font>).
    uint32 _keywordSize : 4 {0};
    uint32 _fontSmoothing : 2 {ToUnderlying(FontSmoothingMode::Auto)};
    // True if a web page specifies a non-generic font family as the first font family.
    uint32 _isSpecifiedFont : 1 {false};

  public:
    FontCascadeDescription() noexcept = default;

    KRYS_NODISCARD bool operator==(const FontCascadeDescription &) const noexcept = default;

    KRYS_NODISCARD uint32 FamilyCount() const noexcept
    {
      return static_cast<uint32>(_families.size());
    }
    KRYS_NODISCARD const StringAtom &FirstFamily() const noexcept
    {
      return FamilyAt(0);
    }
    KRYS_NODISCARD const StringAtom &FamilyAt(uint32 i) const noexcept
    {
      return _families[i];
    }
    KRYS_NODISCARD const List<StringAtom> &Families() const noexcept
    {
      return _families;
    }

    KRYS_NODISCARD static bool FamilyNamesAreEqual(const StringAtom &, const StringAtom &) noexcept;
    KRYS_NODISCARD static size_t FamilyNameHash(const StringAtom &) noexcept;
    KRYS_NODISCARD static utf8_string FoldedFamilyName(utf8_stringview) noexcept;

    KRYS_NODISCARD uint32 EffectiveFamilyCount() const noexcept;
    KRYS_NODISCARD FontFamilySpecification EffectiveFamilyAt(uint32) const noexcept;

    KRYS_NODISCARD float SpecifiedSize() const noexcept
    {
      return _specifiedSize;
    }
    KRYS_NODISCARD bool IsAbsoluteSize() const noexcept
    {
      return _isAbsoluteSize;
    }
    KRYS_NODISCARD FontSelectionValue LighterWeight() const noexcept
    {
      return LighterWeight(Weight());
    }
    KRYS_NODISCARD FontSelectionValue BolderWeight() const noexcept
    {
      return BolderWeight(Weight());
    }
    KRYS_NODISCARD static FontSelectionValue LighterWeight(FontSelectionValue) noexcept;
    KRYS_NODISCARD static FontSelectionValue BolderWeight(FontSelectionValue) noexcept;

    /// @brief Only use fixed default size when there is only one font family, and that family is "monospace".
    KRYS_NODISCARD bool UseFixedDefaultSize() const noexcept
    {
      return FamilyCount() == 1 && FirstFamily() == css::WebKitFontFamilyNames::MonospaceFamily();
    }

    KRYS_NODISCARD boo::Kerning GetKerning() const noexcept
    {
      return static_cast<boo::Kerning>(_kerning);
    }
    KRYS_NODISCARD uint32 KeywordSize() const noexcept
    {
      return _keywordSize;
    }
    KRYS_NODISCARD css::ValueId KeywordSizeAsIdentifier() const noexcept
    {
      auto identifier = _keywordSize
                          ? static_cast<css::ValueId>(ToUnderlying(css::ValueId::XxSmall) + _keywordSize - 1)
                          : css::ValueId::Invalid;
      krys_debug_assert(identifier == css::ValueId::Invalid
                        || (identifier >= css::ValueId::XxSmall && identifier <= css::ValueId::XxxLarge));
      return identifier;
    }
    KRYS_NODISCARD FontSmoothingMode FontSmoothing() const noexcept
    {
      return static_cast<FontSmoothingMode>(_fontSmoothing);
    }
    KRYS_NODISCARD FontSmoothingMode UsedFontSmoothing() const noexcept;
    KRYS_NODISCARD bool IsSpecifiedFont() const noexcept
    {
      return _isSpecifiedFont;
    }

    void SetOneFamily(const StringAtom &family) noexcept
    {
      krys_debug_assert(_families.size() == 1);
      _families[0] = family;
    }
    void SetFamilies(const List<StringAtom> &families)
    {
      _families = families;
    }
    void SetFamilies(List<StringAtom> &&families) noexcept
    {
      _families = krys::move(families);
    }
    void SetSpecifiedSize(float s) noexcept
    {
      _specifiedSize = ClampTo<float>(s);
    }
    void SetIsAbsoluteSize(bool s) noexcept
    {
      _isAbsoluteSize = s;
    }
    void SetKerning(boo::Kerning kerning) noexcept
    {
      _kerning = ToUnderlying(kerning);
    }
    void SetKeywordSize(uint32 size) noexcept
    {
      krys_debug_assert(size <= 8);
      _keywordSize = size;
      krys_debug_assert(_keywordSize == size); // Make sure it fits in the bitfield.
    }
    void SetKeywordSizeFromIdentifier(css::ValueId identifier) noexcept
    {
      krys_debug_assert(identifier == css::ValueId::Invalid
                        || (identifier >= css::ValueId::XxSmall && identifier <= css::ValueId::XxxLarge));
      static_assert(ToUnderlying(css::ValueId::XxxLarge) - ToUnderlying(css::ValueId::XxSmall) + 1 == 8,
                    "Maximum keyword size should be 8.");
      SetKeywordSize(identifier != css::ValueId::Invalid
                       ? ToUnderlying(identifier) - ToUnderlying(css::ValueId::XxSmall) + 1u
                       : 0u);
    }
    void SetFontSmoothing(FontSmoothingMode smoothing) noexcept
    {
      _fontSmoothing = ToUnderlying(smoothing);
    }
    void SetIsSpecifiedFont(bool isSpecifiedFont) noexcept
    {
      _isSpecifiedFont = isSpecifiedFont;
    }

    // FIXME: `resolveFontSizeAdjustFromFontIfNeeded(const Font &)` comes with the font backend.
  };
}
