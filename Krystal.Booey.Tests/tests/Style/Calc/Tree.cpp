#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/Style/Calc/TreeCopy.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Calc/TreeTraversal.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::style::calc::tests
{
  namespace
  {
    template <typename... Cs>
    List<Child> ChildList(Cs &&...children)
    {
      List<Child> list;
      (list.push_back(std::forward<Cs>(children)), ...);
      return list;
    }

    double Evaluate(Child &&root, double percentResolutionLength, float zoom = 1.0f)
    {
      return calc::Evaluate(Tree {.Root = krys::move(root)}, percentResolutionLength,
                            ZoomFactor {zoom, 1.0f});
    }
  }

  TEST_CASE("Style calc Tree(ComputeDepth)", "[Style][Calc][Tree]")
  {
    CHECK(ComputeDepth(Tree {.Root = CreateNumber(1.0)}) == 1);
    CHECK(ComputeDepth(Tree {.Root = Add(CreateNumber(1.0), CreateDimension(2.0))}) == 2);
    CHECK(ComputeDepth(
            Tree {.Root = Add(CreateNumber(1.0), Multiply(CreateDimension(2.0), CreatePercentage(3.0)))})
          == 3);
    CHECK(ComputeDepth(
            Tree {.Root = CreateChild(Clamp {.Min = css::keywords::None {},
                                             .Val = CreateDimension(1.0),
                                             .Max = Add(CreateDimension(1.0), CreatePercentage(1.0))})})
          == 3);
    CHECK(ComputeDepth(Tree {.Root = CreateChild(RoundUp {.A = CreateDimension(1.0), .B = null})}) == 2);
  }

  TEST_CASE("Style calc Tree(ForAllChildren)", "[Style][Calc][Tree]")
  {
    auto random = Random {.FixedValue = {.BaseValue = 0.5},
                          .Min = CreateDimension(0.0),
                          .Max = CreateDimension(10.0),
                          .Step = null};
    size_t children = 0;
    size_t fixed = 0;
    ForAllChildren(random, krys::CreateVisitor([&](const Random::Fixed &) { ++fixed; },
                                               [&](const auto &) { ++children; }));
    CHECK(fixed == 1);
    CHECK(children == 3);
  }

  TEST_CASE("Style calc Tree(Copy)", "[Style][Calc][Tree]")
  {
    auto tree =
      Tree {.Root = Add(CreateDimension(1.0),
                        CreateChild(Min {.Children = ChildList(CreatePercentage(10.0), CreateNumber(2.0))}))};
    auto copy = calc::Copy(tree);
    CHECK(copy == tree);

    auto blend = Tree {.Root = BlendChildren(CreateDimension(0.0), CreateDimension(10.0), 0.5)};
    CHECK(calc::Copy(blend) == blend);
  }

  TEST_CASE("Style calc Tree(Evaluate)", "[Style][Calc][Tree]")
  {
    CHECK(Evaluate(CreatePercentage(50.0), 200.0) == 100.0);
    CHECK(Evaluate(Add(CreateDimension(10.0), CreatePercentage(50.0)), 200.0) == 110.0);

    // Dimensions are scaled by the zoom, percentages aren't (the resolution length is already zoomed).
    CHECK(Evaluate(Add(CreateDimension(10.0), CreatePercentage(50.0)), 200.0, 2.0f) == 120.0);
    CHECK(Evaluate(CreateNumber(3.0), 0.0, 2.0f) == 6.0);

    CHECK(Evaluate(Subtract(CreatePercentage(100.0), CreateDimension(20.0)), 50.0) == 30.0);
    CHECK(Evaluate(Multiply(CreateNumber(2.0), CreatePercentage(10.0)), 100.0) == 20.0);
    CHECK(
      Evaluate(CreateChild(Min {.Children = ChildList(CreatePercentage(10.0), CreateDimension(15.0))}), 100.0)
      == 10.0);
    CHECK(
      Evaluate(CreateChild(Max {.Children = ChildList(CreatePercentage(10.0), CreateDimension(15.0))}), 100.0)
      == 15.0);
    CHECK(
      Evaluate(CreateChild(Clamp {
                 .Min = CreateDimension(5.0), .Val = CreatePercentage(1.0), .Max = css::keywords::None {}}),
               100.0)
      == 5.0);
    CHECK(
      Evaluate(CreateChild(Hypot {.Children = ChildList(CreateDimension(3.0), CreatePercentage(4.0))}), 100.0)
      == 5.0);
    CHECK(Evaluate(BlendChildren(CreateDimension(0.0), CreatePercentage(100.0), 0.25), 40.0) == 10.0);
    CHECK(Evaluate(CreateChild(Random {.FixedValue = {.BaseValue = 0.5},
                                       .Min = CreateDimension(0.0),
                                       .Max = CreateDimension(10.0),
                                       .Step = null}),
                   0.0)
          == 5.0);

    CHECK(calc::Evaluate(Tree {.Root = CreateDimension(4.0)}, 0.0, ZoomNeeded {}) == 4.0);
  }

  TEST_CASE("Style calc Value", "[Style][Calc][Value]")
  {
    auto value = Value::Create(css::Category::LengthPercentage, css::NonNegative,
                               Tree {.Root = Subtract(CreatePercentage(100.0), CreateDimension(20.0))});
    CHECK(value->GetCategory() == css::Category::LengthPercentage);
    CHECK(value->GetRange() == css::NonNegative);
    CHECK(value->Evaluate(50.0, ZoomNeeded {}) == 30.0);

    // Results are clamped to the range.
    CHECK(value->Evaluate(10.0, ZoomNeeded {}) == 0.0);

    // NaN evaluates to 0.
    auto nan = Value::Create(css::Category::LengthPercentage, css::All,
                             Tree {.Root = CreateDimension(Limits<double>::quiet_NaN())});
    CHECK(nan->Evaluate(0.0, ZoomFactor {1.0f, 1.0f}) == 0.0);

    auto copy = Value::Create(value->GetCategory(), value->GetRange(), value->CopyTree());
    CHECK(*copy == *value);
    CHECK(value->CopyRoot() == value->GetTree().Root);
  }
}
