#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
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

  TEST_CASE_METHOD(ValueFixture, "FunctionValue::Create", "[CSS][Values][FunctionValue]")
  {
    SECTION("No arguments")
    {
      auto function = FunctionValue::Create(ValueId::Rotate);
      CHECK(function->IsFunctionValue());
      CHECK(function->Name() == ValueId::Rotate);
      CHECK(function->size() == 0u);
      CHECK(CssText(*function) == "rotate()");
    }

    SECTION("One to four arguments are comma-separated")
    {
      CHECK(CssText(*FunctionValue::Create(ValueId::Scale, PrimitiveValue::Create(2.0, UnitType::Number)))
            == "scale(2)");
      CHECK(CssText(*FunctionValue::Create(ValueId::Translate, Px(1.0), Px(2.0))) == "translate(1px, 2px)");
      CHECK(CssText(*FunctionValue::Create(ValueId::Translate, Px(1.0), Px(2.0), Px(3.0)))
            == "translate(1px, 2px, 3px)");
      CHECK(CssText(*FunctionValue::Create(ValueId::Translate, Px(1.0), Px(2.0), Px(3.0), Px(4.0)))
            == "translate(1px, 2px, 3px, 4px)");
    }

    SECTION("Argument list")
    {
      auto function = FunctionValue::Create(ValueId::Matrix,
                                            ValueListBuilder {PrimitiveValue::Create(1.0, UnitType::Number),
                                                              PrimitiveValue::Create(0.0, UnitType::Number),
                                                              PrimitiveValue::Create(0.0, UnitType::Number),
                                                              PrimitiveValue::Create(1.0, UnitType::Number),
                                                              PrimitiveValue::Create(5.0, UnitType::Number)});
      CHECK(function->size() == 5u);
      CHECK(function->Separator() == ValueSeparator::CommaSeparator);
      CHECK(CssText(*function) == "matrix(1, 0, 0, 1, 5)");
    }

    SECTION("The name keeps its case")
    {
      CHECK(CssText(*FunctionValue::Create(ValueId::TranslateX, Px(1.0))) == "translateX(1px)");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FunctionValue equality and hashing", "[CSS][Values][FunctionValue]")
  {
    auto a = FunctionValue::Create(ValueId::Translate, Px(1.0), Px(2.0));
    auto b = FunctionValue::Create(ValueId::Translate, Px(1.0), Px(2.0));
    auto differentName = FunctionValue::Create(ValueId::Scale, Px(1.0), Px(2.0));
    auto differentArgument = FunctionValue::Create(ValueId::Translate, Px(1.0), Px(3.0));
    auto fewerArguments = FunctionValue::Create(ValueId::Translate, Px(1.0));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *differentName);
    CHECK_FALSE(*a == *differentArgument);
    CHECK_FALSE(*a == *fewerArguments);

    size_t hashA = 0uz, hashB = 0uz, hashName = 0uz, hashArgument = 0uz;
    CHECK(a->AddHash(hashA));
    CHECK(b->AddHash(hashB));
    CHECK(differentName->AddHash(hashName));
    CHECK(differentArgument->AddHash(hashArgument));
    CHECK(hashA == hashB);
    CHECK(hashA != hashName);
    CHECK(hashA != hashArgument);
  }

  TEST_CASE_METHOD(ValueFixture, "FunctionValue visits its arguments", "[CSS][Values][FunctionValue]")
  {
    auto function =
      FunctionValue::Create(ValueId::Translate, Px(1.0), PrimitiveValue::Create(2.0, UnitType::em));

    auto children = Children(*function);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == &(*function)[0u]);
    CHECK(children[1uz] == &(*function)[1u]);

    CHECK(function->ComputedStyleDependencies().Properties == List<PropertyId> {PropertyId::FontSize});
  }
}
