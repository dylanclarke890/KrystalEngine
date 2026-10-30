#include "Krystal.Booey/CSS/Values/FontStyleRangeValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<ValueList> ObliqueRange(double from, double to)
    {
      return ValueList::CreateSpaceSeparated(PrimitiveValue::Create(from, UnitType::deg),
                                             PrimitiveValue::Create(to, UnitType::deg));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleRangeValue::Create", "[CSS][Values][FontStyleRangeValue]")
  {
    SECTION("Style only")
    {
      auto value = FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Italic));
      CHECK(value->IsFontStyleRangeValue());
      CHECK(CssText(*value->FontStyleValue) == "italic");
      CHECK_FALSE(value->ObliqueValues);
    }

    SECTION("Style and oblique range")
    {
      auto value =
        FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), ObliqueRange(10.0, 20.0));
      CHECK(CssText(*value->FontStyleValue) == "oblique");
      REQUIRE(value->ObliqueValues);
      CHECK(CssText(*value->ObliqueValues) == "10deg 20deg");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleRangeValue serialisation", "[CSS][Values][FontStyleRangeValue]")
  {
    CHECK(CssText(*FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Normal))) == "normal");
    CHECK(CssText(
            *FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), ObliqueRange(10.0, 20.0)))
          == "oblique 10deg 20deg");
    CHECK(CssText(*FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), nullptr))
          == "oblique");
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleRangeValue equality", "[CSS][Values][FontStyleRangeValue]")
  {
    auto a = FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), ObliqueRange(10.0, 20.0));
    auto b = FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), ObliqueRange(10.0, 20.0));
    auto otherStyle =
      FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Italic), ObliqueRange(10.0, 20.0));
    auto otherRange =
      FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), ObliqueRange(10.0, 30.0));
    auto noRange = FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *otherStyle);
    CHECK_FALSE(*a == *otherRange);
    CHECK_FALSE(*a == *noRange);
    CHECK_FALSE(*noRange == *a);
    CHECK(*noRange == *FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique)));
  }

  TEST_CASE_METHOD(ValueFixture, "FontStyleRangeValue visits its values",
                   "[CSS][Values][FontStyleRangeValue]")
  {
    SECTION("Style only")
    {
      auto value = FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Italic));
      auto children = Children(*value);
      REQUIRE(children.size() == 1uz);
      CHECK(children[0uz] == &*value->FontStyleValue);
    }

    SECTION("Style and oblique range")
    {
      auto value =
        FontStyleRangeValue::Create(PrimitiveValue::Create(ValueId::Oblique), ObliqueRange(10.0, 20.0));
      auto children = Children(*value);
      REQUIRE(children.size() == 2uz);
      CHECK(children[0uz] == &*value->FontStyleValue);
      CHECK(children[1uz] == value->ObliqueValues.get());
    }
  }
}
