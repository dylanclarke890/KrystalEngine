#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CalcTestUtils.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"

namespace krys::boo::css::calc::tests
{
  namespace
  {
    // `Child` is move-only, so it can't come from an initializer list.
    SimplificationOptions Options(css::Category category = css::Category::Length, bool removeZeros = false)
    {
      return SimplificationOptions {.Category = category,
                                    .Range = All,
                                    .ConversionData = null,
                                    .SymbolTable = {},
                                    .AllowZeroValueLengthRemovalFromSum = removeZeros};
    }

    Child Simplified(Child root, const SimplificationOptions &options = Options())
    {
      return CopyAndSimplify(root, options);
    }

    template <typename T>
    bool Holds(const Child &child)
    {
      return krys::HoldsAlternative<T>(child);
    }

    bool IsNumber(const Child &child, double value)
    {
      auto *number = get_if<Number>(&child);
      return number && number->Value == Catch::Approx(value);
    }

    bool IsPercentage(const Child &child, double value)
    {
      auto *percentage = get_if<Percentage>(&child);
      return percentage && percentage->Value == Catch::Approx(value);
    }

    bool IsCanonical(const Child &child, double value, Dimension dimension)
    {
      auto *canonical = get_if<CanonicalDimension>(&child);
      return canonical && canonical->Dimension == dimension && canonical->Value == Catch::Approx(value);
    }

    bool IsNonCanonical(const Child &child, double value, UnitType unit)
    {
      auto *nonCanonical = get_if<NonCanonicalDimension>(&child);
      return nonCanonical && nonCanonical->Unit == unit && nonCanonical->Value == Catch::Approx(value);
    }
  }

  TEST_CASE("TreeSimplification(canonicalize)", "[CSS][Calc][TreeSimplification]")
  {
    auto canonical = [](double value, UnitType unit)
    {
      return Canonicalize(NonCanonicalDimension {.Value = value, .Unit = unit}, null);
    };

    CHECK(canonical(1.0, UnitType::in).value().Value == 96.0);
    CHECK(canonical(2.54, UnitType::cm).value().Value == Catch::Approx(96.0));
    CHECK(canonical(72.0, UnitType::pt).value().Value == Catch::Approx(96.0));
    CHECK(canonical(1.0, UnitType::turn).value().Dimension == Dimension::Angle);
    CHECK(canonical(1.0, UnitType::turn).value().Value == 360.0);
    CHECK(canonical(1500.0, UnitType::ms).value().Value == Catch::Approx(1.5));
    CHECK(canonical(2.0, UnitType::khz).value().Value == 2000.0);
    CHECK(canonical(96.0, UnitType::dpi).value().Value == Catch::Approx(1.0));

    // Font, viewport and container relative lengths need conversion data.
    CHECK_FALSE(canonical(1.0, UnitType::em));
    CHECK_FALSE(canonical(1.0, UnitType::vw));
    CHECK_FALSE(canonical(1.0, UnitType::cqi));
  }

  TEST_CASE("TreeSimplification(leaves)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(IsCanonical(Simplified(Num(1.0, UnitType::in)), 96.0, Dimension::Length));
    CHECK(IsNonCanonical(Simplified(Num(2.0, UnitType::em)), 2.0, UnitType::em));
    CHECK(IsNumber(Simplified(Num(3.0)), 3.0));

    auto symbols = Options(css::Category::Number);
    symbols.SymbolTable = SymbolTable {{ValueId::R, UnitType::Number, 0.5}};
    CHECK(
      IsNumber(Simplified(CreateChild(Symbol {.Id = ValueId::R, .Unit = UnitType::Number}), symbols), 0.5));
    CHECK(krys::HoldsAlternative<Symbol>(
      Simplified(CreateChild(Symbol {.Id = ValueId::G, .Unit = UnitType::Number}), symbols)));

    // sibling-count() and sibling-index() need a style builder.
    CHECK(krys::HoldsAlternative<SiblingCount>(Simplified(CreateChild(SiblingCount {}))));
    CHECK(krys::HoldsAlternative<SiblingIndex>(Simplified(CreateChild(SiblingIndex {}))));
  }

  TEST_CASE("TreeSimplification(sum)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(IsCanonical(Simplified(OperationNode(Add(Num(1.0, UnitType::px), Num(2.0, UnitType::px)))), 3.0,
                      Dimension::Length));
    CHECK(IsCanonical(Simplified(OperationNode(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px)))), 100.0,
                      Dimension::Length));
    CHECK(IsNonCanonical(Simplified(OperationNode(Add(Num(1.0, UnitType::em), Num(2.0, UnitType::em)))), 3.0,
                         UnitType::em));

    // Mixed units stay a sum, and nested sums are flattened.
    auto mixed = Simplified(OperationNode(
      Sum {.Children = ChildList(Num(1.0, UnitType::em),
                                 OperationNode(Add(Num(2.0, UnitType::px), Num(1.0, UnitType::vw))),
                                 Num(3.0, UnitType::px))}));
    auto *sum = get_if<IndirectNode<Sum>>(&mixed);
    REQUIRE(sum);
    REQUIRE((*sum)->Children.size() == 3);
    CHECK(IsNonCanonical((*sum)->Children[0], 1.0, UnitType::em));
    CHECK(IsCanonical((*sum)->Children[1], 5.0, Dimension::Length));
    CHECK(IsNonCanonical((*sum)->Children[2], 1.0, UnitType::vw));

    // Zero lengths are only removed when allowed.
    CHECK(Holds<IndirectNode<Sum>>(
      Simplified(OperationNode(Add(Num(0.0, UnitType::px), Num(1.0, UnitType::em))))));
    CHECK(IsNonCanonical(Simplified(OperationNode(Add(Num(0.0, UnitType::px), Num(1.0, UnitType::em))),
                                    Options(css::Category::Length, true)),
                         1.0, UnitType::em));
    CHECK(IsCanonical(Simplified(OperationNode(Add(Num(0.0, UnitType::px), Num(0.0, UnitType::em))),
                                 Options(css::Category::Length, true)),
                      0.0, Dimension::Length));
  }

  TEST_CASE("TreeSimplification(product)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(IsNumber(Simplified(OperationNode(Multiply(Num(2.0), Num(3.0)), Type::CreateNumber())), 6.0));
    CHECK(IsCanonical(Simplified(OperationNode(Multiply(Num(2.0), Num(3.0, UnitType::px)))), 6.0,
                      Dimension::Length));
    CHECK(
      IsCanonical(Simplified(OperationNode(Multiply(
                    Num(3.0, UnitType::px), OperationNode(Invert {.A = Num(2.0)}, Type::CreateNumber())))),
                  1.5, Dimension::Length));

    // 9.3: a number times a sum of numerics distributes into the sum.
    auto distributed = Simplified(
      OperationNode(Multiply(Num(2.0), OperationNode(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em))))));
    auto *sum = get_if<IndirectNode<Sum>>(&distributed);
    REQUIRE(sum);
    CHECK(IsCanonical((*sum)->Children[0], 2.0, Dimension::Length));
    CHECK(IsNonCanonical((*sum)->Children[1], 2.0, UnitType::em));

    // 9.4: px * px isn't a type a math function can resolve to, so the product stays.
    CHECK(Holds<IndirectNode<Product>>(
      Simplified(OperationNode(Multiply(Num(1.0, UnitType::px), Num(1.0, UnitType::px))))));
  }

  TEST_CASE("TreeSimplification(negate and invert)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(
      IsCanonical(Simplified(OperationNode(Negate {.A = Num(3.0, UnitType::px)})), -3.0, Dimension::Length));
    CHECK(IsNonCanonical(
      Simplified(OperationNode(Negate {.A = OperationNode(Negate {.A = Num(3.0, UnitType::em)})})), 3.0,
      UnitType::em));
    CHECK(IsNumber(Simplified(OperationNode(Invert {.A = Num(4.0)}, Type::CreateNumber())), 0.25));
    CHECK(IsNonCanonical(
      Simplified(OperationNode(Invert {.A = OperationNode(Invert {.A = Num(2.0, UnitType::em)})})), 2.0,
      UnitType::em));

    // -(1em + 2px) negates each term.
    auto negatedSum = Simplified(
      OperationNode(Negate {.A = OperationNode(Add(Num(1.0, UnitType::em), Num(2.0, UnitType::px)))}));
    auto *sum = get_if<IndirectNode<Sum>>(&negatedSum);
    REQUIRE(sum);
    CHECK(IsNonCanonical((*sum)->Children[0], -1.0, UnitType::em));
    CHECK(IsCanonical((*sum)->Children[1], -2.0, Dimension::Length));

    // -(1em * 2em) negates one factor, not both.
    auto negatedProduct = Simplified(
      OperationNode(Negate {.A = OperationNode(Multiply(Num(1.0, UnitType::em), Num(2.0, UnitType::em)))}));
    auto *product = get_if<IndirectNode<Product>>(&negatedProduct);
    REQUIRE(product);
    CHECK(IsNonCanonical((*product)->Children[0], -1.0, UnitType::em));
    CHECK(IsNonCanonical((*product)->Children[1], 2.0, UnitType::em));
  }

  TEST_CASE("TreeSimplification(min, max and clamp)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(IsCanonical(
      Simplified(OperationNode(Min {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::px))})),
      1.0, Dimension::Length));
    CHECK(IsCanonical(
      Simplified(OperationNode(Max {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::in))})),
      96.0, Dimension::Length));

    auto partial = Simplified(OperationNode(
      Min {.Children = ChildList(Num(3.0, UnitType::px), Num(1.0, UnitType::em), Num(1.0, UnitType::px))}));
    auto *min = get_if<IndirectNode<Min>>(&partial);
    REQUIRE(min);
    REQUIRE((*min)->Children.size() == 2);
    CHECK(IsCanonical((*min)->Children[0], 1.0, Dimension::Length));
    CHECK(IsNonCanonical((*min)->Children[1], 1.0, UnitType::em));

    // Percentages that resolve against a length can't be compared yet.
    auto percentages = Simplified(OperationNode(Max {.Children = ChildList(Num(10.0, UnitType::Percentage),
                                                                           Num(20.0, UnitType::Percentage))}),
                                  Options(css::Category::LengthPercentage));
    CHECK(get_if<IndirectNode<Max>>(&percentages));
    CHECK(IsPercentage(Simplified(OperationNode(Max {.Children = ChildList(Num(10.0, UnitType::Percentage),
                                                                           Num(20.0, UnitType::Percentage))}),
                                  Options(css::Category::Percentage)),
                       20.0));

    CHECK(IsCanonical(
      Simplified(OperationNode(
        Clamp {.Min = Num(1.0, UnitType::px), .Val = Num(5.0, UnitType::px), .Max = Num(3.0, UnitType::px)})),
      3.0, Dimension::Length));
    CHECK(
      IsCanonical(Simplified(OperationNode(Clamp {
                    .Min = keywords::None {}, .Val = Num(5.0, UnitType::px), .Max = Num(3.0, UnitType::px)})),
                  3.0, Dimension::Length));
    CHECK(
      IsCanonical(Simplified(OperationNode(Clamp {
                    .Min = Num(6.0, UnitType::px), .Val = Num(5.0, UnitType::px), .Max = keywords::None {}})),
                  6.0, Dimension::Length));
    CHECK(
      IsNonCanonical(Simplified(OperationNode(Clamp {
                       .Min = keywords::None {}, .Val = Num(5.0, UnitType::em), .Max = keywords::None {}})),
                     5.0, UnitType::em));
    CHECK(Holds<IndirectNode<Clamp>>(Simplified(OperationNode(
      Clamp {.Min = Num(1.0, UnitType::em), .Val = Num(5.0, UnitType::px), .Max = Num(3.0, UnitType::px)}))));
  }

  TEST_CASE("TreeSimplification(stepped values)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(IsCanonical(
      Simplified(OperationNode(RoundNearest {.A = Num(7.0, UnitType::px), .B = Num(5.0, UnitType::px)})), 5.0,
      Dimension::Length));
    CHECK(IsCanonical(
      Simplified(OperationNode(RoundUp {.A = Num(7.0, UnitType::px), .B = Num(5.0, UnitType::px)})), 10.0,
      Dimension::Length));
    CHECK(
      IsNumber(Simplified(OperationNode(RoundDown {.A = Num(2.6), .B = null}, Type::CreateNumber())), 2.0));
    CHECK(Holds<IndirectNode<RoundNearest>>(
      Simplified(OperationNode(RoundNearest {.A = Num(7.0, UnitType::em), .B = null}))));
    CHECK(
      IsCanonical(Simplified(OperationNode(Mod {.A = Num(-7.0, UnitType::px), .B = Num(3.0, UnitType::px)})),
                  2.0, Dimension::Length));
    CHECK(
      IsCanonical(Simplified(OperationNode(Rem {.A = Num(-7.0, UnitType::px), .B = Num(3.0, UnitType::px)})),
                  -1.0, Dimension::Length));

    // Non-canonical units can't be resolved without conversion data.
    CHECK(Holds<IndirectNode<Mod>>(
      Simplified(OperationNode(Mod {.A = Num(7.0, UnitType::em), .B = Num(3.0, UnitType::em)}))));
  }

  TEST_CASE("TreeSimplification(trigonometric and exponential)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(
      IsNumber(Simplified(OperationNode(Sin {.A = Num(90.0, UnitType::deg)}, Type::CreateNumber())), 1.0));
    CHECK(IsNumber(Simplified(OperationNode(Cos {.A = Num(0.0)}, Type::CreateNumber())), 1.0));
    CHECK(IsCanonical(Simplified(OperationNode(Asin {.A = Num(1.0)}, Type::CreateAngle())), 90.0,
                      Dimension::Angle));
    CHECK(
      IsCanonical(Simplified(OperationNode(Atan2 {.A = Num(1.0, UnitType::px), .B = Num(1.0, UnitType::px)},
                                           Type::CreateAngle())),
                  45.0, Dimension::Angle));

    CHECK(IsNumber(Simplified(OperationNode(Pow {.A = Num(2.0), .B = Num(3.0)}, Type::CreateNumber())), 8.0));
    CHECK(IsNumber(Simplified(OperationNode(Sqrt {.A = Num(9.0)}, Type::CreateNumber())), 3.0));
    CHECK(IsNumber(Simplified(OperationNode(Log {.A = Num(8.0), .B = Num(2.0)}, Type::CreateNumber())), 3.0));
    CHECK(IsNumber(Simplified(OperationNode(Exp {.A = Num(0.0)}, Type::CreateNumber())), 1.0));
    CHECK(IsCanonical(Simplified(OperationNode(
                        Hypot {.Children = ChildList(Num(3.0, UnitType::px), Num(4.0, UnitType::px))})),
                      5.0, Dimension::Length));
    CHECK(Holds<IndirectNode<Hypot>>(Simplified(
      OperationNode(Hypot {.Children = ChildList(Num(3.0, UnitType::px), Num(4.0, UnitType::em))}))));
  }

  TEST_CASE("TreeSimplification(sign-related and progress)", "[CSS][Calc][TreeSimplification]")
  {
    CHECK(IsCanonical(Simplified(OperationNode(Abs {.A = Num(-2.0, UnitType::px)})), 2.0, Dimension::Length));
    CHECK(
      IsNumber(Simplified(OperationNode(Sign {.A = Num(-3.0, UnitType::em)}, Type::CreateNumber())), -1.0));
    CHECK(IsNumber(Simplified(OperationNode(Progress {.Value = Num(5.0, UnitType::px),
                                                      .Start = Num(0.0, UnitType::px),
                                                      .End = Num(10.0, UnitType::px)},
                                            Type::CreateNumber())),
                   0.5));

    // random() needs a style builder for its base value.
    auto random = Simplified(
      OperationNode(Random {.Sharing = Random::SharingFixed {css::Number<css::ClosedUnitRange> {0.5}},
                            .Min = Num(0.0, UnitType::px),
                            .Max = Num(10.0, UnitType::px),
                            .Step = null}));
    CHECK(get_if<IndirectNode<Random>>(&random));
  }

  TEST_CASE("TreeSimplification(CanSimplify and CopyAndSimplify(Tree))", "[CSS][Calc][TreeSimplification]")
  {
    auto tree = Tree {.Root = OperationNode(Add(Num(1.0, UnitType::px), Num(2.0, UnitType::px))),
                      .Type = Type::CreateLength(),
                      .Stage = Stage::Specified};
    CHECK(CanSimplify(tree, Options()));

    auto simplified = CopyAndSimplify(tree, Options());
    CHECK(IsCanonical(simplified.Root, 3.0, Dimension::Length));
    CHECK(simplified.Stage == Stage::Specified);
    CHECK_FALSE(CanSimplify(simplified, Options()));

    // The original is untouched.
    CHECK(get_if<IndirectNode<Sum>>(&tree.Root));
  }
}
