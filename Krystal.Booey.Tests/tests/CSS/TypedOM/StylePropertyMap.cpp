#include "Krystal.Booey/CSS/TypedOM/StylePropertyMap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetList.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSRGB.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"
#include "Krystal.Booey/DOM/Nodes/StyledElement.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    constexpr utf8_stringview StyleMapMarkup = u8R"(<!DOCTYPE html><style>
      #a { margin-left: 5px; --y: 1 }
    </style><div id=a style="width: 10px; --x: foo"></div>)";

    /// @brief The first value of @p property as text, or "undefined".
    KRYS_NODISCARD std::string Get(const StylePropertyMapReadOnly &map, CSSOMStringView property)
    {
      auto result = map.Get(property);
      REQUIRE(result.HasValue());
      return result.Value() ? ToNarrow(result.Value()->ToString()) : "undefined";
    }

    KRYS_NODISCARD List<CSSStyleValueOrString> Strings(std::initializer_list<utf8_stringview> values)
    {
      List<CSSStyleValueOrString> list;
      for (auto value : values)
      {
        list.push_back(CSSOMString(value));
      }
      return list;
    }

    KRYS_NODISCARD List<CSSStyleValueOrString> Values(Ref<CSSStyleValue> value)
    {
      List<CSSStyleValueOrString> list;
      list.push_back(RefPtr<CSSStyleValue>(krys::move(value)));
      return list;
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StylePropertyMap: inline style (attributeStyleMap)",
                   "[CSS][TypedOM][css-typed-om:inline-stylepropertymap-objects]")
  {
    style::tests::ResolverFixture fixture {utf8_string(StyleMapMarkup)};
    auto &element = Downcast<dom::StyledElement>(fixture.ById(u8"a"));
    auto &map = element.AttributeStyleMap();
    CHECK(&map == &element.AttributeStyleMap());

    SECTION("reading")
    {
      CHECK(map.Size() == 2);
      CHECK(Get(map, u8"width") == "10px");
      CHECK(Get(map, u8"WIDTH") == "10px");
      CHECK(Get(map, u8"height") == "undefined");
      CHECK(Get(map, u8"--x") == "foo");
      CHECK(map.Get(u8"bogus").GetException().Code() == dom::ExceptionCode::TypeError);
      CHECK(map.Has(u8"width").Value());
      CHECK_FALSE(map.Has(u8"height").Value());

      auto entries = map.Entries();
      REQUIRE(entries.size() == 2);
      CHECK(ToNarrow(entries[0].Property) == "width");
      CHECK(ToNarrow(entries[1].Property) == "--x");
    }

    SECTION("setting")
    {
      REQUIRE_FALSE(map.Set(u8"height", Strings({u8"20px"})).HasException());
      CHECK(Get(map, u8"height") == "20px");
      CHECK(ToNarrow(element.GetAttribute(u8"style").value_or(dom::DOMString())).find("height: 20px")
            != std::string::npos);

      REQUIRE_FALSE(map.Set(u8"height", Values(CSSNumericFactory::Px(30))).HasException());
      CHECK(Get(map, u8"height") == "30px");

      // A colour value is set as the colour its function makes; it isn't a length.
      auto red = CSSRGB::Create(1.0, 0.0, 0.0).ReleaseValue();
      REQUIRE_FALSE(map.Set(u8"color", Values(red)).HasException());
      CHECK(Get(map, u8"color") == "rgb(255, 0, 0)");
      CHECK(map.Set(u8"height", Values(red)).GetException().Code() == dom::ExceptionCode::TypeError);

      // A number isn't a length, and a single-valued property takes one value.
      CHECK(map.Set(u8"height", Values(CSSNumericFactory::Number(3))).GetException().Code()
            == dom::ExceptionCode::TypeError);
      CHECK(map.Set(u8"height", Strings({u8"1px", u8"2px"})).HasException());
      CHECK(map.Set(u8"height", Strings({u8"red"})).HasException());
      CHECK(map.Set(u8"bogus", Strings({u8"1px"})).HasException());

      // Shorthands are set from their serialisation.
      REQUIRE_FALSE(map.Set(u8"margin", Strings({u8"1px 2px"})).HasException());
      CHECK(Get(map, u8"margin") == "1px 2px");
      CHECK(Get(map, u8"margin-left") == "2px");

      // A custom property's value is unparsed.
      REQUIRE_FALSE(map.Set(u8"--x", Strings({u8"bar"})).HasException());
      CHECK(Is<CSSUnparsedValue>(*map.Get(u8"--x").Value()));
      CHECK(Get(map, u8"--x") == "bar");
    }

    SECTION("list-valued properties")
    {
      REQUIRE_FALSE(map.Set(u8"transition-duration", Strings({u8"1s", u8"2s"})).HasException());
      auto all = map.GetAll(u8"transition-duration");
      REQUIRE(all.HasValue());
      CHECK(all.Value().size() == 2);

      REQUIRE_FALSE(map.Append(u8"transition-duration", Strings({u8"3s"})).HasException());
      CHECK(map.GetAll(u8"transition-duration").Value().size() == 3);
      CHECK(map.Append(u8"width", Strings({u8"1px"})).HasException());
      CHECK(map.Append(u8"--x", Strings({u8"1"})).HasException());
    }

    SECTION("deleting and clearing")
    {
      REQUIRE_FALSE(map.Delete(u8"width").HasException());
      CHECK(Get(map, u8"width") == "undefined");
      REQUIRE_FALSE(map.Delete(u8"--x").HasException());
      CHECK(map.Size() == 0);
      CHECK(map.Delete(u8"bogus").HasException());

      REQUIRE_FALSE(map.Set(u8"width", Strings({u8"1px"})).HasException());
      map.Clear();
      CHECK(map.Size() == 0);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StylePropertyMap: computed style (computedStyleMap)",
                   "[CSS][TypedOM][css-typed-om:computed-stylepropertymapreadonly-objects]")
  {
    style::tests::ResolverFixture fixture {utf8_string(StyleMapMarkup)};
    auto &element = fixture.ById(u8"a");
    auto &map = element.ComputedStyleMap();
    CHECK(&map == &element.ComputedStyleMap());
    CHECK_FALSE(Is<StylePropertyMap>(map));

    CHECK(Get(map, u8"margin-left") == "5px");
    CHECK(Get(map, u8"width") == "10px");
    CHECK(Get(map, u8"--y") == "1");

    // The exposed properties, then the custom properties.
    auto exposed = fixture.Document->ExposedComputedCSSPropertyIds().size();
    CHECK(map.Size() == exposed + 2);
    auto entries = map.Entries();
    REQUIRE(entries.size() == exposed + 2);
    CHECK(ToNarrow(entries[exposed].Property) == "--x");
    CHECK(ToNarrow(entries[exposed + 1].Property) == "--y");

    // It's live.
    REQUIRE_FALSE(element.SetAttribute(u8"style", u8"width: 20px").HasException());
    CHECK(Get(map, u8"width") == "20px");
  }

  TEST_CASE_METHOD(ValueFixture, "StylePropertyMap: style rule (styleMap)",
                   "[CSS][TypedOM][css-typed-om:declared-stylepropertymap-objects]")
  {
    style::tests::ResolverFixture fixture {utf8_string(StyleMapMarkup)};
    auto &sheet = Downcast<CSSStyleSheet>(*fixture.Document->StyleSheets().Item(0));
    auto &rule = RuleAt<CSSStyleRule>(sheet, 0);
    auto &map = rule.StyleMap();
    CHECK(&map == &rule.StyleMap());

    CHECK(map.Size() == 2);
    CHECK(Get(map, u8"margin-left") == "5px");
    CHECK(Get(map, u8"--y") == "1");

    REQUIRE_FALSE(map.Set(u8"color", Strings({u8"red"})).HasException());
    CHECK(RuleText(rule).find("color: red") != std::string::npos);
    REQUIRE_FALSE(map.Delete(u8"margin-left").HasException());
    CHECK(Get(map, u8"margin-left") == "undefined");

    // The rule's style applies to the element.
    CHECK(Get(fixture.ById(u8"a").ComputedStyleMap(), u8"color") == "rgb(255, 0, 0)");

    map.Clear();
    CHECK(map.Size() == 0);
  }
}
