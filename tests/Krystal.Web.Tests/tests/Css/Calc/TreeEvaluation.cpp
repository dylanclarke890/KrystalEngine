#include "Krystal.Web/Css/Calc/TreeEvaluation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/CalcTestUtils.hpp"
#include "Krystal.Web/Css/Calc/SymbolTable.hpp"
#include "Krystal.Web/Css/Calc/Tree.hpp"
#include "Krystal.Web/Css/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Web/Css/Values/Units.hpp"
#include <cmath>
#include <numbers>

namespace krys::web::css::calc::tests
{
  namespace
  {
    Maybe<double> Evaluate(Child &&root, const SymbolTable &symbols = {},
                           Maybe<ToLengthConversionData> conversionData = null)
    {
      auto tree = Tree {.Root = krys::move(root), .Type = Type::CreateLength(), .Stage = Stage::Specified};
      return EvaluateDouble(tree, EvaluationOptions {.Category = css::Category::Length,
                                                     .Range = All,
                                                     .ConversionData = krys::move(conversionData),
                                                     .SymbolTable = symbols});
    }
  }

  TEST_CASE("TreeEvaluation(leaves)", "[CSS][Calc][TreeEvaluation]")
  {
    CHECK(Evaluate(Num(2.0)) == 2.0);
    CHECK(Evaluate(Num(50.0, UnitType::Percentage)) == 50.0);
    CHECK(Evaluate(Num(3.0, UnitType::px)) == 3.0);
    CHECK(Evaluate(Num(1.0, UnitType::in)) == 96.0);
    CHECK(Evaluate(Num(1.0, UnitType::turn)) == 360.0);

    // Relative lengths can't be evaluated without conversion data.
    CHECK_FALSE(Evaluate(Num(1.0, UnitType::em)));
    CHECK_FALSE(Evaluate(Num(1.0, UnitType::vw)));

    SymbolTable symbols {{ValueId::R, UnitType::Percentage, 25.0}};
    CHECK(Evaluate(CreateChild(Symbol {.Id = ValueId::R, .Unit = UnitType::Percentage}), symbols) == 25.0);

    // Tree-counting functions need a style builder.
    CHECK_FALSE(Evaluate(CreateChild(SiblingCount {})));
    CHECK_FALSE(Evaluate(CreateChild(SiblingIndex {})));
  }

  TEST_CASE("TreeEvaluation(operations)", "[CSS][Calc][TreeEvaluation]")
  {
    CHECK(Evaluate(OperationNode(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px)))) == 100.0);
    CHECK(Evaluate(OperationNode(Subtract(Num(10.0, UnitType::px), Num(4.0, UnitType::px)))) == 6.0);
    CHECK(Evaluate(OperationNode(Multiply(Num(2.0), Num(3.0, UnitType::px)))) == 6.0);
    CHECK(Evaluate(OperationNode(Multiply(Num(3.0, UnitType::px), OperationNode(Invert {.A = Num(2.0)}))))
          == 1.5);
    CHECK(Evaluate(OperationNode(Min {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::in))}))
          == 3.0);
    CHECK(Evaluate(OperationNode(Max {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::in))}))
          == 96.0);
    CHECK(
      Evaluate(OperationNode(Hypot {.Children = ChildList(Num(3.0, UnitType::px), Num(4.0, UnitType::px))}))
      == 5.0);
    CHECK(Evaluate(OperationNode(
            Clamp {.Min = Num(1.0, UnitType::px), .Val = Num(5.0, UnitType::px), .Max = keywords::None {}}))
          == 5.0);
    CHECK(Evaluate(OperationNode(
            Clamp {.Min = keywords::None {}, .Val = Num(5.0, UnitType::px), .Max = Num(2.0, UnitType::px)}))
          == 2.0);
    CHECK(Evaluate(OperationNode(RoundUp {.A = Num(7.0, UnitType::px), .B = Num(5.0, UnitType::px)}))
          == 10.0);
    CHECK(Evaluate(OperationNode(Pow {.A = Num(2.0), .B = Num(10.0)})) == 1024.0);
    CHECK(Evaluate(OperationNode(Sin {.A = Num(0.5)})) == Catch::Approx(std::sin(0.5)));
    CHECK(Evaluate(OperationNode(Progress {.Value = Num(5.0), .Start = Num(0.0), .End = Num(10.0)})) == 0.5);

    // Any child that can't be evaluated fails the whole operation.
    CHECK_FALSE(Evaluate(OperationNode(Add(Num(1.0, UnitType::em), Num(4.0, UnitType::px)))));
    CHECK_FALSE(Evaluate(OperationNode(
      Clamp {.Min = Num(1.0, UnitType::em), .Val = Num(5.0, UnitType::px), .Max = keywords::None {}})));

    // random() needs a style builder.
    CHECK_FALSE(Evaluate(
      OperationNode(Random {.Sharing = Random::SharingFixed {css::Number<css::ClosedUnitRange> {0.5}},
                            .Min = Num(0.0, UnitType::px),
                            .Max = Num(10.0, UnitType::px),
                            .Step = null})));
  }

  // An angle that reaches evaluation unsimplified (such as `sin(sibling-index() * 10deg)`) is in canonical
  // degrees, as simplification takes it; a number is radians.
  TEST_CASE("TreeEvaluation(trigonometric functions of angles)", "[CSS][Calc][TreeEvaluation]")
  {
    CHECK(Evaluate(OperationNode(Sin {.A = Num(90.0, UnitType::deg)})) == Catch::Approx(1.0));
    CHECK(Evaluate(OperationNode(Cos {.A = Num(180.0, UnitType::deg)})) == Catch::Approx(-1.0));
    CHECK(Evaluate(OperationNode(Tan {.A = Num(45.0, UnitType::deg)})) == Catch::Approx(1.0));
    CHECK(Evaluate(OperationNode(Sin {.A = Num(std::numbers::pi / 2)})) == Catch::Approx(1.0));
  }

  TEST_CASE("TreeEvaluation(with default conversion data)", "[CSS][Calc][TreeEvaluation]")
  {
    // Default conversion data has no style, render view or builder: absolute lengths resolve, and
    // builder-dependent functions still fail.
    auto conversionData = ToLengthConversionData {};
    CHECK(Evaluate(OperationNode(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px))), {}, conversionData)
          == 100.0);
    CHECK_FALSE(Evaluate(CreateChild(SiblingCount {}), {}, conversionData));
  }
}
