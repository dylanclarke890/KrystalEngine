#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadowProperty.hpp"

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

    TextShadow Full()
    {
      return TextShadow {.Color = Color {KeywordColor {ValueId::Blue}},
                         .Location = {L {1_css_px}, L {-2_css_px}},
                         .Blur = NL {3_css_px}};
    }
  }

  TEST_CASE("TextShadow(Serialise)", "[CSS][TextDecoration][TextShadow]")
  {
    CHECK(SerialiseForTest(TextShadow {.Location = {L {1_css_px}, L {2_css_px}}}) == u8"1px 2px");
    CHECK(SerialiseForTest(Full()) == u8"blue 1px -2px 3px");
    CHECK(SerialiseForTest(TextShadow {.Location = {L {1_css_em}, L {0_css_px}}, .Blur = NL {2_css_px}})
          == u8"1em 0px 2px");

    CHECK(SerialiseForTest(TextShadowProperty {keywords::None {}}) == u8"none");
    CHECK(SerialiseForTest(TextShadowProperty {
            TextShadowProperty::List {Full(), TextShadow {.Location = {L {0_css_px}, L {0_css_px}}}}})
          == u8"blue 1px -2px 3px, 0px 0px");
  }

  TEST_CASE("TextShadow(Equality)", "[CSS][TextDecoration][TextShadow]")
  {
    CHECK(Full() == Full());

    auto noBlur = Full();
    noBlur.Blur = {};
    CHECK(noBlur != Full());

    auto noColor = Full();
    noColor.Color = {};
    CHECK(noColor != Full());

    CHECK(TextShadowProperty {keywords::None {}} == TextShadowProperty {keywords::None {}});
    CHECK(TextShadowProperty {keywords::None {}} != TextShadowProperty {TextShadowProperty::List {Full()}});
  }

  TEST_CASE("TextShadow(Dependencies and visitation)", "[CSS][TextDecoration][TextShadow]")
  {
    CHECK(CollectComputedStyleDependencies(Full()).Properties.empty());
    CHECK(CollectComputedStyleDependencies(
            TextShadow {.Location = {L {0_css_px}, L {0_css_px}}, .Blur = NL {1_css_em}})
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    CHECK(VisitValueChildren(visitor, TextShadowProperty {TextShadowProperty::List {Full()}})
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
