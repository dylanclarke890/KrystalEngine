#include "Krystal.Booey/CSS/Calc/TreeSerialisation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"

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

    // `Child` is move-only, so it can't come from an initializer list.
    template <typename... Cs>
    List<Child> ChildList(Cs &&...children)
    {
      List<Child> list;
      (list.push_back(std::forward<Cs>(children)), ...);
      return list;
    }

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
    CHECK(SerialiseTree(Node(Add(Num(1.0, UnitType::px), Num(50.0, UnitType::Percentage))))
          == u8"calc(50% + 1px)");
    CHECK(SerialiseTree(Node(Add(Num(2.0, UnitType::vw), Num(1.0, UnitType::em)))) == u8"calc(1em + 2vw)");
    CHECK(SerialiseTree(Node(Add(Num(1.0, UnitType::vmax), Num(1.0, UnitType::svb))))
          == u8"calc(1svb + 1vmax)");

    // Negative values and Negate nodes are written as subtraction.
    CHECK(SerialiseTree(Node(Add(Num(1.0, UnitType::px), Num(-2.0, UnitType::px)))) == u8"calc(1px - 2px)");
    CHECK(SerialiseTree(Node(Subtract(Num(1.0, UnitType::px), Num(2.0, UnitType::em))))
          == u8"calc(1px - 2em)");

    // Products, with Invert nodes as division.
    CHECK(SerialiseTree(Node(Multiply(Num(2.0), Num(3.0, UnitType::px)))) == u8"calc(2 * 3px)");
    CHECK(SerialiseTree(
            Node(Multiply(Num(3.0, UnitType::px), Node(Invert {.A = Num(2.0)}, Type::CreateNumber()))))
          == u8"calc(3px / 2)");

    // Nested sums keep their parentheses.
    CHECK(SerialiseTree(Node(Multiply(Num(2.0), Node(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em))))))
          == u8"calc(2 * (1em + 1px))");
  }

  // FIXME: a root Negate/Invert serialises only its child, dropping the operation (`calc(1em)`). Upstream
  // does the same: step 4 of serialising a math function serialises the root's children, and only Sum/Product
  // have overloads that serialise the node itself. Simplification normally removes these roots.
  TEST_CASE("TreeSerialisation(root negate and invert)", "[CSS][Calc][TreeSerialisation][!shouldfail]")
  {
    CHECK(SerialiseTree(Node(Negate {.A = Num(1.0, UnitType::em)})) == u8"calc(-1 * 1em)");
    CHECK(SerialiseTree(Node(Invert {.A = Num(2.0)}, Type::CreateNumber())) == u8"calc(1 / 2)");
  }

  TEST_CASE("TreeSerialisation(math functions)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(Node(Min {.Children = ChildList(Num(1.0, UnitType::px), Num(2.0, UnitType::em))}))
          == u8"min(1px, 2em)");
    CHECK(SerialiseTree(Node(Max {.Children = ChildList(Num(1.0, UnitType::px))})) == u8"max(1px)");
    CHECK(SerialiseTree(Node(
            Clamp {.Min = keywords::None {}, .Val = Num(1.0, UnitType::px), .Max = Num(2.0, UnitType::px)}))
          == u8"clamp(none, 1px, 2px)");
    CHECK(SerialiseTree(Node(RoundUp {.A = Num(1.5, UnitType::px), .B = Num(1.0, UnitType::px)}))
          == u8"round(up, 1.5px, 1px)");
    CHECK(SerialiseTree(Node(RoundNearest {.A = Num(1.5)}, Type::CreateNumber())) == u8"round(nearest, 1.5)");
    CHECK(SerialiseTree(Node(Mod {.A = Num(5.0, UnitType::px), .B = Num(2.0, UnitType::px)}))
          == u8"mod(5px, 2px)");
    CHECK(SerialiseTree(Node(Sin {.A = Num(90.0, UnitType::deg)}, Type::CreateNumber())) == u8"sin(90deg)");
    CHECK(SerialiseTree(Node(Log {.A = Num(8.0), .B = Num(2.0)}, Type::CreateNumber())) == u8"log(8, 2)");
    CHECK(SerialiseTree(Node(Log {.A = Num(8.0)}, Type::CreateNumber())) == u8"log(8)");

    // Arguments that are sums lose their parentheses.
    CHECK(SerialiseTree(Node(Abs {.A = Node(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em)))}))
          == u8"abs(1em + 1px)");
  }

  TEST_CASE("TreeSerialisation(leaves without calc)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(Child {Symbol {.Id = ValueId::R, .Unit = UnitType::Number}}) == u8"calc(r)");
    CHECK(SerialiseTree(Child {SiblingIndex {}}) == u8"sibling-index()");
    CHECK(SerialiseTree(Child {SiblingCount {}}) == u8"sibling-count()");

    // Serialising a child directly skips the math function.
    CHECK(SerialisationForCSS(Node(Add(Num(1.0, UnitType::px), Num(2.0, UnitType::px))),
                              {.Range = All, .SerialisationContext = DefaultSerialisationContext()})
          == u8"(1px + 2px)");
  }

  TEST_CASE("TreeSerialisation(random and anchors)", "[CSS][Calc][TreeSerialisation]")
  {
    CHECK(SerialiseTree(
            Node(Random {.Sharing = Random::SharingOptions {.Identifier = CSSOMStringAtom {u8"--a"},
                                                            .ElementShared = keywords::ElementShared {}},
                         .Min = Num(1.0, UnitType::px),
                         .Max = Num(5.0, UnitType::px),
                         .Step = Num(1.0, UnitType::px)}))
          == u8"random(--a element-shared, 1px, 5px, 1px)");
    CHECK(SerialiseTree(
            Node(Random {.Sharing = Random::SharingOptions {.Identifier = Random::SharingOptions::Auto {}},
                         .Min = Num(1.0, UnitType::px),
                         .Max = Num(5.0, UnitType::px)}))
          == u8"random(1px, 5px)");
    CHECK(SerialiseTree(
            Node(Random {.Sharing = Random::SharingFixed {.Value = css::Number<css::ClosedUnitRange> {0.5}},
                         .Min = Num(1.0, UnitType::px),
                         .Max = Num(5.0, UnitType::px)}))
          == u8"random(fixed 0.5, 1px, 5px)");

    CHECK(SerialiseTree(Node(Anchor {.ElementName = CSSOMStringAtom {u8"--a"},
                                     .Side = AnchorSide {ValueId::Top},
                                     .Fallback = Num(4.0, UnitType::px)}))
          == u8"anchor(--a top, 4px)");
    CHECK(SerialiseTree(Node(Anchor {.ElementName = CSSOMStringAtom::Null(),
                                     .Side = AnchorSide {Num(50.0, UnitType::Percentage)}}))
          == u8"anchor(50%)");
    CHECK(SerialiseTree(Node(AnchorSize {.ElementName = CSSOMStringAtom {u8"--a"},
                                         .Dimension = style::AnchorSizeDimension::SelfInline}))
          == u8"anchor-size(--a self-inline)");
    CHECK(SerialiseTree(
            Node(AnchorSize {.ElementName = CSSOMStringAtom::Null(), .Fallback = Num(1.0, UnitType::px)}))
          == u8"anchor-size(1px)");

    // Fallbacks that aren't leaves keep their math function.
    CHECK(
      SerialiseTree(Node(AnchorSize {.ElementName = CSSOMStringAtom::Null(),
                                     .Fallback = Node(Add(Num(1.0, UnitType::px), Num(1.0, UnitType::em)))}))
      == u8"anchor-size(calc(1em + 1px))");
  }
}
