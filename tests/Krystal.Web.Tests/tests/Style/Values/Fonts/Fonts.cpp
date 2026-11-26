#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Web/Css/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Web/Css/Values/ValuePool.hpp"
#include "Krystal.Web/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Web/Style/Builder/BuilderState.hpp"
#include "Krystal.Web/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontFamily.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontFeatureSettings.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontPalette.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontStyle.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantAlternates.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantEastAsian.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantLigatures.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariantNumeric.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontVariationSettings.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontWeight.hpp"
#include "Krystal.Web/Style/Values/Fonts/FontWidth.hpp"
#include "Krystal.Web/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Web/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Web/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::web::style::tests
{
  using css::PropertyId;

  namespace
  {
    // `font: caption` sets every longhand to the system font keyword.
    template <typename T>
    KRYS_NODISCARD T ConvertSystemFont(BuilderState &state) noexcept
    {
      auto caption = css::PrimitiveValue::Create(css::ValueId::Caption);
      return ConvertCSSValueToStyle<T>(state, *caption);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontFamilies", "[Style][Values][Fonts]")
  {
    auto families = Convert<FontFamilies>(PropertyId::FontFamily, u8"\"Foo Bar\", serif, Arial");
    REQUIRE(families.size() == 3);
    CHECK(families.IsSpecifiedFont());
    CHECK(families.First().Value == u8"Foo Bar");
    CHECK(families.ToPlatform()[1] == css::generic_font_family_names::SerifFamily());
    CHECK(families.Last().Value == u8"Arial");
    CHECK(Serialised(families) == "\"Foo Bar\", serif, Arial");
    CHECK(Created(families) == "\"Foo Bar\", serif, Arial");

    auto generic = Convert<FontFamilies>(PropertyId::FontFamily, u8"monospace, Courier");
    CHECK(generic.FirstFontKind() == FontFamilyKind::Generic);
    CHECK(generic.First().Value == css::generic_font_family_names::MonospaceFamily());
    CHECK(Serialised(generic) == "monospace, Courier");

    auto system = ConvertSystemFont<FontFamilies>(*State);
    CHECK(system.FirstFontKind() == FontFamilyKind::Generic);
    CHECK(system.First().Value == css::generic_font_family_names::StandardFamily());
    CHECK_FALSE(IsInvalid());

    CHECK(families == Convert<FontFamilies>(PropertyId::FontFamily, u8"\"Foo Bar\", serif, Arial"));
    CHECK_FALSE(families == generic);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontFeatureSettings", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontFeatureSettings>(PropertyId::FontFeatureSettings, u8"normal").Platform().empty());

    auto settings = Convert<FontFeatureSettings>(PropertyId::FontFeatureSettings, u8"\"liga\" 0, \"kern\"");
    REQUIRE(settings.Platform().size() == 2);
    // Settings are kept sorted by tag.
    CHECK(settings.Platform()[0].Tag() == FontFeatureTag("kern"));
    CHECK(settings.Platform()[0].Value() == 1);
    CHECK(settings.Platform()[1].Value() == 0);
    CHECK(Serialised(settings) == "\"kern\", \"liga\" 0");
    CHECK(Created(settings) == "\"kern\", \"liga\" 0");
    CHECK(Serialised(FontFeatureSettings {css::keywords::Normal {}}) == "normal");

    CHECK(ConvertSystemFont<FontFeatureSettings>(*State).Platform().empty());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontVariationSettings", "[Style][Values][Fonts]")
  {
    auto settings =
      Convert<FontVariationSettings>(PropertyId::FontVariationSettings, u8"\"wght\" 400, \"wdth\" 75.5");
    REQUIRE(settings.Platform().size() == 2);
    CHECK(settings.Platform()[1].Value() == 400.f);
    CHECK(Serialised(settings) == "\"wdth\" 75.5, \"wght\" 400");
    CHECK(Created(settings) == "\"wdth\" 75.5, \"wght\" 400");
    CHECK(Created(FontVariationSettings {css::keywords::Normal {}}) == "normal");

    auto from = Convert<FontVariationSettings>(PropertyId::FontVariationSettings, u8"\"wght\" 400");
    auto to = Convert<FontVariationSettings>(PropertyId::FontVariationSettings, u8"\"wght\" 600");
    CHECK(Blending<FontVariationSettings> {}.CanBlend(from, to));
    CHECK_FALSE(Blending<FontVariationSettings> {}.CanBlend(from, settings));
    auto blended = Blending<FontVariationSettings> {}.Blend(from, to, BlendingContext {0.5});
    CHECK(blended.Platform()[0].Value() == 500.f);
    CHECK(Blending<FontVariationSettings> {}.Blend(from, to, BlendingContext {1.0, true}) == to);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontPalette", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontPalette>(PropertyId::FontPalette, u8"dark").Platform().PaletteType
          == web::FontPalette::Type::Dark);
    CHECK(Serialised(Convert<FontPalette>(PropertyId::FontPalette, u8"light")) == "light");

    auto custom = Convert<FontPalette>(PropertyId::FontPalette, u8"--custom");
    CHECK(custom.Platform().PaletteType == web::FontPalette::Type::Custom);
    CHECK(custom.Platform().Identifier == u8"--custom");
    CHECK(Serialised(custom) == "--custom");
    CHECK(Created(custom) == "--custom");

    CHECK(ConvertSystemFont<FontPalette>(*State).Platform().PaletteType == web::FontPalette::Type::Normal);
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontSizeAdjust", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontSizeAdjust>(PropertyId::FontSizeAdjust, u8"none").IsNone());

    auto number = Convert<FontSizeAdjust>(PropertyId::FontSizeAdjust, u8"0.5");
    CHECK(number.GetMetric() == FontSizeAdjust::Metric::ExHeight);
    CHECK(number.MetricValue() == 0.5f);
    CHECK(Serialised(number) == "0.5");
    CHECK(Created(number) == "0.5");

    auto capHeight = Convert<FontSizeAdjust>(PropertyId::FontSizeAdjust, u8"cap-height 0.75");
    CHECK(capHeight.GetMetric() == FontSizeAdjust::Metric::CapHeight);
    CHECK(Serialised(capHeight) == "cap-height 0.75");
    CHECK(Created(capHeight) == "cap-height 0.75");

    // `from-font` resolves against the primary font: the bundled Liberation Sans, whose x-height is
    // 1082/2048em and whose water ideogram is missing (1em).
    auto fromFont = Convert<FontSizeAdjust>(PropertyId::FontSizeAdjust, u8"ic-width from-font");
    CHECK(fromFont.IsFromFont());
    CHECK(fromFont.GetMetric() == FontSizeAdjust::Metric::IcWidth);
    CHECK(fromFont.ResolvedMetricValue(Style) == 1.f);
    CHECK(Serialised(fromFont) == "ic-width 1");
    CHECK(*Convert<FontSizeAdjust>(PropertyId::FontSizeAdjust, u8"from-font").ResolvedMetricValue(Style)
          == Catch::Approx(1082.f / 2048.f));
    CHECK(Serialised(Convert<FontSizeAdjust>(PropertyId::FontSizeAdjust, u8"from-font")) == "0.52832");

    CHECK(Blending<FontSizeAdjust> {}.CanBlend(
      number, FontSizeAdjust {FontSizeAdjust::Metric::ExHeight, FontSizeAdjust::Number {1}}));
    CHECK_FALSE(Blending<FontSizeAdjust> {}.CanBlend(number, capHeight));
    auto blended = Blending<FontSizeAdjust> {}.Blend(
      number, FontSizeAdjust {FontSizeAdjust::Metric::ExHeight, FontSizeAdjust::Number {1}},
      BlendingContext {0.5});
    CHECK(blended.MetricValue() == 0.75f);

    CHECK(ConvertSystemFont<FontSizeAdjust>(*State).IsNone());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontStyle", "[Style][Values][Fonts]")
  {
    auto normal = Convert<FontStyle>(PropertyId::FontStyle, u8"normal");
    CHECK(normal.IsNormal());
    CHECK(Serialised(normal) == "normal");

    auto italic = Convert<FontStyle>(PropertyId::FontStyle, u8"italic");
    CHECK(italic.IsItalic());
    CHECK(italic.PlatformAxis() == FontStyleAxis::ital);
    CHECK(Serialised(italic) == "italic");
    CHECK(Created(italic) == "italic");

    auto oblique = Convert<FontStyle>(PropertyId::FontStyle, u8"oblique");
    CHECK(oblique.IsOblique());
    CHECK(oblique.PlatformSlope() == ItalicValue());
    CHECK(Serialised(oblique) == "oblique");

    auto angle = Convert<FontStyle>(PropertyId::FontStyle, u8"oblique 20deg");
    CHECK(angle.IsOblique());
    CHECK(angle.IsConsideredItalic());
    CHECK(angle.GetAngle().value().Value == 20);
    CHECK(Serialised(angle) == "oblique 20deg");
    CHECK(Created(angle) == "oblique 20deg");
    CHECK_FALSE(Convert<FontStyle>(PropertyId::FontStyle, u8"oblique 10deg").IsConsideredItalic());

    CHECK(Blending<FontStyle> {}.CanBlend(oblique, angle));
    CHECK_FALSE(Blending<FontStyle> {}.CanBlend(italic, angle));
    auto blended = Blending<FontStyle> {}.Blend(Convert<FontStyle>(PropertyId::FontStyle, u8"oblique 10deg"),
                                                angle, BlendingContext {0.5});
    CHECK(blended.GetAngle().value().Value == 15);
    CHECK(Blending<FontStyle> {}.Blend(normal, normal, BlendingContext {0.5}).IsNormal());
    CHECK(Blending<FontStyle> {}.Blend(normal, angle, BlendingContext {0.25, true}) == normal);

    CHECK(ConvertSystemFont<FontStyle>(*State).IsNormal());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontWeight", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontWeight>(PropertyId::FontWeight, u8"normal").IsNormal());
    auto bold = Convert<FontWeight>(PropertyId::FontWeight, u8"bold");
    CHECK(bold.IsBold());
    CHECK(bold.IsConsideredBold());
    CHECK(Serialised(bold) == "700");
    CHECK(Created(bold) == "700");
    CHECK(Serialised(Convert<FontWeight>(PropertyId::FontWeight, u8"550")) == "550");

    // `bolder` and `lighter` are relative to the parent's weight (`normal`, then `bold`).
    CHECK(Convert<FontWeight>(PropertyId::FontWeight, u8"bolder").Platform() == FontSelectionValue {700});
    CHECK(Convert<FontWeight>(PropertyId::FontWeight, u8"lighter").Platform() == FontSelectionValue {100});
    auto description = ParentStyle.GetFontDescription();
    description.SetWeight(BoldWeightValue());
    ParentStyle.SetFontDescription(krys::move(description));
    CHECK(Convert<FontWeight>(PropertyId::FontWeight, u8"bolder").Platform() == FontSelectionValue {900});
    CHECK(Convert<FontWeight>(PropertyId::FontWeight, u8"lighter").Platform() == FontSelectionValue {400});

    CHECK(Blending<FontWeight> {}.Blend(FontWeight {css::keywords::Normal {}}, bold, BlendingContext {0.5})
          == FontWeight {FontSelectionValue {550}});

    CHECK(ConvertSystemFont<FontWeight>(*State).IsNormal());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontWidth", "[Style][Values][Fonts]")
  {
    auto condensed = Convert<FontWidth>(PropertyId::FontWidth, u8"condensed");
    CHECK(condensed.Platform() == CondensedWidthValue());
    CHECK(Serialised(condensed) == "75%");
    CHECK(Created(condensed) == "75%");
    CHECK(Convert<FontWidth>(PropertyId::FontWidth, u8"semi-expanded").Platform()
          == SemiExpandedWidthValue());
    CHECK(Serialised(Convert<FontWidth>(PropertyId::FontWidth, u8"150%")) == "150%");

    CHECK(Blending<FontWidth> {}.Blend(condensed, FontWidth {css::keywords::Normal {}}, BlendingContext {0.5})
          == FontWidth {FontSelectionValue {87.5f}});

    CHECK(ConvertSystemFont<FontWidth>(*State).Platform() == NormalWidthValue());
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontVariantAlternates", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontVariantAlternates>(PropertyId::FontVariantAlternates, u8"normal").IsNormal());

    auto historical = Convert<FontVariantAlternates>(PropertyId::FontVariantAlternates, u8"historical-forms");
    CHECK(historical.GetPlatform().GetValues().HistoricalForms);
    CHECK(Serialised(historical) == "historical-forms");
    CHECK(Created(historical) == "historical-forms");

    auto functions = Convert<FontVariantAlternates>(
      PropertyId::FontVariantAlternates, u8"swash(flowing) styleset(a, b) historical-forms stylistic(s)");
    const auto &values = functions.GetPlatform().GetValues();
    CHECK(values.Swash == CSSOMString {u8"flowing"});
    CHECK(values.Styleset == List<CSSOMString> {u8"a", u8"b"});
    CHECK(values.Stylistic == CSSOMString {u8"s"});
    CHECK_FALSE(values.Ornaments);
    CHECK(Serialised(functions) == "stylistic(s) historical-forms styleset(a, b) swash(flowing)");
    CHECK(Created(functions) == "stylistic(s) historical-forms styleset(a, b) swash(flowing)");
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontVariantEastAsian", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontVariantEastAsian>(PropertyId::FontVariantEastAsian, u8"normal").IsNormal());
    auto value = Convert<FontVariantEastAsian>(PropertyId::FontVariantEastAsian, u8"ruby full-width jis78");
    CHECK(value.GetPlatform().Variant == FontVariantEastAsianVariant::Jis78);
    CHECK(value.GetPlatform().Width == FontVariantEastAsianWidth::Full);
    CHECK(value.GetPlatform().Ruby == FontVariantEastAsianRuby::Yes);
    CHECK(Serialised(value) == "jis78 full-width ruby");
    CHECK(Created(value) == "jis78 full-width ruby");
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontVariantLigatures", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontVariantLigatures>(PropertyId::FontVariantLigatures, u8"normal").IsNormal());
    auto none = Convert<FontVariantLigatures>(PropertyId::FontVariantLigatures, u8"none");
    CHECK(none.IsNone());
    CHECK(Serialised(none) == "none");

    auto value = Convert<FontVariantLigatures>(PropertyId::FontVariantLigatures,
                                               u8"no-contextual discretionary-ligatures no-common-ligatures");
    CHECK(value.GetPlatform().Common == web::FontVariantLigatures::No);
    CHECK(value.GetPlatform().Discretionary == web::FontVariantLigatures::Yes);
    CHECK(value.GetPlatform().Historical == web::FontVariantLigatures::Normal);
    CHECK(Serialised(value) == "no-common-ligatures discretionary-ligatures no-contextual");
    CHECK(Created(value) == "no-common-ligatures discretionary-ligatures no-contextual");
  }

  TEST_CASE_METHOD(BuilderFixture, "Style FontVariantNumeric", "[Style][Values][Fonts]")
  {
    CHECK(Convert<FontVariantNumeric>(PropertyId::FontVariantNumeric, u8"normal").IsNormal());
    auto value = Convert<FontVariantNumeric>(PropertyId::FontVariantNumeric,
                                             u8"slashed-zero ordinal tabular-nums oldstyle-nums");
    CHECK(value.GetPlatform().Figure == FontVariantNumericFigure::OldStyleNumbers);
    CHECK(value.GetPlatform().Spacing == FontVariantNumericSpacing::TabularNumbers);
    CHECK(value.GetPlatform().Fraction == FontVariantNumericFraction::Normal);
    CHECK(Serialised(value) == "oldstyle-nums tabular-nums ordinal slashed-zero");
    CHECK(Created(value) == "oldstyle-nums tabular-nums ordinal slashed-zero");
  }
}
