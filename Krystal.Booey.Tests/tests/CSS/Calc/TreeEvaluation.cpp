#include "Krystal.Booey/CSS/Calc/TreeEvaluation.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include <cmath>

namespace krys::boo::css::calc::tests
{
  namespace
  {
    Child Num(double value, UnitType unit = UnitType::Number)
    {
      return CreateNumeric(value, unit);
    }

    template <typename Op>
    Child Node(Op &&op, Type type = Type::CreateLength())
    {
      return CreateChild(krys::move(op), type);
    }

    template <typename... Cs>
    List<Child> ChildList(Cs &&...children)
    {
      List<Child> list;
      (list.push_back(std::forward<Cs>(children)), ...);
      return list;
    }

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
    CHECK(Evaluate(Node(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px)))) == 100.0);
    CHECK(Evaluate(Node(Subtract(Num(10.0, UnitType::px), Num(4.0, UnitType::px)))) == 6.0);
    CHECK(Evaluate(Node(Multiply(Num(2.0), Num(3.0, UnitType::px)))) == 6.0);
    CHECK(Evaluate(Node(Multiply(Num(3.0, UnitType::px), Node(Invert {.A = Num(2.0)})))) == 1.5);
    CHECK(Evaluate(Node(Min {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::in))})) == 3.0);
    CHECK(Evaluate(Node(Max {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::in))}))
          == 96.0);
    CHECK(Evaluate(Node(Hypot {.Children = ChildList(Num(3.0, UnitType::px), Num(4.0, UnitType::px))}))
          == 5.0);
    CHECK(Evaluate(Node(
            Clamp {.Min = Num(1.0, UnitType::px), .Val = Num(5.0, UnitType::px), .Max = keywords::None {}}))
          == 5.0);
    CHECK(Evaluate(Node(
            Clamp {.Min = keywords::None {}, .Val = Num(5.0, UnitType::px), .Max = Num(2.0, UnitType::px)}))
          == 2.0);
    CHECK(Evaluate(Node(RoundUp {.A = Num(7.0, UnitType::px), .B = Num(5.0, UnitType::px)})) == 10.0);
    CHECK(Evaluate(Node(Pow {.A = Num(2.0), .B = Num(10.0)})) == 1024.0);
    CHECK(Evaluate(Node(Sin {.A = Num(0.5)})) == Catch::Approx(std::sin(0.5)));
    CHECK(Evaluate(Node(Progress {.Value = Num(5.0), .Start = Num(0.0), .End = Num(10.0)})) == 0.5);

    // Any child that can't be evaluated fails the whole operation.
    CHECK_FALSE(Evaluate(Node(Add(Num(1.0, UnitType::em), Num(4.0, UnitType::px)))));
    CHECK_FALSE(Evaluate(
      Node(Clamp {.Min = Num(1.0, UnitType::em), .Val = Num(5.0, UnitType::px), .Max = keywords::None {}})));

    // random() needs a style builder.
    CHECK_FALSE(
      Evaluate(Node(Random {.Sharing = Random::SharingFixed {css::Number<css::ClosedUnitRange> {0.5}},
                            .Min = Num(0.0, UnitType::px),
                            .Max = Num(10.0, UnitType::px),
                            .Step = null})));
  }

  // FIXME: trigonometric functions evaluate their argument as radians. Simplification converts canonical
  // angles (degrees) first, but evaluation doesn't, so an angle that reaches evaluation unsimplified (e.g.
  // `sin(sibling-index() * 10deg)` evaluated with `DoubleValue`) is read as radians. Upstream does the same.
  TEST_CASE("TreeEvaluation(trigonometric functions of angles)", "[CSS][Calc][TreeEvaluation][!shouldfail]")
  {
    CHECK(Evaluate(Node(Sin {.A = Num(90.0, UnitType::deg)})) == Catch::Approx(1.0));
  }

  TEST_CASE("TreeEvaluation(with default conversion data)", "[CSS][Calc][TreeEvaluation]")
  {
    // Default conversion data has no style, render view or builder: absolute lengths resolve, and
    // builder-dependent functions still fail.
    auto conversionData = ToLengthConversionData {};
    CHECK(Evaluate(Node(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px))), {}, conversionData) == 100.0);
    CHECK_FALSE(Evaluate(CreateChild(SiblingCount {}), {}, conversionData));
  }
}
