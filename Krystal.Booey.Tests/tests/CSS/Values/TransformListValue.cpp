#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<Value> TranslateXFunction(double px)
    {
      return FunctionValue::Create(ValueId::TranslateX, PrimitiveValue::Create(px, UnitType::px));
    }

    Ref<Value> RotateFunction(double deg)
    {
      return FunctionValue::Create(ValueId::Rotate, PrimitiveValue::Create(deg, UnitType::deg));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "TransformListValue::Create", "[CSS][Values][TransformListValue]")
  {
    SECTION("From a list")
    {
      auto value =
        TransformListValue::Create(ValueListBuilder {TranslateXFunction(10.0), RotateFunction(45.0)});
      CHECK(value->IsTransformListValue());
      CHECK(value->ContainsValueList());
      REQUIRE(value->Length() == 2u);
      CHECK(value->Item(0u)->IsFunctionValue());
      CHECK(value->Item(1u)->IsFunctionValue());
    }

    SECTION("From a single value")
    {
      auto value = TransformListValue::Create(TranslateXFunction(10.0));
      REQUIRE(value->Length() == 1u);
      CHECK(CssText(*value->Item(0u)) == "translateX(10px)");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "TransformListValue serialisation", "[CSS][Values][TransformListValue]")
  {
    CHECK(CssText(*TransformListValue::Create(TranslateXFunction(10.0))) == "translateX(10px)");
    CHECK(
      CssText(*TransformListValue::Create(ValueListBuilder {TranslateXFunction(10.0), RotateFunction(45.0)}))
      == "translateX(10px) rotate(45deg)");
  }

  TEST_CASE_METHOD(ValueFixture, "TransformListValue equality and hashing",
                   "[CSS][Values][TransformListValue]")
  {
    auto a = TransformListValue::Create(ValueListBuilder {TranslateXFunction(10.0), RotateFunction(45.0)});
    auto b = TransformListValue::Create(ValueListBuilder {TranslateXFunction(10.0), RotateFunction(45.0)});
    auto reordered =
      TransformListValue::Create(ValueListBuilder {RotateFunction(45.0), TranslateXFunction(10.0)});
    auto different =
      TransformListValue::Create(ValueListBuilder {TranslateXFunction(20.0), RotateFunction(45.0)});
    auto spaceList = ValueList::CreateSpaceSeparated(TranslateXFunction(10.0), RotateFunction(45.0));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *reordered);
    CHECK_FALSE(*a == *different);
    CHECK_FALSE(*a == *spaceList);

    size_t hashA = 0uz, hashB = 0uz, hashDifferent = 0uz;
    CHECK(a->AddHash(hashA));
    CHECK(b->AddHash(hashB));
    CHECK(different->AddHash(hashDifferent));
    CHECK(hashA == hashB);
    CHECK(hashA != hashDifferent);
  }

  TEST_CASE_METHOD(ValueFixture, "TransformListValue visits its items", "[CSS][Values][TransformListValue]")
  {
    auto value =
      TransformListValue::Create(ValueListBuilder {TranslateXFunction(10.0), RotateFunction(45.0)});
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == value->Item(0u));
    CHECK(children[1uz] == value->Item(1u));
  }
}
