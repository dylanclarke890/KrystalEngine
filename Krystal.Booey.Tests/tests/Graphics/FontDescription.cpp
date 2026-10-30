#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Graphics/FontPalette.hpp"
#include "Krystal.Booey/Graphics/FontSizeAdjust.hpp"
#include "Krystal.Booey/Graphics/SystemFontDatabase.hpp"

namespace krys::boo::tests
{
  TEST_CASE("FontMetrics", "[Graphics][FontDescription]")
  {
    FontMetrics metrics;
    CHECK(metrics.UnitsPerEm() == FontMetrics::DefaultUnitsPerEm);
    CHECK(metrics.Ascent() == 0.f);
    CHECK_FALSE(metrics.XHeight());

    metrics.SetAscent(12.6f);
    metrics.SetDescent(3.4f);
    metrics.SetLineGap(1.5f);
    metrics.SetCapHeight(9.f);
    CHECK(metrics.IntAscent() == 13);
    CHECK(metrics.IntDescent() == 3);
    CHECK(metrics.IntLineGap() == 2);
    CHECK(metrics.Height() == 16.f);
    CHECK(metrics.IntHeight() == 16);
    CHECK(*metrics.CapHeight() == 9.f);

    // Ideographic and central baselines centre the em box.
    CHECK(metrics.Ascent(FontBaseline::Ideographic) == 8.f);
    CHECK(metrics.IntAscent(FontBaseline::Central) == 8);
    CHECK(metrics.IntDescent(FontBaseline::Central) == 8);

    metrics.SetAscent(-2.f);
    CHECK(metrics.IntAscent() == 0);

    FontMetrics other;
    other.SetAscent(13.f);
    other.SetDescent(3.f);
    other.SetLineGap(2.f);
    metrics.SetAscent(12.6f);
    CHECK(metrics.HasIdenticalAscentDescentAndLineGap(other));
    CHECK(ScaleEmToUnits(500.f, 1'000) == 0.5f);
    CHECK(ScaleEmToUnits(500.f, 0) == 500.f);
  }

  TEST_CASE("FontSizeAdjust", "[Graphics][FontDescription]")
  {
    FontSizeAdjust none;
    CHECK(none.IsNone());
    CHECK_FALSE(none.ShouldResolveFromFont());

    FontSizeAdjust fromFont {FontSizeAdjust::Metric::CapHeight, FontSizeAdjust::ValueType::FromFont,
                             std::nullopt};
    CHECK_FALSE(fromFont.IsNone());
    CHECK(fromFont.ShouldResolveFromFont());

    FontMetrics metrics;
    CHECK_FALSE(fromFont.Resolve(16.f, metrics));
    metrics.SetCapHeight(12.f);
    metrics.SetXHeight(8.f);
    CHECK(fromFont.Resolve(16.f, metrics) == 0.75f);
    CHECK(FontSizeAdjust {}.Resolve(16.f, metrics) == 0.5f);
    CHECK_FALSE(fromFont.Resolve(0.f, metrics));

    FontSizeAdjust number {FontSizeAdjust::Metric::ExHeight, FontSizeAdjust::ValueType::Number, 0.5f};
    CHECK_FALSE(number.IsNone());
    CHECK(number != none);
  }

  TEST_CASE("FontPalette", "[Graphics][FontDescription]")
  {
    FontPalette normal;
    FontPalette dark {FontPalette::Type::Dark, StringAtom {u8"ignored"}};
    CHECK(normal == FontPalette {FontPalette::Type::Normal, StringAtom {u8"other"}});
    CHECK(dark == FontPalette {FontPalette::Type::Dark, StringAtom::Null()});
    CHECK(FontPalette {FontPalette::Type::Custom, StringAtom {u8"--a"}}
          != FontPalette {FontPalette::Type::Custom, StringAtom {u8"--b"}});
    CHECK(DefaultHash<FontPalette>::Hash(dark)
          == DefaultHash<FontPalette>::Hash({FontPalette::Type::Dark, StringAtom::Null()}));
  }

  TEST_CASE("FontDescription", "[Graphics][FontDescription]")
  {
    FontDescription description;
    CHECK(description.Weight() == NormalWeightValue());
    CHECK(description.Width() == NormalWidthValue());
    CHECK_FALSE(description.FontStyleSlope());
    CHECK(description.Script() == USCRIPT_COMMON);
    CHECK(description.ComputedLocale() == StringAtom::Null());
    CHECK(description.GetFontPalette().PaletteType == FontPalette::Type::Normal);
    CHECK(description.VariantAlternates().IsNormal());
    CHECK(description.VariantSettings().IsAllNormal());
    CHECK(description.GetFontStyleAxis() == FontStyleAxis::slnt);
    CHECK(description.OpticalSizing() == FontOpticalSizing::Auto);
    CHECK(description.HasAutoFontSynthesisWeight());
    CHECK(description.ShouldAllowUserInstalledFonts() == AllowUserInstalledFonts::No);

    SECTION("sizes")
    {
      description.SetComputedSize(32.f, 2.f);
      CHECK(description.ComputedSize() == 32.f);
      CHECK(description.UnzoomedComputedSize() == 16.f);
      CHECK(description.ComputedSizeForRangeZoomOption(css::RangeZoomOptions::Unzoomed) == 32.f);
      description.SetEvaluationTimeZoomEnabled(true);
      CHECK(description.ComputedSizeForRangeZoomOption(css::RangeZoomOptions::Unzoomed) == 16.f);
      CHECK(description.ComputedSizeForRangeZoomOption(css::RangeZoomOptions::Default) == 32.f);

      CHECK(description.AdjustedSizeForFontFace(0.5f) == 16.f);
      description.SetFontSizeAdjust(
        {FontSizeAdjust::Metric::ExHeight, FontSizeAdjust::ValueType::Number, 0.5f});
      CHECK(description.AdjustedSizeForFontFace(0.5f) == 32.f);
    }

    SECTION("style")
    {
      description.SetIsItalic(true);
      CHECK(description.FontStyleSlope() == ItalicValue());
      description.SetIsItalic(false);
      CHECK_FALSE(description.FontStyleSlope());
      description.SetWeight(BoldWeightValue());
      CHECK(description.GetFontSelectionRequest().Weight == BoldWeightValue());
    }

    SECTION("variants")
    {
      description.SetVariantNumeric({FontVariantNumericFigure::OldStyleNumbers,
                                     FontVariantNumericSpacing::Normal,
                                     FontVariantNumericFraction::StackedFractions,
                                     FontVariantNumericOrdinal::Yes, FontVariantNumericSlashedZero::Yes});
      CHECK(description.VariantNumericFigure() == FontVariantNumericFigure::OldStyleNumbers);
      CHECK(description.VariantNumericFraction() == FontVariantNumericFraction::StackedFractions);
      CHECK(description.VariantNumeric().SlashedZero == FontVariantNumericSlashedZero::Yes);

      description.SetVariantLigatures({FontVariantLigatures::No, FontVariantLigatures::Yes,
                                       FontVariantLigatures::Normal, FontVariantLigatures::No});
      CHECK(description.VariantLigatures().Discretionary == FontVariantLigatures::Yes);
      CHECK(description.VariantContextualAlternates() == FontVariantLigatures::No);

      description.SetVariantEastAsian({FontVariantEastAsianVariant::Traditional,
                                       FontVariantEastAsianWidth::Full, FontVariantEastAsianRuby::Yes});
      CHECK(description.VariantEastAsianVariant() == FontVariantEastAsianVariant::Traditional);
      CHECK(description.VariantEastAsian().Ruby == FontVariantEastAsianRuby::Yes);

      description.SetVariantCaps(FontVariantCaps::AllSmall);
      description.SetVariantEmoji(FontVariantEmoji::Emoji);
      auto settings = description.VariantSettings();
      CHECK_FALSE(settings.IsAllNormal());
      CHECK(settings.Caps == FontVariantCaps::AllSmall);
      CHECK(settings.Emoji == FontVariantEmoji::Emoji);
      CHECK(settings.NumericOrdinal == FontVariantNumericOrdinal::Yes);
    }

    SECTION("locale")
    {
      description.SetSpecifiedLocale(StringAtom {u8"ja-JP"});
      CHECK(description.Script() == USCRIPT_KATAKANA_OR_HIRAGANA);
      CHECK(description.ComputedLocale() == StringAtom {u8"ja-JP"});
      CHECK(description.SpecifiedLocale() == StringAtom {u8"ja-JP"});

      // Han locales compute to a specialised Chinese locale.
      description.SetSpecifiedLocale(StringAtom {u8"zh"});
      CHECK(description.Script() == USCRIPT_HAN);
      CHECK(description.ComputedLocale() == StringAtom {u8"zh-hans"});
      CHECK(description.SpecifiedLocale() == StringAtom {u8"zh"});

      description.SetSpecifiedLocale(StringAtom {u8"zh-TW"});
      CHECK(description.Script() == USCRIPT_TRADITIONAL_HAN);
      CHECK(description.ComputedLocale() == StringAtom {u8"zh-TW"});
    }

    SECTION("equality")
    {
      FontDescription other;
      CHECK(description == other);
      other.SetTextRenderingMode(TextRenderingMode::OptimizeLegibility);
      CHECK(description != other);
      CHECK(other.GetTextRenderingMode() == TextRenderingMode::OptimizeLegibility);
    }

    CHECK(
      FontDescription::PlatformResolveGenericFamily(USCRIPT_LATIN, StringAtom::Null(), StringAtom {u8"serif"})
      == StringAtom::Null());
  }

  TEST_CASE("FontCascadeDescription", "[Graphics][FontDescription]")
  {
    FontCascadeDescription description;
    CHECK(description.FamilyCount() == 1);
    CHECK(description.FirstFamily() == StringAtom::Null());
    CHECK(description.KeywordSize() == 0);
    CHECK(description.KeywordSizeAsIdentifier() == css::ValueId::Invalid);
    CHECK(description.GetKerning() == Kerning::Auto);
    CHECK(description.UsedFontSmoothing() == FontSmoothingMode::Auto);

    SECTION("relative weights")
    {
      CHECK(FontCascadeDescription::LighterWeight(FontSelectionValue {50}) == FontSelectionValue {50});
      CHECK(FontCascadeDescription::LighterWeight(FontSelectionValue {500}) == FontSelectionValue {100});
      CHECK(FontCascadeDescription::LighterWeight(FontSelectionValue {700}) == FontSelectionValue {400});
      CHECK(FontCascadeDescription::LighterWeight(FontSelectionValue {900}) == FontSelectionValue {700});
      CHECK(FontCascadeDescription::BolderWeight(FontSelectionValue {300}) == FontSelectionValue {400});
      CHECK(FontCascadeDescription::BolderWeight(FontSelectionValue {400}) == FontSelectionValue {700});
      CHECK(FontCascadeDescription::BolderWeight(FontSelectionValue {700}) == FontSelectionValue {900});
      CHECK(FontCascadeDescription::BolderWeight(FontSelectionValue {950}) == FontSelectionValue {950});
      CHECK(description.BolderWeight() == BoldWeightValue());
    }

    SECTION("keyword sizes")
    {
      description.SetKeywordSizeFromIdentifier(css::ValueId::XxSmall);
      CHECK(description.KeywordSize() == 1);
      description.SetKeywordSizeFromIdentifier(css::ValueId::XxxLarge);
      CHECK(description.KeywordSize() == 8);
      CHECK(description.KeywordSizeAsIdentifier() == css::ValueId::XxxLarge);
      description.SetKeywordSizeFromIdentifier(css::ValueId::Invalid);
      CHECK(description.KeywordSize() == 0);
    }

    SECTION("families")
    {
      description.SetOneFamily(css::WebKitFontFamilyNames::MonospaceFamily());
      CHECK(description.UseFixedDefaultSize());
      description.SetFamilies(
        List<StringAtom> {StringAtom {u8"Arial"}, css::WebKitFontFamilyNames::SerifFamily()});
      CHECK_FALSE(description.UseFixedDefaultSize());
      CHECK(description.FamilyCount() == 2);
      CHECK(description.EffectiveFamilyCount() == 2);
      CHECK(description.EffectiveFamilyAt(1) == css::WebKitFontFamilyNames::SerifFamily());

      CHECK(FontCascadeDescription::FamilyNamesAreEqual(StringAtom {u8"Arial"}, StringAtom {u8"aRIAL"}));
      CHECK_FALSE(
        FontCascadeDescription::FamilyNamesAreEqual(StringAtom {u8"Arial"}, StringAtom {u8"Arial2"}));
      CHECK(FontCascadeDescription::FamilyNameHash(StringAtom {u8"Arial"})
            == FontCascadeDescription::FamilyNameHash(StringAtom {u8"ARIAL"}));
      CHECK(FontCascadeDescription::FoldedFamilyName(u8"Times New Roman") == u8"times new roman");
    }

    SECTION("equality")
    {
      FontCascadeDescription other;
      CHECK(description == other);
      other.SetSpecifiedSize(12.f);
      CHECK(description != other);
      other = description;
      other.SetIsSpecifiedFont(true);
      CHECK(description != other);
      other = description;
      other.SetFamilies(List<StringAtom> {StringAtom {u8"Arial"}});
      CHECK(description != other);
    }
  }

  TEST_CASE("FontCascade", "[Graphics][FontDescription]")
  {
    FontCascadeDescription description;
    description.SetComputedSize(20.f);
    description.SetShouldDisableLigaturesForSpacing(true);
    description.SetKerning(Kerning::NoShift);
    FontCascade cascade {krys::move(description)};

    CHECK(cascade.Size() == 20.f);
    CHECK_FALSE(cascade.GetFontDescription().ShouldDisableLigaturesForSpacing());
    CHECK_FALSE(cascade.EnableKerning());
    // `text-rendering: auto` is advanced text rendering.
    CHECK(cascade.RequiresShaping());
    CHECK(cascade.Generation() != 0);

    SECTION("primary font metrics")
    {
      // No family is given, so the primary font is the bundled Liberation Sans.
      const auto &metrics = cascade.MetricsOfPrimaryFont();
      CHECK(metrics.IntAscent() == 18);
      CHECK(metrics.IntLineSpacing() == 23);
      CHECK(*metrics.IdeogramWidth() == 20.f);
      CHECK(*metrics.XHeight() == Catch::Approx(10.56640625f));
      CHECK(*metrics.CapHeight() == Catch::Approx(13.759765625f));
      CHECK(cascade.ZeroWidth() == Catch::Approx(11.123046875f));
      CHECK(cascade.PrimaryFont()->PlatformData().FamilyName() == u8"Liberation Sans");
    }

    SECTION("spacing and equality")
    {
      // Cascades without fonts are never equal.
      FontCascade unresolved = cascade;
      CHECK_FALSE(unresolved == cascade);

      cascade.Update();
      FontCascade copy = cascade;
      CHECK(copy == cascade);
      copy.SetLetterSpacing(2.f);
      CHECK(copy.LetterSpacing() == 2.f);
      CHECK(copy != cascade);

      FontCascadeDescription bold = cascade.GetFontDescription();
      bold.SetWeight(BoldWeightValue());
      FontCascade withSpacing {krys::move(bold), copy};
      CHECK(withSpacing.LetterSpacing() == 2.f);
      CHECK(withSpacing.Weight() == BoldWeightValue());
      CHECK(withSpacing.Generation() > cascade.Generation());
    }

    SECTION("kerning")
    {
      cascade.MutableFontDescription().SetKerning(Kerning::Normal);
      cascade.UpdateEnableKerning();
      CHECK(cascade.EnableKerning());
      cascade.MutableFontDescription().SetKerning(Kerning::Auto);
      cascade.MutableFontDescription().SetTextRenderingMode(TextRenderingMode::OptimizeSpeed);
      cascade.UpdateEnableKerning();
      cascade.UpdateRequiresShaping();
      CHECK_FALSE(cascade.EnableKerning());
      CHECK_FALSE(cascade.RequiresShaping());
      cascade.MutableFontDescription().SetVariantCaps(FontVariantCaps::Small);
      cascade.UpdateRequiresShaping();
      CHECK(cascade.RequiresShaping());
      CHECK(cascade.IsSmallCaps());
    }
  }

  TEST_CASE("SystemFontDatabase", "[Graphics][FontDescription]")
  {
    auto &database = SystemFontDatabase::Singleton();
    CHECK(&database == &SystemFontDatabase::Singleton());
    CHECK(database.SystemFontShorthandFamily(SystemFontDatabase::FontShorthand::Caption)
          == css::WebKitFontFamilyNames::StandardFamily());
    CHECK(database.SystemFontShorthandSize(SystemFontDatabase::FontShorthand::StatusBar) == 16.f);
    CHECK(database.SystemFontShorthandWeight(SystemFontDatabase::FontShorthand::Menu) == NormalWeightValue());
  }
}
