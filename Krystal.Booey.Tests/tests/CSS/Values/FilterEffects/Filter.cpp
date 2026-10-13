#include "Krystal.Booey/CSS/Values/FilterEffects/Filter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using L = Length<AllUnzoomed>;
    using NL = Length<NonNegativeUnzoomed>;

    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    FilterReference Reference(utf8_stringview url)
    {
      ParserContext context;
      context.BaseURL = url::URL {u8"https://example.com/"};
      return FilterReference {*CompleteURL(CSSOMString {url}, context)};
    }

    DropShadowFunction Shadow()
    {
      return DropShadowFunction {.Parameters = {.Color = Color {KeywordColor {ValueId::Red}},
                                                .Location = {L {1_css_px}, L {2_css_px}},
                                                .StdDeviation = NL {3_css_px}}};
    }

    Filter Filters(FilterValueList::Container &&values)
    {
      return Filter {FilterValueList {krys::move(values)}};
    }
  }

  TEST_CASE("Filter(Serialise functions)", "[CSS][FilterEffects][Filter]")
  {
    // Every parameter is optional.
    CHECK(SerialiseForTest(BlurFunction {}) == u8"blur()");
    CHECK(SerialiseForTest(BrightnessFunction {}) == u8"brightness()");
    CHECK(SerialiseForTest(ContrastFunction {}) == u8"contrast()");
    CHECK(SerialiseForTest(GrayscaleFunction {}) == u8"grayscale()");
    CHECK(SerialiseForTest(HueRotateFunction {}) == u8"hue-rotate()");
    CHECK(SerialiseForTest(InvertFunction {}) == u8"invert()");
    CHECK(SerialiseForTest(OpacityFunction {}) == u8"opacity()");
    CHECK(SerialiseForTest(SaturateFunction {}) == u8"saturate()");
    CHECK(SerialiseForTest(SepiaFunction {}) == u8"sepia()");

    CHECK(SerialiseForTest(BlurFunction {.Parameters = {Blur::Parameter {2_css_px}}}) == u8"blur(2px)");
    CHECK(
      SerialiseForTest(BrightnessFunction {.Parameters = {Brightness::Parameter {Number<NonNegative> {1.5}}}})
      == u8"brightness(1.5)");
    CHECK(SerialiseForTest(
            ContrastFunction {.Parameters = {Contrast::Parameter {Percentage<NonNegative> {200.0}}}})
          == u8"contrast(200%)");
    CHECK(SerialiseForTest(GrayscaleFunction {
            .Parameters = {Grayscale::Parameter {Percentage<ClosedPercentageRangeClampUpper> {50.0}}}})
          == u8"grayscale(50%)");
    CHECK(SerialiseForTest(HueRotateFunction {.Parameters = {HueRotate::Parameter {90_css_deg}}})
          == u8"hue-rotate(90deg)");
    CHECK(SerialiseForTest(
            InvertFunction {.Parameters = {Invert::Parameter {Number<ClosedUnitRangeClampUpper> {1.0}}}})
          == u8"invert(1)");
    CHECK(SerialiseForTest(OpacityFunction {
            .Parameters = {Opacity::Parameter {Percentage<ClosedPercentageRangeClampUpper> {25.0}}}})
          == u8"opacity(25%)");
    CHECK(SerialiseForTest(SaturateFunction {.Parameters = {Saturate::Parameter {Number<NonNegative> {3.0}}}})
          == u8"saturate(3)");
    CHECK(SerialiseForTest(
            SepiaFunction {.Parameters = {Sepia::Parameter {Number<ClosedUnitRangeClampUpper> {0.0}}}})
          == u8"sepia(0)");
  }

  TEST_CASE("Filter(Serialise drop-shadow)", "[CSS][FilterEffects][Filter]")
  {
    CHECK(SerialiseForTest(Shadow()) == u8"drop-shadow(red 1px 2px 3px)");
    CHECK(SerialiseForTest(DropShadowFunction {.Parameters = {.Location = {L {-1_css_px}, L {0_css_px}}}})
          == u8"drop-shadow(-1px 0px)");
    CHECK(SerialiseForTest(DropShadowFunction {
            .Parameters = {.Location = {L {1_css_em}, L {2_css_em}}, .StdDeviation = NL {0_css_px}}})
          == u8"drop-shadow(1em 2em 0px)");
  }

  TEST_CASE("Filter(Serialise property)", "[CSS][FilterEffects][Filter]")
  {
    CHECK(SerialiseForTest(Filter {keywords::None {}}) == u8"none");
    CHECK(SerialiseForTest(Reference(u8"#shadow")) == u8"url(\"#shadow\")");
    CHECK(SerialiseForTest(Filters({BlurFunction {.Parameters = {Blur::Parameter {2_css_px}}}}))
          == u8"blur(2px)");
    CHECK(SerialiseForTest(Filters({Reference(u8"#a"), Shadow(), SepiaFunction {}}))
          == u8"url(\"#a\") drop-shadow(red 1px 2px 3px) sepia()");
  }

  TEST_CASE("Filter(Equality)", "[CSS][FilterEffects][Filter]")
  {
    CHECK(BlurFunction {} == BlurFunction {});
    CHECK(BlurFunction {} != BlurFunction {.Parameters = {Blur::Parameter {0_css_px}}});
    CHECK(Shadow() == Shadow());

    auto noColor = Shadow();
    noColor.Parameters.Color = {};
    CHECK(noColor != Shadow());

    CHECK(Reference(u8"#a") == Reference(u8"#a"));
    CHECK(Reference(u8"#a") != Reference(u8"#b"));

    // Same parameter, different function.
    CHECK(FilterData {SepiaFunction {}} != FilterData {GrayscaleFunction {}});
    CHECK(FilterData {SepiaFunction {}} == FilterData {SepiaFunction {}});

    CHECK(Filter {keywords::None {}} == Filter {keywords::None {}});
    CHECK(Filter {keywords::None {}} != Filters({SepiaFunction {}}));
    CHECK(Filters({SepiaFunction {}, BlurFunction {}}) != Filters({BlurFunction {}, SepiaFunction {}}));
  }

  TEST_CASE("Filter(SwitchOn)", "[CSS][FilterEffects][Filter]")
  {
    FilterData data {Reference(u8"#a")};
    auto isReference =
      data.SwitchOn([](const FilterReference &) { return true; }, [](const auto &) { return false; });
    CHECK(isReference);
  }

  TEST_CASE("Filter(Dependencies and visitation)", "[CSS][FilterEffects][Filter]")
  {
    CHECK(CollectComputedStyleDependencies(Filter {keywords::None {}}).Properties.empty());
    CHECK(CollectComputedStyleDependencies(Filters({Shadow(), Reference(u8"#a")})).Properties.empty());
    CHECK(CollectComputedStyleDependencies(
            Filters({SepiaFunction {}, BlurFunction {.Parameters = {Blur::Parameter {1_css_em}}}}))
            .Properties
          == List<PropertyId> {PropertyId::FontSize});
    CHECK(CollectComputedStyleDependencies(
            DropShadowFunction {.Parameters = {.Location = {L {1_css_px}, L {2_css_em}}}})
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    // Raw numerics, keyword colours and URLs have no child values.
    CHECK(VisitValueChildren(visitor, Filters({Shadow(), Reference(u8"#a"), BlurFunction {}}))
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
