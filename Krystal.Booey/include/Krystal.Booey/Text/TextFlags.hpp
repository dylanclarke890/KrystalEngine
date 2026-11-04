#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FontFeatureValues;

  enum class TextRenderingMode : uint8
  {
    Auto,
    OptimizeSpeed,
    OptimizeLegibility,
    GeometricPrecision
  };

  enum class FontSmoothingMode : uint8
  {
    Auto,
    None,
    Antialiased,
    SubpixelAntialiased
  };

  enum class FontOrientation : bool
  {
    Horizontal,
    Vertical
  };

  enum class NonCJKGlyphOrientation : bool
  {
    Mixed,
    Upright
  };

  struct ExpansionBehavior
  {
    enum class Behavior : uint8
    {
      Forbid,
      Allow,
      Force
    };

    constexpr static uint32 BitsOfKind = 2;
    Behavior Left : BitsOfKind;
    Behavior Right : BitsOfKind;

    constexpr ExpansionBehavior() noexcept : Left(Behavior::Forbid), Right(Behavior::Allow)
    {
    }

    constexpr ExpansionBehavior(Behavior left, Behavior right) noexcept : Left(left), Right(right)
    {
    }

    KRYS_NODISCARD constexpr bool operator==(const ExpansionBehavior &) const noexcept = default;

    KRYS_NODISCARD constexpr static ExpansionBehavior DefaultBehavior() noexcept
    {
      return {};
    }

    KRYS_NODISCARD constexpr static ExpansionBehavior AllowRightOnly() noexcept
    {
      return {Behavior::Forbid, Behavior::Allow};
    }

    KRYS_NODISCARD constexpr static ExpansionBehavior AllowLeftOnly() noexcept
    {
      return {Behavior::Allow, Behavior::Forbid};
    }

    KRYS_NODISCARD constexpr static ExpansionBehavior ForceLeftOnly() noexcept
    {
      return {Behavior::Force, Behavior::Forbid};
    }

    KRYS_NODISCARD constexpr static ExpansionBehavior ForbidAll() noexcept
    {
      return {Behavior::Forbid, Behavior::Forbid};
    }
  };

  enum class FontSynthesisLonghandValue : bool
  {
    None,
    Auto
  };

  enum class FontVariantLigatures : uint8
  {
    Normal,
    Yes,
    No
  };

  enum class FontVariantPosition : uint8
  {
    Normal,
    Subscript,
    Superscript
  };

  enum class FontVariantCaps : uint8
  {
    Normal,
    Small,
    AllSmall,
    Petite,
    AllPetite,
    Unicase,
    Titling
  };

  enum class FontVariantNumericFigure : uint8
  {
    Normal,
    LiningNumbers,
    OldStyleNumbers
  };

  enum class FontVariantNumericSpacing : uint8
  {
    Normal,
    ProportionalNumbers,
    TabularNumbers
  };

  enum class FontVariantNumericFraction : uint8
  {
    Normal,
    DiagonalFractions,
    StackedFractions
  };

  enum class FontVariantNumericOrdinal : bool
  {
    Normal,
    Yes
  };

  enum class FontVariantNumericSlashedZero : bool
  {
    Normal,
    Yes
  };

  struct FontVariantAlternatesValues
  {
    friend struct krys::MarkableTraits<FontVariantAlternatesValues>;

    Maybe<css::CSSOMString> Stylistic;
    List<css::CSSOMString> Styleset;
    List<css::CSSOMString> CharacterVariant;
    Maybe<css::CSSOMString> Swash;
    Maybe<css::CSSOMString> Ornaments;
    Maybe<css::CSSOMString> Annotation;
    bool HistoricalForms {false};

    KRYS_NODISCARD bool operator==(const FontVariantAlternatesValues &) const noexcept = default;

  private:
    bool _isEmpty {false};
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::FontVariantAlternatesValues>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::FontVariantAlternatesValues &value) noexcept
    {
      return value._isEmpty;
    }

    KRYS_NODISCARD static boo::FontVariantAlternatesValues EmptyValue() noexcept
    {
      boo::FontVariantAlternatesValues emptyValue;
      emptyValue._isEmpty = true;
      return emptyValue;
    }
  };
}

namespace krys::boo
{
  class FontVariantAlternates
  {
    using Values = FontVariantAlternatesValues;

  private:
    Markable<Values> _values;

    FontVariantAlternates() noexcept = default;

  public:
    KRYS_NODISCARD bool operator==(const FontVariantAlternates &) const noexcept = default;

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return !_values;
    }

    KRYS_NODISCARD const Values &GetValues() const noexcept
    {
      krys_debug_assert(!IsNormal());
      return *_values;
    }

    KRYS_NODISCARD Values &ValuesRef() noexcept
    {
      if (IsNormal())
      {
        SetValues();
      }

      return *_values;
    }

    void SetValues() noexcept
    {
      _values = Values {};
    }

    KRYS_NODISCARD static FontVariantAlternates Normal() noexcept
    {
      return {};
    }
  };

  enum class FontVariantEastAsianVariant : uint8
  {
    Normal,
    Jis78,
    Jis83,
    Jis90,
    Jis04,
    Simplified,
    Traditional
  };

  enum class FontVariantEastAsianWidth : uint8
  {
    Normal,
    Full,
    Proportional
  };

  enum class FontVariantEastAsianRuby : uint8
  {
    Normal,
    Yes
  };

  enum class FontVariantEmoji : uint8
  {
    Normal,
    Text,
    Emoji,
    Unicode,
  };

  struct FontVariantSettings
  {
    FontVariantLigatures CommonLigatures {FontVariantLigatures::Normal};
    FontVariantLigatures DiscretionaryLigatures {FontVariantLigatures::Normal};
    FontVariantLigatures HistoricalLigatures {FontVariantLigatures::Normal};
    FontVariantLigatures ContextualAlternates {FontVariantLigatures::Normal};
    FontVariantPosition Position {FontVariantPosition::Normal};
    FontVariantCaps Caps {FontVariantCaps::Normal};
    FontVariantNumericFigure NumericFigure {FontVariantNumericFigure::Normal};
    FontVariantNumericSpacing NumericSpacing {FontVariantNumericSpacing::Normal};
    FontVariantNumericFraction NumericFraction {FontVariantNumericFraction::Normal};
    FontVariantNumericOrdinal NumericOrdinal {FontVariantNumericOrdinal::Normal};
    FontVariantNumericSlashedZero NumericSlashedZero {FontVariantNumericSlashedZero::Normal};
    FontVariantAlternates Alternates {FontVariantAlternates::Normal()};
    FontVariantEastAsianVariant EastAsianVariant {FontVariantEastAsianVariant::Normal};
    FontVariantEastAsianWidth EastAsianWidth {FontVariantEastAsianWidth::Normal};
    FontVariantEastAsianRuby EastAsianRuby {FontVariantEastAsianRuby::Normal};
    FontVariantEmoji Emoji {FontVariantEmoji::Normal};

    KRYS_NODISCARD bool IsAllNormal() const noexcept
    {
      return CommonLigatures == FontVariantLigatures::Normal
             && DiscretionaryLigatures == FontVariantLigatures::Normal
             && HistoricalLigatures == FontVariantLigatures::Normal
             && ContextualAlternates == FontVariantLigatures::Normal
             && Position == FontVariantPosition::Normal && Caps == FontVariantCaps::Normal
             && NumericFigure == FontVariantNumericFigure::Normal
             && NumericSpacing == FontVariantNumericSpacing::Normal
             && NumericFraction == FontVariantNumericFraction::Normal
             && NumericOrdinal == FontVariantNumericOrdinal::Normal
             && NumericSlashedZero == FontVariantNumericSlashedZero::Normal && Alternates.IsNormal()
             && EastAsianVariant == FontVariantEastAsianVariant::Normal
             && EastAsianWidth == FontVariantEastAsianWidth::Normal
             && EastAsianRuby == FontVariantEastAsianRuby::Normal && Emoji == FontVariantEmoji::Normal;
    }

    KRYS_NODISCARD bool operator==(const FontVariantSettings &) const noexcept = default;
  };

  struct FontVariantLigaturesValues
  {
    FontVariantLigatures Common {FontVariantLigatures::Normal};
    FontVariantLigatures Discretionary {FontVariantLigatures::Normal};
    FontVariantLigatures Historical {FontVariantLigatures::Normal};
    FontVariantLigatures Contextual {FontVariantLigatures::Normal};

    KRYS_NODISCARD constexpr bool operator==(const FontVariantLigaturesValues &) const noexcept = default;
  };

  struct FontVariantNumericValues
  {
    FontVariantNumericFigure Figure {FontVariantNumericFigure::Normal};
    FontVariantNumericSpacing Spacing {FontVariantNumericSpacing::Normal};
    FontVariantNumericFraction Fraction {FontVariantNumericFraction::Normal};
    FontVariantNumericOrdinal Ordinal {FontVariantNumericOrdinal::Normal};
    FontVariantNumericSlashedZero SlashedZero {FontVariantNumericSlashedZero::Normal};

    KRYS_NODISCARD constexpr bool operator==(const FontVariantNumericValues &) const noexcept = default;
  };

  struct FontVariantEastAsianValues
  {
    FontVariantEastAsianVariant Variant {FontVariantEastAsianVariant::Normal};
    FontVariantEastAsianWidth Width {FontVariantEastAsianWidth::Normal};
    FontVariantEastAsianRuby Ruby {FontVariantEastAsianRuby::Normal};

    KRYS_NODISCARD constexpr bool operator==(const FontVariantEastAsianValues &) const noexcept = default;
  };

  enum class FontWidthVariant : uint8
  {
    RegularWidth,
    HalfWidth,
    ThirdWidth,
    QuarterWidth,
    LastFontWidthVariant = QuarterWidth
  };

  constexpr uint32 FontWidthVariantWidth = 2;

  static_assert(!(static_cast<uint32>(FontWidthVariant::LastFontWidthVariant) >> FontWidthVariantWidth),
                "FontWidthVariantWidth is correct");

  enum class Kerning : uint8
  {
    Auto,
    Normal,
    NoShift
  };

  enum class FontOpticalSizing : bool
  {
    None,
    Auto
  };

  // https://www.microsoft.com/typography/otspec/fvar.htm#VAT
  enum class FontStyleAxis : uint8
  {
    slnt,
    ital
  };

  enum class AllowUserInstalledFonts : bool
  {
    No,
    Yes
  };

  using FeaturesMap = HashMap<FontTag, int, FourCharacterTagHash>;

  KRYS_NODISCARD FeaturesMap ComputeFeatureSettingsFromVariants(const FontVariantSettings &,
                                                                RefPtr<FontFeatureValues>) noexcept;

  enum class ResolvedEmojiPolicy : uint8
  {
    NoPreference,
    RequireText,
    RequireEmoji,
  };

  enum class ColorGlyphType : uint8
  {
    Outline,
    Color,
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::FontVariantAlternatesValues>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontVariantAlternatesValues &key) noexcept;
  };

  template <>
  struct DefaultHash<boo::FontVariantAlternates>
  {
    KRYS_NODISCARD static size_t Hash(const boo::FontVariantAlternates &key) noexcept;
  };
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, FontSmoothingMode mode) noexcept;
}
