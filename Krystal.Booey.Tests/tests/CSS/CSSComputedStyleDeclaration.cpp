#include "Krystal.Booey/CSS/CSSComputedStyleDeclaration.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Value(CSSStyleDeclaration &declaration, CSSOMStringView property)
    {
      return ToNarrow(declaration.GetPropertyValue(CSSOMString(property)));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSComputedStyleDeclaration: getComputedStyle", "[CSS][CSSOM]")
  {
    style::tests::ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a { color: rgb(0, 0, 255); margin: 1px 2px; --x: 3 }
      #a::before { content: "b"; color: rgb(255, 0, 0) }
    </style><div id=a></div>)"};
    auto &a = fixture.ById(u8"a");
    auto style = GetComputedStyle(a);

    CHECK(style->GetStyleDeclarationType() == StyleDeclarationType::Style);
    CHECK(Value(*style, u8"color") == "rgb(0, 0, 255)");
    CHECK(Value(*style, u8"margin") == "1px 2px");
    CHECK(Value(*style, u8"margin-left") == "2px");
    CHECK(Value(*style, u8"--x") == "3");
    CHECK(Value(*style, u8"not-a-property").empty());
    CHECK(style->GetPropertyPriority(u8"color").empty());
    CHECK(style->GetPropertyShorthand(u8"color").empty());
    CHECK_FALSE(style->IsPropertyImplicit(u8"color"));
    CHECK(style->CssText().empty());
    CHECK_FALSE(style->ParentRule());

    // The declaration is read-only.
    CHECK(style->SetProperty(u8"color", u8"red", u8"").HasException());
    CHECK(style->RemoveProperty(u8"color").HasException());
    CHECK(style->SetCssText(u8"color: red").HasException());

    // It lists every exposed computed property, then the custom properties.
    auto exposed = static_cast<uint32>(fixture.Document->ExposedComputedCSSPropertyIds().size());
    CHECK(exposed > 0);
    CHECK(style->Length() == exposed + 1);
    CHECK(ToNarrow(style->Item(0))
          == ToNarrow(ToString(fixture.Document->ExposedComputedCSSPropertyIds()[0])));
    CHECK(ToNarrow(style->Item(exposed)) == "--x");
    CHECK(style->Item(exposed + 1).empty());

    // It's live.
    REQUIRE_FALSE(a.SetAttribute(u8"style", u8"color: rgb(0, 128, 0)").HasException());
    CHECK(Value(*style, u8"color") == "rgb(0, 128, 0)");

    auto &properties = static_cast<CSSStyleProperties &>(*style);
    CHECK(ToNarrow(properties.GetPropertyValueInternal(PropertyId::MarginTop)) == "1px");
    CHECK(ToNarrow(properties.CopyProperties()->GetPropertyValue(PropertyId::MarginTop)) == "1px");

    SECTION("pseudo-elements")
    {
      auto before = GetComputedStyle(a, u8"::before");
      CHECK(Value(*before, u8"color") == "rgb(255, 0, 0)");
      CHECK(Value(*before, u8"content") == "\"b\"");

      // A string that doesn't start with a colon is ignored.
      CHECK(Value(*GetComputedStyle(a, u8"before"), u8"color") == "rgb(0, 128, 0)");

      // An invalid pseudo-element gives an empty declaration.
      auto invalid = GetComputedStyle(a, u8"::nonsense");
      CHECK(invalid->Length() == 0);
      CHECK(Value(*invalid, u8"color").empty());
    }
  }
}
