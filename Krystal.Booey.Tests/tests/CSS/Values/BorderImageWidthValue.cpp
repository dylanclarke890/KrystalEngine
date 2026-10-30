#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
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

  TEST_CASE_METHOD(ValueFixture, "BorderImageWidthValue::Create", "[CSS][Values][BorderImageWidthValue]")
  {
    auto value = BorderImageWidthValue::Create(Quad {Px(1.0), Px(2.0), Px(3.0), Px(4.0)}, false);
    CHECK(value->IsBorderImageWidthValue());
    CHECK_FALSE(value->OverridesBorderWidths());
    CHECK(CssText(value->Widths().Top()) == "1px");
    CHECK(CssText(value->Widths().Left()) == "4px");
    CHECK(BorderImageWidthValue::Create(Quad {Px(1.0)}, true)->OverridesBorderWidths());
  }

  TEST_CASE_METHOD(ValueFixture, "BorderImageWidthValue serialisation",
                   "[CSS][Values][BorderImageWidthValue]")
  {
    CHECK(CssText(*BorderImageWidthValue::Create(Quad {Px(1.0)}, false)) == "1px");
    CHECK(CssText(*BorderImageWidthValue::Create(Quad {Px(1.0), Px(2.0), Px(1.0), Px(2.0)}, false))
          == "1px 2px");
    CHECK(CssText(*BorderImageWidthValue::Create(
            Quad {PrimitiveValue::Create(ValueId::Auto), Px(2.0), Px(3.0), Px(4.0)}, false))
          == "auto 2px 3px 4px");

    // Only the -webkit-border-image shorthand overrides the border widths; the longhand serialises as empty.
    CHECK(CssText(*BorderImageWidthValue::Create(Quad {Px(1.0)}, true)) == "");
  }

  TEST_CASE_METHOD(ValueFixture, "BorderImageWidthValue equality", "[CSS][Values][BorderImageWidthValue]")
  {
    auto a = BorderImageWidthValue::Create(Quad {Px(1.0)}, false);
    CHECK(*a == *BorderImageWidthValue::Create(Quad {Px(1.0)}, false));
    CHECK_FALSE(*a == *BorderImageWidthValue::Create(Quad {Px(1.0)}, true));
    CHECK_FALSE(*a == *BorderImageWidthValue::Create(Quad {Px(2.0)}, false));
  }
}
