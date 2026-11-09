#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSImageValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/Numeric/CSSMathSum.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Text(const CSSStyleValue &value)
    {
      return ToNarrow(value.ToString());
    }

    KRYS_NODISCARD Ref<CSSStyleValue> Parsed(dom::Document &document, CSSOMStringView property,
                                             CSSOMStringView cssText)
    {
      auto result = CSSStyleValue::Parse(document, property, cssText);
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    KRYS_NODISCARD dom::ExceptionCode ParseError(dom::Document &document, CSSOMStringView property,
                                                 CSSOMStringView cssText)
    {
      auto result = CSSStyleValue::Parse(document, property, cssText);
      REQUIRE(result.HasException());
      return result.ReleaseException().Code();
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSStyleValue: parse",
                   "[CSS][TypedOM][css-typed-om:dom-cssstylevalue-parse]")
  {
    auto document = CreateRef<dom::Document>();

    SECTION("numeric values")
    {
      auto width = Parsed(*document, u8"width", u8"10px");
      REQUIRE(Is<CSSUnitValue>(*width));
      CHECK(Downcast<CSSUnitValue>(*width).Value() == 10);
      CHECK(ToNarrow(CSSOMString(Downcast<CSSUnitValue>(*width).Unit())) == "px");

      // The property name is lowercased.
      CHECK(Text(*Parsed(*document, u8"WIDTH", u8"50%")) == "50%");
      CHECK(Text(*Parsed(*document, u8"opacity", u8"0.5")) == "0.5");

      auto sum = Parsed(*document, u8"width", u8"calc(10px + 1em)");
      CHECK(Is<CSSMathSum>(*sum));
    }

    SECTION("keywords")
    {
      auto display = Parsed(*document, u8"display", u8"BLOCK");
      REQUIRE(Is<CSSKeywordValue>(*display));
      CHECK(ToNarrow(Downcast<CSSKeywordValue>(*display).Value()) == "block");
      CHECK(Is<CSSKeywordValue>(*Parsed(*document, u8"width", u8"inherit")));
      // `none` filters and shadows, and keyword easing functions, are keywords.
      CHECK(Text(*Parsed(*document, u8"filter", u8"none")) == "none");
      CHECK(Text(*Parsed(*document, u8"box-shadow", u8"none")) == "none");
      CHECK(Is<CSSKeywordValue>(*Parsed(*document, u8"transition-timing-function", u8"ease-in")));
    }

    SECTION("unparsed values")
    {
      auto custom = Parsed(*document, u8"--x", u8" foo(1) ");
      REQUIRE(Is<CSSUnparsedValue>(*custom));
      CHECK(Text(*custom) == " foo(1) ");

      auto reference = Parsed(*document, u8"width", u8"var(--a)");
      REQUIRE(Is<CSSUnparsedValue>(*reference));
      CHECK(Text(*reference) == "var(--a)");
    }

    SECTION("images and transforms")
    {
      auto image = Parsed(*document, u8"background-image", u8"url(a.png)");
      CHECK(Is<CSSImageValue>(*image));

      auto transform = Parsed(*document, u8"transform", u8"translate(10px, 20px) rotate(45deg)");
      REQUIRE(Is<CSSTransformValue>(*transform));
      CHECK(Downcast<CSSTransformValue>(*transform).Length() == 2);
      CHECK(Text(*transform) == "translate(10px, 20px) rotate(45deg)");
    }

    SECTION("other values")
    {
      // Values the Typed OM has no class for, and shorthands, are generic style values.
      auto color = Parsed(*document, u8"color", u8"rgb(1, 2, 3)");
      CHECK(color->StyleValueType() == CSSStyleValueType::CSSStyleValue);
      CHECK(Text(*color) == "rgb(1, 2, 3)");
      // A named colour is a keyword.
      CHECK(Is<CSSKeywordValue>(*Parsed(*document, u8"color", u8"red")));
      CHECK(Text(*Parsed(*document, u8"margin", u8"1px 2px")) == "1px 2px");
    }

    SECTION("errors")
    {
      CHECK(ParseError(*document, u8"not-a-property", u8"1") == dom::ExceptionCode::TypeError);
      CHECK(ParseError(*document, u8"width", u8"red") == dom::ExceptionCode::TypeError);
      CHECK(ParseError(*document, u8"width", u8"") == dom::ExceptionCode::TypeError);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSStyleValue: parseAll",
                   "[CSS][TypedOM][css-typed-om:dom-cssstylevalue-parseall]")
  {
    auto document = CreateRef<dom::Document>();

    // A list-valued property's value is subdivided into its iterations.
    auto all = CSSStyleValue::ParseAll(*document, u8"transition-duration", u8"1s, 2s");
    REQUIRE(all.HasValue());
    REQUIRE(all.Value().size() == 2);
    CHECK(Text(*all.Value()[0]) == "1s");
    CHECK(Text(*all.Value()[1]) == "2s");
    CHECK(Text(*Parsed(*document, u8"transition-duration", u8"1s, 2s")) == "1s");

    auto single = CSSStyleValue::ParseAll(*document, u8"width", u8"1px");
    REQUIRE(single.HasValue());
    CHECK(single.Value().size() == 1);
  }
}
