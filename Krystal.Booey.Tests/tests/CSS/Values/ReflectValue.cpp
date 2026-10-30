#include "Krystal.Booey/CSS/Values/ReflectValue.hpp"
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

  TEST_CASE_METHOD(ValueFixture, "ReflectValue::Create", "[CSS][Values][ReflectValue]")
  {
    SECTION("Without a mask")
    {
      auto value = ReflectValue::Create(ValueId::Below, Px(5.0), nullptr);
      CHECK(value->IsReflectValue());
      CHECK(value->Direction() == ValueId::Below);
      CHECK(CssText(value->Offset()) == "5px");
      CHECK(value->Mask() == nullptr);
    }

    SECTION("With a mask")
    {
      Ref<Value> mask = PrimitiveValue::Create(ValueId::None);
      auto value = ReflectValue::Create(ValueId::Above, Px(5.0), mask);
      CHECK(value->Mask() == mask.get());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ReflectValue serialisation", "[CSS][Values][ReflectValue]")
  {
    CHECK(CssText(*ReflectValue::Create(ValueId::Below, Px(5.0), nullptr)) == "below 5px");
    CHECK(CssText(*ReflectValue::Create(ValueId::Left, Px(0.0), PrimitiveValue::Create(ValueId::None)))
          == "left 0px none");
  }

  TEST_CASE_METHOD(ValueFixture, "ReflectValue equality", "[CSS][Values][ReflectValue]")
  {
    auto a = ReflectValue::Create(ValueId::Below, Px(5.0), nullptr);
    CHECK(*a == *ReflectValue::Create(ValueId::Below, Px(5.0), nullptr));
    CHECK_FALSE(*a == *ReflectValue::Create(ValueId::Above, Px(5.0), nullptr));
    CHECK_FALSE(*a == *ReflectValue::Create(ValueId::Below, Px(6.0), nullptr));
    CHECK_FALSE(*a == *ReflectValue::Create(ValueId::Below, Px(5.0), PrimitiveValue::Create(ValueId::None)));

    auto masked = ReflectValue::Create(ValueId::Below, Px(5.0), PrimitiveValue::Create(ValueId::None));
    CHECK(*masked == *ReflectValue::Create(ValueId::Below, Px(5.0), PrimitiveValue::Create(ValueId::None)));
    CHECK_FALSE(*masked
                == *ReflectValue::Create(ValueId::Below, Px(5.0), PrimitiveValue::Create(ValueId::Auto)));
  }

  TEST_CASE_METHOD(ValueFixture, "ReflectValue visits its offset and mask", "[CSS][Values][ReflectValue]")
  {
    SECTION("Without a mask")
    {
      auto value = ReflectValue::Create(ValueId::Below, Px(5.0), nullptr);
      auto children = Children(*value);
      REQUIRE(children.size() == 1uz);
      CHECK(children[0uz] == &value->Offset());
    }

    SECTION("With a mask")
    {
      auto value = ReflectValue::Create(ValueId::Below, Px(5.0), PrimitiveValue::Create(ValueId::None));
      auto children = Children(*value);
      REQUIRE(children.size() == 2uz);
      CHECK(children[0uz] == &value->Offset());
      CHECK(children[1uz] == value->Mask());
    }

    SECTION("Visitation stops when the visitor is done")
    {
      auto value = ReflectValue::Create(ValueId::Below, Px(5.0), PrimitiveValue::Create(ValueId::None));
      size_t visited = 0uz;
      auto status = value->VisitChildren(
        [&](Value &)
        {
          ++visited;
          return IterationStatus::Done;
        });
      CHECK(status == IterationStatus::Done);
      CHECK(visited == 1uz);
    }
  }
}
