#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSStyleProperties: a style rule's declarations", "[CSS][CSSOM][CSSStyleProperties]")
  {
    auto sheet = SheetFromText(u8"a { color: red; margin: 1px 2px; --x: foo; float: left; }");
    auto &rule = RuleAt<CSSStyleRule>(*sheet, 0);
    CSSStyleDeclaration &style = rule.Style();

    CHECK(style.GetStyleDeclarationType() == StyleDeclarationType::Style);
    CHECK(Is<CSSStyleProperties>(style));
    CHECK(style.ParentElement() == nullptr);
    CHECK(style.CssRules() == nullptr);
    CHECK(ToNarrow(style.CssText()) == "color: red; margin: 1px 2px; --x: foo; float: left;");

    SECTION("length and item")
    {
      CHECK(style.Length() == 7);
      CHECK(ToNarrow(style.Item(0)) == "color");
      CHECK(ToNarrow(style.Item(1)) == "margin-top");
      CHECK(ToNarrow(style.Item(5)) == "--x");
      CHECK(ToNarrow(style.Item(6)) == "float");
      CHECK(style.Item(7).empty());
      CHECK(style.IsSupportedPropertyIndex(6));
      CHECK_FALSE(style.IsSupportedPropertyIndex(7));
    }

    SECTION("getters")
    {
      CHECK(ToNarrow(style.GetPropertyValue(u8"color")) == "red");
      CHECK(ToNarrow(style.GetPropertyValue(u8"COLOR")) == "red");
      CHECK(ToNarrow(style.GetPropertyValue(u8"margin")) == "1px 2px");
      CHECK(ToNarrow(style.GetPropertyValue(u8"margin-left")) == "2px");
      CHECK(ToNarrow(style.GetPropertyValue(u8"--x")) == "foo");
      CHECK(style.GetPropertyValue(u8"--y").empty());
      CHECK(style.GetPropertyValue(u8"background").empty());
      CHECK(style.GetPropertyValue(u8"nonsense").empty());

      CHECK(style.GetPropertyPriority(u8"color").empty());
      CHECK(ToNarrow(style.GetPropertyShorthand(u8"margin-top")) == "margin");
      CHECK(style.GetPropertyShorthand(u8"color").empty());
      CHECK_FALSE(style.IsPropertyImplicit(u8"margin-top"));
      CHECK(style.ParentRule() == &rule);
    }

    SECTION("setProperty")
    {
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"IMPORTANT").HasException());
      CHECK(ToNarrow(style.GetPropertyValue(u8"color")) == "blue");
      CHECK(ToNarrow(style.GetPropertyPriority(u8"color")) == "important");

      REQUIRE_FALSE(style.SetProperty(u8"--x", u8"bar", u8"important").HasException());
      CHECK(ToNarrow(style.GetPropertyValue(u8"--x")) == "bar");
      CHECK(ToNarrow(style.GetPropertyPriority(u8"--x")) == "important");

      // An invalid value or priority, or an unknown property, changes nothing.
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"nonsense", u8"").HasException());
      REQUIRE_FALSE(style.SetProperty(u8"color", u8"green", u8"importantish").HasException());
      REQUIRE_FALSE(style.SetProperty(u8"nonsense", u8"green", u8"").HasException());
      CHECK(ToNarrow(style.GetPropertyValue(u8"color")) == "blue");

      CHECK(RuleText(rule)
            == "a { color: blue !important; margin: 1px 2px; --x: bar !important; float: left; }");
    }

    SECTION("removeProperty")
    {
      // Removing a shorthand doesn't return its value yet (FIXME in RemoveShorthandProperty).
      auto removed = style.RemoveProperty(u8"margin");
      REQUIRE(removed.HasValue());
      CHECK(removed.Value().empty());

      CHECK(style.Length() == 3);

      auto custom = style.RemoveProperty(u8"--x");
      REQUIRE(custom.HasValue());
      CHECK(ToNarrow(custom.Value()) == "foo");

      auto missing = style.RemoveProperty(u8"background");
      REQUIRE(missing.HasValue());
      CHECK(missing.Value().empty());

      CHECK(RuleText(rule) == "a { color: red; float: left; }");

      auto longhand = style.RemoveProperty(u8"color");
      REQUIRE(longhand.HasValue());
      CHECK(ToNarrow(longhand.Value()) == "red");
      CHECK(RuleText(rule) == "a { float: left; }");
    }

    SECTION("setCssText")
    {
      REQUIRE_FALSE(style.SetCssText(u8"padding: 3px; color: green").HasException());
      CHECK(ToNarrow(style.CssText()) == "padding: 3px; color: green;");
      CHECK(RuleText(rule) == "a { padding: 3px; color: green; }");
    }

    SECTION("cssFloat and the IDL attributes")
    {
      auto &properties = rule.Style();
      CHECK(ToNarrow(properties.CssFloat()) == "left");
      REQUIRE_FALSE(properties.SetCssFloat(u8"right").HasException());
      CHECK(ToNarrow(properties.GetPropertyValueInternal(PropertyId::Float)) == "right");

      CHECK(ToNarrow(properties.GetPropertyValueForCamelCasedIDLAttribute(u8"marginLeft")) == "2px");
      REQUIRE_FALSE(
        properties.SetPropertyValueForCamelCasedIDLAttribute(u8"marginLeft", u8"5px").HasException());
      CHECK(ToNarrow(properties.GetPropertyValueForDashedIDLAttribute(u8"margin-left")) == "5px");
      REQUIRE_FALSE(
        properties.SetPropertyValueForDashedIDLAttribute(u8"margin-left", u8"6px").HasException());
      CHECK(ToNarrow(properties.GetPropertyValueForCamelCasedIDLAttribute(u8"marginLeft")) == "6px");

      REQUIRE_FALSE(
        properties.SetPropertyValueForWebKitCasedIDLAttribute(u8"webkitUserModify", u8"read-write")
          .HasException());
      CHECK(ToNarrow(properties.GetPropertyValueForWebKitCasedIDLAttribute(u8"webkitUserModify"))
            == "read-write");
    }

    SECTION("copyProperties")
    {
      auto copy = rule.Style().CopyProperties();
      CHECK(ToNarrow(copy->AsText(DefaultSerialisationContext())) == ToNarrow(style.CssText()));
      copy->Clear();
      CHECK(style.Length() == 7);
    }
  }

  TEST_CASE("CSSStyleProperties: a rule that has lost its sheet can't change",
            "[CSS][CSSOM][CSSStyleProperties]")
  {
    auto rule = Parser::ParseRule(u8"a { color: red; }", CSSOMTestContext(), nullptr,
                                  Parser::AllowedRules::RegularRules);
    REQUIRE(rule);
    auto wrapper = rule->CreateCSSOMWrapper();
    auto &style = Downcast<CSSStyleRule>(*wrapper).Style();

    CHECK(style.ParentStyleSheet() == nullptr);
    REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
    REQUIRE_FALSE(style.SetCssText(u8"color: blue").HasException());
    REQUIRE(style.RemoveProperty(u8"color").HasValue());
    CHECK(ToNarrow(style.GetPropertyValue(u8"color")) == "red");
  }

  TEST_CASE("CSSStyleProperties: a declaration block's own wrapper", "[CSS][CSSOM][CSSStyleProperties]")
  {
    auto properties = ParseDeclarations(u8"color: red; margin: 1px");
    CHECK_FALSE(properties->HasCSSOMWrapper());
    CHECK(properties->CssStyleProperties() == nullptr);

    auto &style = properties->EnsureCSSStyleProperties();
    CHECK(properties->HasCSSOMWrapper());
    CHECK(&style == properties->CssStyleProperties());
    CHECK(&style == &properties->EnsureCSSStyleProperties());
    CHECK(style.ParentRule() == nullptr);
    CHECK(style.ParentStyleSheet() == nullptr);

    // It shares the declaration block's reference count.
    auto count = properties->GetRefCount();
    {
      RefPtr<CSSStyleDeclaration> held = ShareRefPtr<CSSStyleDeclaration>(&style);
      CHECK(properties->GetRefCount() == count + 1);
    }
    CHECK(properties->GetRefCount() == count);

    REQUIRE_FALSE(style.SetProperty(u8"color", u8"blue", u8"").HasException());
    CHECK(ToNarrow(properties->AsText(DefaultSerialisationContext())) == "color: blue; margin: 1px;");
    REQUIRE_FALSE(style.SetCssText(u8"padding: 0").HasException());
    CHECK(ToNarrow(style.CssText()) == "padding: 0px;");
  }

  TEST_CASE("CSSStyleProperties: property names from scripts", "[CSS][CSSOM][CSSStyleProperties]")
  {
    using enum PropertyId;
    auto id = [](const char8 *name)
    {
      return CSSStyleProperties::GetPropertyIdFromJavaScriptPropertyName(CSSOMStringAtom(name));
    };

    CHECK(id(u8"color") == Color);
    CHECK(id(u8"backgroundColor") == BackgroundColor);
    CHECK(id(u8"borderTopLeftRadius") == BorderTopLeftRadius);
    CHECK(id(u8"webkitUserModify") == WebkitUserModify);
    CHECK(id(u8"WebkitUserModify") == WebkitUserModify);
    CHECK(id(u8"cssFloat") == Invalid);
    CHECK(id(u8"BackgroundColor") == Invalid);
    CHECK(id(u8"background-color") == BackgroundColor);
    CHECK(id(u8"") == Invalid);
    CHECK(id(u8"nonsense") == Invalid);
    CHECK(id(u8"epubWritingMode") == Invalid);
    CHECK(CSSStyleProperties::GetPropertyIdFromJavaScriptPropertyName(CSSOMStringAtom::Null()) == Invalid);
  }
}
