#include "Krystal.Booey/CSS/Calc/TreeSerialisation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CalcTestUtils.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"

namespace krys::boo::css::calc::tests
{
  namespace
  {
    // `Child` is move-only, so it can't come from an initializer list.
    CSSOMString SerialiseTree(Child &&root, Stage stage = Stage::Specified, css::Range range = All)
    {
      auto tree = Tree {.Root = krys::move(root), .Type = Type::CreateLength(), .Stage = stage};
      return SerialisationForCSS(tree,
                                 {.Range = range, .SerialisationContext = DefaultSerialisationContext()});
    }
  }

  TEST_CASE("TreeSerialisation(numeric roots)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(Num(1.0, UnitType::px)) == u8"calc(1px)");
    CHECK(SerialiseTree(Num(1.5, UnitType::em)) == u8"calc(1.5em)");
    CHECK(SerialiseTree(Num(50.0, UnitType::Percentage)) == u8"calc(50%)");
    CHECK(SerialiseTree(Num(2.0)) == u8"calc(2)");
    CHECK(SerialiseTree(Num(Limits<double>::infinity(), UnitType::px)) == u8"calc(infinity * 1px)");
    CHECK(SerialiseTree(Num(-Limits<double>::infinity())) == u8"calc(-infinity)");

    // Computed values are clamped to the range and serialised without calc().
    CHECK(SerialiseTree(Num(-5.0, UnitType::px), Stage::Computed, NonNegative) == u8"0px");
    CHECK(SerialiseTree(Num(Limits<double>::quiet_NaN(), UnitType::px), Stage::Computed) == u8"0px");
  }

  TEST_CASE("TreeSerialisation(sums and products)", "[CSS][Calc][TreeSerialisation]")
  {
    // Children are sorted: numbers, percentages, then dimensions by unit name.
    CHECK(SerialiseTree(OperationNode(Add(Num(1.0, UnitType::px), Num(50.0, UnitType::Percentage))))
          == u8"calc(50% + 1px)");
    CHECK(SerialiseTree(OperationNode(Add(Num(2.0, UnitType::vw), Num(1.0, UnitType::em))))
          == u8"calc(1em + 2vw)");
    CHECK(SerialiseTree(OperationNode(Add(Num(1.0, UnitType::vmax), Num(1.0, UnitType::svb))))
          == u8"calc(1svb + 1vmax)");

    // Negative values and Negate nodes are written as subtraction.
    CHECK(SerialiseTree(OperationNode(Add(Num(1.0, UnitType::px), Num(-2.0, UnitType::px))))
          == u8"calc(1px - 2px)");
    CHECK(SerialiseTree(OperationNode(Subtract(Num(1.0, UnitType::px), Num(2.0, UnitType::em))))
          == u8"calc(1px - 2em)");

    // Products, with Invert nodes as division.
    CHECK(SerialiseTree(OperationNode(Multiply(Num(2.0), Num(3.0, UnitType::px)))) == u8"calc(2 * 3px)");
    CHECK(SerialiseTree(OperationNode(
            Multiply(Num(3.0, UnitType::px), OperationNode(Invert {.A = Num(2.0)}, Type::CreateNumber()))))
          == u8"calc(3px / 2)");

    // Nested sums keep their parentheses.
    CHECK(SerialiseTree(OperationNode(
            Multiply(Num(2.0), OperationNode(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em))))))
          == u8"calc(2 * (1em + 1px))");
  }

  // A root Negate or Invert is serialised itself, as a root Sum or Product is, rather than only its child.
  TEST_CASE("TreeSerialisation(root negate and invert)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(OperationNode(Negate {.A = Num(1.0, UnitType::em)})) == u8"calc(-1 * 1em)");
    CHECK(SerialiseTree(OperationNode(Invert {.A = Num(2.0)}, Type::CreateNumber())) == u8"calc(1 / 2)");
  }

  TEST_CASE("TreeSerialisation(math functions)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(
            OperationNode(Min {.Children = ChildList(Num(1.0, UnitType::px), Num(2.0, UnitType::em))}))
          == u8"min(1px, 2em)");
    CHECK(SerialiseTree(OperationNode(Max {.Children = ChildList(Num(1.0, UnitType::px))})) == u8"max(1px)");
    CHECK(SerialiseTree(OperationNode(
            Clamp {.Min = keywords::None {}, .Val = Num(1.0, UnitType::px), .Max = Num(2.0, UnitType::px)}))
          == u8"clamp(none, 1px, 2px)");
    CHECK(SerialiseTree(OperationNode(RoundUp {.A = Num(1.5, UnitType::px), .B = Num(1.0, UnitType::px)}))
          == u8"round(up, 1.5px, 1px)");
    CHECK(SerialiseTree(OperationNode(RoundNearest {.A = Num(1.5)}, Type::CreateNumber()))
          == u8"round(nearest, 1.5)");
    CHECK(SerialiseTree(OperationNode(Mod {.A = Num(5.0, UnitType::px), .B = Num(2.0, UnitType::px)}))
          == u8"mod(5px, 2px)");
    CHECK(SerialiseTree(OperationNode(Sin {.A = Num(90.0, UnitType::deg)}, Type::CreateNumber()))
          == u8"sin(90deg)");
    CHECK(SerialiseTree(OperationNode(Log {.A = Num(8.0), .B = Num(2.0)}, Type::CreateNumber()))
          == u8"log(8, 2)");
    CHECK(SerialiseTree(OperationNode(Log {.A = Num(8.0)}, Type::CreateNumber())) == u8"log(8)");

    // Arguments that are sums lose their parentheses.
    CHECK(SerialiseTree(
            OperationNode(Abs {.A = OperationNode(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em)))}))
          == u8"abs(1em + 1px)");
  }

  TEST_CASE("TreeSerialisation(leaves without calc)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(Child {Symbol {.Id = ValueId::R, .Unit = UnitType::Number}}) == u8"calc(r)");
    CHECK(SerialiseTree(Child {SiblingIndex {}}) == u8"sibling-index()");
    CHECK(SerialiseTree(Child {SiblingCount {}}) == u8"sibling-count()");

    // Serialising a child directly skips the math function.
    CHECK(SerialisationForCSS(OperationNode(Add(Num(1.0, UnitType::px), Num(2.0, UnitType::px))),
                              {.Range = All, .SerialisationContext = DefaultSerialisationContext()})
          == u8"(1px + 2px)");
  }

  TEST_CASE("TreeSerialisation(random and anchors)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(OperationNode(
            Random {.Sharing = Random::SharingOptions {.Identifier = u8"--a",
                                                       .ElementShared = keywords::ElementShared {}},
                    .Min = Num(1.0, UnitType::px),
                    .Max = Num(5.0, UnitType::px),
                    .Step = Num(1.0, UnitType::px)}))
          == u8"random(--a element-shared, 1px, 5px, 1px)");
    CHECK(SerialiseTree(OperationNode(
            Random {.Sharing = Random::SharingOptions {.Identifier = Random::SharingOptions::Auto {}},
                    .Min = Num(1.0, UnitType::px),
                    .Max = Num(5.0, UnitType::px)}))
          == u8"random(1px, 5px)");
    CHECK(SerialiseTree(OperationNode(
            Random {.Sharing = Random::SharingFixed {.Value = css::Number<css::ClosedUnitRange> {0.5}},
                    .Min = Num(1.0, UnitType::px),
                    .Max = Num(5.0, UnitType::px)}))
          == u8"random(fixed 0.5, 1px, 5px)");

    CHECK(SerialiseTree(OperationNode(Anchor {
            .ElementName = u8"--a", .Side = AnchorSide {ValueId::Top}, .Fallback = Num(4.0, UnitType::px)}))
          == u8"anchor(--a top, 4px)");
    CHECK(SerialiseTree(
            OperationNode(Anchor {.ElementName = null, .Side = AnchorSide {Num(50.0, UnitType::Percentage)}}))
          == u8"anchor(50%)");
    CHECK(SerialiseTree(OperationNode(
            AnchorSize {.ElementName = u8"--a", .Dimension = style::AnchorSizeDimension::SelfInline}))
          == u8"anchor-size(--a self-inline)");
    CHECK(SerialiseTree(OperationNode(AnchorSize {.ElementName = null, .Fallback = Num(1.0, UnitType::px)}))
          == u8"anchor-size(1px)");

    // Fallbacks that aren't leaves keep their math function.
    CHECK(
      SerialiseTree(OperationNode(AnchorSize {
        .ElementName = null, .Fallback = OperationNode(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em)))}))
      == u8"anchor-size(calc(1em + 1px))");
  }
}
