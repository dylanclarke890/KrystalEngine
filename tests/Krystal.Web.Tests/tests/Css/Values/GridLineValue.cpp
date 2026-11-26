#include "Krystal.Web/Css/Values/GridLineValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    RefPtr<PrimitiveValue> SpanKeyword()
    {
      return PrimitiveValue::Create(ValueId::Span);
    }

    RefPtr<PrimitiveValue> Int(double value)
    {
      return PrimitiveValue::CreateInteger(value);
    }

    RefPtr<PrimitiveValue> Name(const CSSOMString &name)
    {
      return PrimitiveValue::CreateCustomIdent(name);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineValue::Create", "[CSS][Values][GridLineValue]")
  {
    auto span = SpanKeyword();
    auto numeric = Int(2.0);
    auto name = Name(u8"header");
    auto value = GridLineValue::Create(span, numeric, name);

    CHECK(Is<GridLineValue>(*value));
    CHECK(value->SpanValue() == span.get());
    CHECK(value->NumericValue() == numeric.get());
    CHECK(value->GridLineName() == name.get());

    auto empty = GridLineValue::Create(nullptr, nullptr, nullptr);
    CHECK_FALSE(empty->SpanValue());
    CHECK_FALSE(empty->NumericValue());
    CHECK_FALSE(empty->GridLineName());
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineValue serialisation", "[CSS][Values][GridLineValue]")
  {
    SECTION("Single parts")
    {
      CHECK(CssText(*GridLineValue::Create(nullptr, Int(3.0), nullptr)) == "3");
      CHECK(CssText(*GridLineValue::Create(nullptr, nullptr, Name(u8"header"))) == "header");
    }

    SECTION("All parts")
    {
      CHECK(CssText(*GridLineValue::Create(SpanKeyword(), Int(2.0), Name(u8"header"))) == "span 2 header");
    }

    SECTION("1 is omitted only with both a span and a name")
    {
      CHECK(CssText(*GridLineValue::Create(SpanKeyword(), Int(1.0), Name(u8"header"))) == "span header");
      CHECK(CssText(*GridLineValue::Create(SpanKeyword(), Int(1.0), nullptr)) == "span 1");
      CHECK(CssText(*GridLineValue::Create(nullptr, Int(1.0), Name(u8"header"))) == "1 header");
      CHECK(CssText(*GridLineValue::Create(nullptr, Int(1.0), nullptr)) == "1");
    }

    SECTION("Negative lines")
    {
      CHECK(CssText(*GridLineValue::Create(nullptr, Int(-1.0), Name(u8"header"))) == "-1 header");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineValue equality", "[CSS][Values][GridLineValue]")
  {
    auto a = GridLineValue::Create(SpanKeyword(), Int(2.0), Name(u8"header"));
    CHECK(*a == *GridLineValue::Create(SpanKeyword(), Int(2.0), Name(u8"header")));
    CHECK(*GridLineValue::Create(nullptr, nullptr, nullptr)
          == *GridLineValue::Create(nullptr, nullptr, nullptr));

    CHECK_FALSE(*a == *GridLineValue::Create(nullptr, Int(2.0), Name(u8"header")));
    CHECK_FALSE(*a == *GridLineValue::Create(SpanKeyword(), Int(3.0), Name(u8"header")));
    CHECK_FALSE(*a == *GridLineValue::Create(SpanKeyword(), nullptr, Name(u8"header")));
    CHECK_FALSE(*a == *GridLineValue::Create(SpanKeyword(), Int(2.0), Name(u8"footer")));
    CHECK_FALSE(*a == *GridLineValue::Create(SpanKeyword(), Int(2.0), nullptr));
    CHECK_FALSE(*GridLineValue::Create(SpanKeyword(), Int(2.0), nullptr) == *a);
  }
}
