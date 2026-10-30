#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BoxShadowProperty.hpp"
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

    Color Red()
    {
      return Color {KeywordColor {ValueId::Red}};
    }

    BoxShadow Offset(L x, L y)
    {
      return BoxShadow {.Location = {x, y}};
    }

    BoxShadow Full()
    {
      return BoxShadow {.Color = Red(),
                        .Location = {L {1_css_px}, L {2_css_px}},
                        .Blur = NL {3_css_px},
                        .Spread = L {4_css_px},
                        .Inset = keywords::Inset {}};
    }
  }

  TEST_CASE("BoxShadow(Serialise)", "[CSS][Borders][BoxShadow]")
  {
    CHECK(SerialiseForTest(Offset(L {1_css_px}, L {2_css_px})) == u8"1px 2px");
    CHECK(SerialiseForTest(Offset(L {-1_css_px}, L {0_css_px})) == u8"-1px 0px");
    CHECK(SerialiseForTest(Full()) == u8"red 1px 2px 3px 4px inset");

    // Spread without blur, and inset without a colour.
    CHECK(SerialiseForTest(BoxShadow {
            .Location = {L {1_css_px}, L {2_css_px}}, .Spread = L {4_css_px}, .Inset = keywords::Inset {}})
          == u8"1px 2px 4px inset");
    CHECK(SerialiseForTest(
            BoxShadow {.Color = Red(), .Location = {L {1_css_em}, L {2_css_em}}, .Blur = NL {0_css_px}})
          == u8"red 1em 2em 0px");

    // `IsWebkitBoxShadow` doesn't affect serialisation.
    auto webkit = Full();
    webkit.IsWebkitBoxShadow = true;
    CHECK(SerialiseForTest(webkit) == SerialiseForTest(Full()));
  }

  TEST_CASE("BoxShadow(Serialise property)", "[CSS][Borders][BoxShadow]")
  {
    CHECK(SerialiseForTest(BoxShadowProperty {keywords::None {}}) == u8"none");
    CHECK(SerialiseForTest(BoxShadowProperty {BoxShadowProperty::List {Full()}})
          == u8"red 1px 2px 3px 4px inset");
    CHECK(SerialiseForTest(
            BoxShadowProperty {BoxShadowProperty::List {Offset(L {1_css_px}, L {2_css_px}), Full()}})
          == u8"1px 2px, red 1px 2px 3px 4px inset");
  }

  TEST_CASE("BoxShadow(Equality)", "[CSS][Borders][BoxShadow]")
  {
    CHECK(Full() == Full());
    CHECK(Offset(L {1_css_px}, L {2_css_px}) != Offset(L {2_css_px}, L {1_css_px}));
    CHECK(Offset(L {1_css_px}, L {2_css_px}) != Offset(L {1_css_em}, L {2_css_px}));

    auto noInset = Full();
    noInset.Inset = {};
    CHECK(noInset != Full());

    auto noColor = Full();
    noColor.Color = {};
    CHECK(noColor != Full());

    // Equal serialisations, different values.
    auto webkit = Full();
    webkit.IsWebkitBoxShadow = true;
    CHECK(webkit != Full());

    CHECK(BoxShadowProperty {keywords::None {}} == BoxShadowProperty {keywords::None {}});
    CHECK(BoxShadowProperty {keywords::None {}} != BoxShadowProperty {BoxShadowProperty::List {Full()}});
  }

  TEST_CASE("BoxShadow(Dependencies and visitation)", "[CSS][Borders][BoxShadow]")
  {
    CHECK(CollectComputedStyleDependencies(Full()).Properties.empty());
    CHECK(CollectComputedStyleDependencies(BoxShadowProperty {keywords::None {}}).Properties.empty());
    CHECK(CollectComputedStyleDependencies(Offset(L {1_css_em}, L {2_css_px})).Properties
          == List<PropertyId> {PropertyId::FontSize});
    CHECK(CollectComputedStyleDependencies(
            BoxShadowProperty {BoxShadowProperty::List {Full(), Offset(L {1_css_px}, L {2_css_em})}})
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    // Raw lengths and keyword colours have no child values.
    CHECK(VisitValueChildren(visitor, BoxShadowProperty {BoxShadowProperty::List {Full()}})
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
