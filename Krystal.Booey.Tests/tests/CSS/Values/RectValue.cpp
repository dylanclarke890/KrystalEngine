#include "Krystal.Booey/CSS/Values/RectValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<PrimitiveValue> Px(double value) noexcept
    {
      return PrimitiveValue::Create(value, UnitType::px);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "RectValue::Create", "[CSS][Values][RectValue]")
  {
    auto rect = RectValue::Create(Rect {Px(1.0), Px(2.0), Px(3.0), PrimitiveValue::Create(ValueId::Auto)});
    CHECK(rect->IsRectValue());
    CHECK(CssText(rect->GetRect().Top()) == "1px");
    CHECK(CssText(rect->GetRect().Right()) == "2px");
    CHECK(CssText(rect->GetRect().Bottom()) == "3px");
    CHECK(CssText(rect->GetRect().Left()) == "auto");

    const Value &value = *rect;
    CHECK(&value.GetRect() == &rect->GetRect());
  }

  TEST_CASE_METHOD(ValueFixture, "RectValue serialisation", "[CSS][Values][RectValue]")
  {
    CHECK(CssText(*RectValue::Create(Rect {Px(1.0), Px(2.0), Px(3.0), Px(4.0)}))
          == "rect(1px, 2px, 3px, 4px)");

    // Unlike a quad, repeated sides are never omitted.
    CHECK(CssText(*RectValue::Create(Rect {Px(1.0), Px(1.0), Px(1.0), Px(1.0)}))
          == "rect(1px, 1px, 1px, 1px)");
  }

  TEST_CASE_METHOD(ValueFixture, "RectValue equality", "[CSS][Values][RectValue]")
  {
    auto a = RectValue::Create(Rect {Px(1.0), Px(2.0), Px(3.0), Px(4.0)});
    CHECK(*a == *RectValue::Create(Rect {Px(1.0), Px(2.0), Px(3.0), Px(4.0)}));
    CHECK_FALSE(*a == *RectValue::Create(Rect {Px(9.0), Px(2.0), Px(3.0), Px(4.0)}));
    CHECK_FALSE(*a == *RectValue::Create(Rect {Px(1.0), Px(9.0), Px(3.0), Px(4.0)}));
    CHECK_FALSE(*a == *RectValue::Create(Rect {Px(1.0), Px(2.0), Px(9.0), Px(4.0)}));
    CHECK_FALSE(*a == *RectValue::Create(Rect {Px(1.0), Px(2.0), Px(3.0), Px(9.0)}));
  }
}
