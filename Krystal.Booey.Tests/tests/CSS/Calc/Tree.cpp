#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/CSS/Calc/TreeComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Calc/TreeCopy.hpp"
#include "Krystal.Booey/CSS/Calc/TreeNumericIdentity.hpp"
#include "Krystal.Booey/CSS/Calc/TreeTraversal.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"

namespace krys::boo::css::calc::tests
{
  namespace
  {
    Child Px(double value)
    {
      return CreateNumeric(value, UnitType::px);
    }

    Child Em(double value)
    {
      return CreateNumeric(value, UnitType::em);
    }

    Child Percent(double value)
    {
      return CreateNumeric(value, UnitType::Percentage);
    }

    Tree CreateTree(Child &&root, Type type)
    {
      return Tree {.Root = krys::move(root), .Type = type, .Stage = Stage::Specified};
    }
  }

  TEST_CASE("Tree(creating numeric leaves)", "[CSS][Calc][Tree]")
  {
    CHECK(krys::HoldsAlternative<Number>(CreateNumeric(1.0, UnitType::Number)));
    CHECK(krys::HoldsAlternative<Number>(CreateNumeric(1.0, UnitType::Integer)));
    CHECK(krys::HoldsAlternative<Percentage>(Percent(1.0)));
    CHECK(krys::HoldsAlternative<NonCanonicalDimension>(Em(1.0)));

    auto px = Px(2.0);
    REQUIRE(krys::HoldsAlternative<CanonicalDimension>(px));
    CHECK(std::get<CanonicalDimension>(px.Value).Dimension == Dimension::Length);
    CHECK(std::get<CanonicalDimension>(px.Value).Value == 2.0);

    CHECK(std::get<CanonicalDimension>(CreateNumeric(1.0, UnitType::deg).Value).Dimension
          == Dimension::Angle);
    CHECK(std::get<CanonicalDimension>(CreateNumeric(1.0, UnitType::s).Value).Dimension == Dimension::Time);
    CHECK(std::get<CanonicalDimension>(CreateNumeric(1.0, UnitType::hz).Value).Dimension
          == Dimension::Frequency);
    CHECK(std::get<CanonicalDimension>(CreateNumeric(1.0, UnitType::dppx).Value).Dimension
          == Dimension::Resolution);
    CHECK(std::get<CanonicalDimension>(CreateNumeric(1.0, UnitType::fr).Value).Dimension == Dimension::Flex);

    CHECK(IsNumeric(Px(1.0)));
    CHECK_FALSE(IsNumeric(Child {Symbol {.Id = ValueId::R, .Unit = UnitType::Number}}));

    CHECK(ToUnit(Dimension::Angle) == UnitType::deg);
    CHECK(ToUnit(std::get<NonCanonicalDimension>(Em(1.0).Value)) == UnitType::em);

    auto doubled = CreateChildWithValueBasedOn(4.0, std::get<NonCanonicalDimension>(Em(1.0).Value));
    CHECK(std::get<NonCanonicalDimension>(doubled.Value).Value == 4.0);
    CHECK(std::get<NonCanonicalDimension>(doubled.Value).Unit == UnitType::em);
  }

  TEST_CASE("Tree(types)", "[CSS][Calc][Tree]")
  {
    CHECK(GetType(Px(1.0)) == Type::CreateLength());
    CHECK(GetType(Em(1.0)) == Type::CreateLength());
    CHECK(GetType(Percent(1.0)) == Type::CreatePercent());
    CHECK(GetType(Child {Symbol {.Id = ValueId::R, .Unit = UnitType::deg}}) == Type::CreateAngle());
    CHECK(GetType(Child {SiblingIndex {}}) == Type::CreateNumber());

    // px + % -> length with a length percent hint.
    auto sum = Add(Px(1.0), Percent(50.0));
    auto sumType = ToType(sum);
    REQUIRE(sumType);
    CHECK(sumType->Length == 1);
    CHECK(*sumType->PercentHint == PercentHint::Length);

    // px * px -> length².
    CHECK(ToType(Multiply(Px(1.0), Px(2.0)))->Length == 2);

    // px - deg can't be added.
    CHECK_FALSE(ToType(Subtract(Px(1.0), CreateNumeric(1.0, UnitType::deg))));

    // The indirect node carries the type it was made with.
    auto child = CreateChild(Add(Px(1.0), Px(2.0)), Type::CreateLength());
    CHECK(GetType(child) == Type::CreateLength());
    CHECK(GetType(std::get<IndirectNode<Sum>>(child.Value)) == Type::CreateLength());

    // sin() only accepts numbers or angles, and returns a number.
    CHECK(ToType(Sin {.A = CreateNumeric(1.0, UnitType::deg)}) == Type::CreateNumber());
    CHECK_FALSE(ToType(Sin {.A = Px(1.0)}));
    // asin() returns an angle.
    CHECK(ToType(Asin {.A = CreateNumeric(1.0, UnitType::Number)}) == Type::CreateAngle());

    // clamp() with none bounds only looks at the bounds that are present.
    CHECK(ToType(Clamp {.Min = keywords::None {}, .Val = Px(1.0), .Max = Em(2.0)}) == Type::CreateLength());
    CHECK_FALSE(
      ToType(Clamp {.Min = CreateNumeric(1.0, UnitType::s), .Val = Px(1.0), .Max = keywords::None {}}));

    // random() requires the same type.
    CHECK(ToType(Random {.Sharing = Random::SharingOptions {.Identifier = Random::SharingOptions::Auto {}},
                         .Min = Px(1.0),
                         .Max = Px(2.0)})
          == Type::CreateLength());

    // progress() returns a number.
    CHECK(ToType(Progress {.Value = Px(1.0), .Start = Px(0.0), .End = Px(2.0)}) == Type::CreateNumber());
  }

  TEST_CASE("Tree(equality and copying)", "[CSS][Calc][Tree]")
  {
    auto a = CreateTree(CreateChild(Add(Px(1.0), Em(2.0)), Type::CreateLength()), Type::CreateLength());
    auto b = CreateTree(CreateChild(Add(Px(1.0), Em(2.0)), Type::CreateLength()), Type::CreateLength());
    auto c = CreateTree(CreateChild(Add(Px(1.0), Em(3.0)), Type::CreateLength()), Type::CreateLength());
    CHECK(a == b);
    CHECK_FALSE(a == c);

    auto copy = Copy(a);
    CHECK(copy == a);
    // The copy owns its own nodes.
    CHECK(std::get<IndirectNode<Sum>>(copy.Root.Value).Op.get()
          != std::get<IndirectNode<Sum>>(a.Root.Value).Op.get());

    auto anchor = CreateTree(CreateChild(Anchor {.ElementName = CSSOMStringAtom {u8"--a"},
                                                 .Side = AnchorSide {ValueId::Top},
                                                 .Fallback = Px(4.0)},
                                         Type::CreateLength()),
                             Type::CreateLength());
    CHECK(Copy(anchor) == anchor);

    auto side = Copy(AnchorSide {Percent(10.0)});
    CHECK(krys::HoldsAlternative<Child>(side));
  }

  TEST_CASE("Tree(destructuring)", "[CSS][Calc][Tree]")
  {
    auto clamp = Clamp {.Min = Px(1.0), .Val = Px(2.0), .Max = keywords::None {}};
    STATIC_CHECK(std::tuple_size_v<Clamp> == 3);
    STATIC_CHECK(SameType<std::tuple_element_t<1, Clamp>, const Child &>);
    const auto &[min, val, max] = clamp;
    CHECK(krys::HoldsAlternative<Child>(min));
    CHECK(val == Px(2.0));
    CHECK(krys::HoldsAlternative<keywords::None>(max));
  }

  TEST_CASE("Tree(traversal)", "[CSS][Calc][Tree]")
  {
    auto sum = Add(Px(1.0), Em(2.0));
    size_t count = 0;
    ForAllChildNodes(sum, [&](const Child &) { ++count; });
    CHECK(count == 2);

    // `None` and missing optional children are skipped.
    count = 0;
    ForAllChildNodes(Clamp {.Min = keywords::None {}, .Val = Px(1.0), .Max = Px(2.0)},
                     [&](const Child &) { ++count; });
    CHECK(count == 2);
    count = 0;
    ForAllChildNodes(RoundNearest {.A = Px(1.0)}, [&](const Child &) { ++count; });
    CHECK(count == 1);

    // Leaves have no children.
    count = 0;
    ForAllChildNodes(Px(1.0), [&](const Child &) { ++count; });
    CHECK(count == 0);
    ForAllChildNodes(CreateChild(Add(Px(1.0), Px(2.0)), Type::CreateLength()),
                     [&](const Child &) { ++count; });
    CHECK(count == 2);
  }

  TEST_CASE("Tree(numeric identity)", "[CSS][Calc][Tree]")
  {
    STATIC_CHECK(NumberOfNumericIdentityTypes == 64);
    CHECK(ToNumericIdentity(Number {.Value = 1.0}) == NumericIdentity::Number);
    CHECK(ToNumericIdentity(std::get<CanonicalDimension>(Px(1.0).Value)) == NumericIdentity::PX);
    CHECK(ToNumericIdentity(std::get<NonCanonicalDimension>(Em(1.0).Value)) == NumericIdentity::EM);
    CHECK(ToNumericIdentity(std::get<NonCanonicalDimension>(CreateNumeric(1.0, UnitType::in).Value))
          == NumericIdentity::INCH);
    CHECK(IsLength(NumericIdentity::PX));
    CHECK(IsLength(NumericIdentity::CQMAX));
    CHECK_FALSE(IsLength(NumericIdentity::DEG));
    CHECK_FALSE(IsLength(NumericIdentity::Percentage));
  }

  TEST_CASE("Tree(computed style dependencies)", "[CSS][Calc][Tree]")
  {
    ComputedStyleDependencies dependencies;
    CollectComputedStyleDependencies(CreateTree(Px(1.0), Type::CreateLength()), dependencies);
    CHECK(dependencies.IsComputationallyIndependent());

    auto tree = CreateTree(CreateChild(Add(Em(1.0), CreateNumeric(1.0, UnitType::rem)), Type::CreateLength()),
                           Type::CreateLength());
    CollectComputedStyleDependencies(tree, dependencies);
    CHECK(dependencies.Properties == List<PropertyId> {PropertyId::FontSize});
    CHECK(dependencies.RootProperties == List<PropertyId> {PropertyId::FontSize});

    ComputedStyleDependencies symbolDependencies;
    CollectComputedStyleDependencies(
      CreateTree(Child {Symbol {.Id = ValueId::R, .Unit = UnitType::vw}}, Type::CreateLength()),
      symbolDependencies);
    CHECK(symbolDependencies.ViewportDimensions);

    ComputedStyleDependencies anchorDependencies;
    auto anchor = CreateTree(CreateChild(AnchorSize {.ElementName = CSSOMStringAtom::Null(),
                                                     .Dimension = style::AnchorSizeDimension::Width,
                                                     .Fallback = Em(1.0)},
                                         Type::CreateLength()),
                             Type::CreateLength());
    CollectComputedStyleDependencies(anchor, anchorDependencies);
    CHECK(anchorDependencies.Anchors);
    CHECK(anchorDependencies.Properties == List<PropertyId> {PropertyId::FontSize});
  }

  TEST_CASE("Tree(random caching keys)", "[CSS][Calc][Tree]")
  {
    RandomCachingKey a {Variant<Random::SharingOptions::Auto, CSSOMStringAtom> {
      Random::SharingOptions::Auto {.Property = PropertyId::Width, .Index = 0}}};
    RandomCachingKey b {Variant<Random::SharingOptions::Auto, CSSOMStringAtom> {
      Random::SharingOptions::Auto {.Property = PropertyId::Width, .Index = 1}}};
    RandomCachingKey named {
      Variant<Random::SharingOptions::Auto, CSSOMStringAtom> {CSSOMStringAtom {u8"--x"}}};

    CHECK(a == a);
    CHECK_FALSE(a == b);
    CHECK_FALSE(a == named);
    CHECK(RandomCachingKeyHash::Hash(a) == RandomCachingKeyHash::Hash(a));

    CHECK(HashTraits<RandomCachingKey>::IsEmptyValue(HashTraits<RandomCachingKey>::EmptyValue()));
    CHECK(RandomCachingKey {HashTableDeletedValueTag {}}.IsHashTableDeletedValue());

    auto map = RandomCachingKeyMap::Create();
    auto first = map->LookupCSSRandomBaseValue(a);
    CHECK(first >= 0.0);
    CHECK(first < 1.0);
    CHECK(map->LookupCSSRandomBaseValue(a) == first);
    CHECK(map->LookupCSSRandomBaseValue(named) == map->LookupCSSRandomBaseValue(named));
  }

  TEST_CASE("Value", "[CSS][Calc][Value]")
  {
    auto lengthTree = []
    {
      return CreateTree(CreateChild(Add(Px(1.0), Em(2.0)), Type::CreateLength()), Type::CreateLength());
    };

    auto value = calc::Value::Create(Category::Length, NonNegative, lengthTree());
    CHECK(value->GetCategory() == Category::Length);
    CHECK(value->GetRange() == NonNegative);
    CHECK(value->PrimitiveType() == UnitType::px);
    CHECK_FALSE(value->RequiresConversionData());
    CHECK(value->GetTree() == lengthTree());
    CHECK(value->Equals(*calc::Value::Create(Category::Length, All, lengthTree())));
    CHECK_FALSE(
      value->Equals(*calc::Value::Create(Category::Length, All, CreateTree(Px(1.0), Type::CreateLength()))));
    // Sum children are sorted by unit name.
    CHECK(value->CssText(DefaultSerialisationContext()) == u8"calc(2em + 1px)");

    ComputedStyleDependencies dependencies;
    value->CollectComputedStyleDependencies(dependencies);
    CHECK(dependencies.Properties == List<PropertyId> {PropertyId::FontSize});

    Type hinted = Type::CreateLength();
    hinted.ApplyPercentHint(PercentHint::Length);
    CHECK(
      calc::Value::Create(Category::LengthPercentage, All, CreateTree(Percent(5.0), hinted))->PrimitiveType()
      == UnitType::Percentage);
    CHECK(calc::Value::Create(Category::LengthPercentage, All,
                              CreateTree(CreateChild(Add(Px(1.0), Percent(5.0)), hinted), hinted))
            ->PrimitiveType()
          == UnitType::CalcPercentageWithLength);
    CHECK(calc::Value::Create(Category::LengthPercentage, All, CreateTree(Px(1.0), Type::CreateLength()))
            ->PrimitiveType()
          == UnitType::px);
    CHECK(
      calc::Value::Create(Category::Integer, All, CreateTree(CreateNumeric(1.0, UnitType::Number), Type {}))
        ->PrimitiveType()
      == UnitType::Integer);
  }
}
