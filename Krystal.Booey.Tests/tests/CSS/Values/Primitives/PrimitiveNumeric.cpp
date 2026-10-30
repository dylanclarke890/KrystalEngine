#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesCanonicalization.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesEvaluateCalc.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSymbolReplacement.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include <algorithm>

namespace krys::boo::css::tests
{
  namespace
  {
    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    // calc(5em): a length that needs conversion data to resolve.
    Ref<calc::Value> CreateEmCalc()
    {
      return calc::Value::Create(Category::Length, All,
                                 calc::Tree {.Root = calc::CreateNumeric(5.0, UnitType::em),
                                             .Type = calc::Type::CreateLength(),
                                             .Stage = calc::Stage::Specified,
                                             .RequiresConversionData = true});
    }
  }

  TEST_CASE("PrimitiveNumeric(raw values)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    Length<> length {10_css_px};
    CHECK(length.IsRaw());
    CHECK_FALSE(length.IsCalc());
    CHECK_FALSE(length.IsEmpty());
    CHECK(length == 10_css_px);
    CHECK_FALSE(length == 10_css_em);
    CHECK(length == LengthRaw<> {LengthUnit::Px, 10.0});
    REQUIRE(length.GetRaw());
    CHECK(length.GetRaw()->Unit == LengthUnit::Px);
    CHECK(length.GetRaw()->Value == 10.0);
    CHECK_FALSE(length.GetCalc());
    CHECK(length.HoldsAlternative<LengthRaw<>>());
    CHECK_FALSE(length.HoldsAlternative<Length<>::Calc>());

    CHECK(length.SwitchOn([](const LengthRaw<> &raw) { return raw.Value; },
                          [](const Length<>::Calc &) { return -1.0; })
          == 10.0);

    CHECK(Number<> {0.0}.IsKnownZero());
    CHECK_FALSE(Number<> {0.0}.IsKnownNotZero());
    CHECK(Number<> {2.0}.IsKnownNotZero());

    Length<> copy = length;
    CHECK(copy == length);
    Length<> moved = krys::move(copy);
    CHECK(moved == length);
    copy = moved;
    CHECK(copy == length);
    copy = Length<> {3_css_em};
    CHECK(copy == 3_css_em);
  }

  TEST_CASE("PrimitiveNumeric(composite units)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    LengthPercentage<> percentage {50_css_percentage};
    CHECK(percentage == 50_css_percentage);
    CHECK(percentage == PercentageRaw<> {50.0});

    LengthPercentage<> length {4_css_px};
    CHECK(length == 4_css_px);
    CHECK(length == LengthRaw<> {LengthUnit::Px, 4.0});
    CHECK_FALSE(length == percentage);

    AnglePercentage<> angle {90_css_deg};
    CHECK(angle == 90_css_deg);
  }

  TEST_CASE("PrimitiveNumeric(calc values)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    auto value = CreateEmCalc();
    REQUIRE(value->HasOneRef());

    {
      Length<> length {
        Length<>::Calc {calc::Value::Create(value->GetCategory(), value->GetRange(),
                                            calc::Tree {.Root = calc::CreateNumeric(5.0, UnitType::em),
                                                        .Type = calc::Type::CreateLength(),
                                                        .Stage = calc::Stage::Specified,
                                                        .RequiresConversionData = true})}};
      Length<> sharing {Length<>::Calc {*value}};
      CHECK(sharing.IsCalc());
      CHECK_FALSE(sharing.IsRaw());
      CHECK_FALSE(sharing.IsKnownZero());
      CHECK_FALSE(sharing.GetRaw());
      REQUIRE(sharing.GetCalc());
      CHECK(value->GetRefCount() == 2u);

      // Equal trees compare equal even when they are different values.
      CHECK(sharing == length);
      CHECK_FALSE(sharing == Length<> {5_css_em});

      {
        Length<> copy = sharing;
        CHECK(value->GetRefCount() == 3u);
        Length<> moved = krys::move(copy);
        CHECK(value->GetRefCount() == 3u);
        moved = Length<> {1_css_px};
        CHECK(value->GetRefCount() == 2u);
        moved = sharing;
        CHECK(value->GetRefCount() == 3u);
        moved = moved;
        CHECK(value->GetRefCount() == 3u);
      }

      CHECK(value->GetRefCount() == 2u);
      CHECK(RequiresConversionData(sharing));
      CHECK_FALSE(RequiresConversionData(Length<> {1_css_px}));
      CHECK(IsUnevaluatedCalc(sharing));
      CHECK_FALSE(IsUnevaluatedCalc(Length<> {1_css_px}));
    }

    CHECK(value->HasOneRef());
  }

  TEST_CASE("PrimitiveNumeric(serialisation)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    CHECK(SerialiseForTest(Length<> {10_css_px}) == u8"10px");
    CHECK(SerialiseForTest(Number<> {2.5}) == u8"2.5");
    CHECK(SerialiseForTest(LengthPercentage<> {50_css_percentage}) == u8"50%");
    CHECK(SerialiseForTest(AngleRaw<> {AngleUnit::Turn, 0.5}) == u8"0.5turn");
    CHECK(SerialiseForTest(SerialisableNumber {Limits<double>::infinity(), u8"px"}) == u8"infinity * 1px");
    CHECK(SerialiseForTest(Length<> {Length<>::Calc {CreateEmCalc()}}) == u8"calc(5em)");
  }

  TEST_CASE("PrimitiveNumeric(computed style dependencies)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    CHECK(CollectComputedStyleDependencies(Length<> {1_css_px}).IsComputationallyIndependent());

    auto em = CollectComputedStyleDependencies(Length<> {1_css_em});
    CHECK(em.Properties == List<PropertyId> {PropertyId::FontSize});

    auto rlh = CollectComputedStyleDependencies(LengthRaw<> {LengthUnit::Rlh, 1.0});
    CHECK(rlh.RootProperties == List<PropertyId> {PropertyId::FontSize, PropertyId::LineHeight});

    ComputedStyleDependencies dependencies;
    CollectComputedStyleDependencies(dependencies, LengthRaw<> {LengthUnit::Lh, 1.0});
    CollectComputedStyleDependencies(dependencies, LengthRaw<> {LengthUnit::Em, 1.0});
    CHECK(dependencies.Properties == List<PropertyId> {PropertyId::FontSize, PropertyId::LineHeight});

    CHECK(CollectComputedStyleDependencies(LengthRaw<> {LengthUnit::Cqw, 1.0}).ContainerDimensions);
    CHECK(CollectComputedStyleDependencies(LengthRaw<> {LengthUnit::Svh, 1.0}).ViewportDimensions);
    CHECK(CollectComputedStyleDependencies(LengthPercentage<> {1_css_em}).Properties
          == List<PropertyId> {PropertyId::FontSize});

    auto calc = CollectComputedStyleDependencies(Length<> {Length<>::Calc {CreateEmCalc()}});
    CHECK(calc.Properties == List<PropertyId> {PropertyId::FontSize});
  }

  TEST_CASE("PrimitiveNumeric(value visitation)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    CHECK(VisitValueChildren(visitor, LengthRaw<> {LengthUnit::Px, 1.0}) == IterationStatus::Continue);
    CHECK(VisitValueChildren(visitor, Length<> {1_css_px}) == IterationStatus::Continue);
    CHECK(VisitValueChildren(visitor, Length<> {Length<>::Calc {CreateEmCalc()}})
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }

  TEST_CASE("PrimitiveNumeric(canonicalisation)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    CHECK(Canonicalize(AngleRaw<> {AngleUnit::Turn, 0.5}) == 180.0);
    CHECK(Canonicalize(TimeRaw<> {TimeUnit::Ms, 1500.0}) == 1.5);
    CHECK(Canonicalize(FrequencyRaw<> {FrequencyUnit::Khz, 2.0}) == 2000.0);
    CHECK(Canonicalize(ResolutionRaw<> {ResolutionUnit::X, 2.0}) == 2.0);
    CHECK(Canonicalize(ResolutionRaw<> {ResolutionUnit::Dppx, 3.0}) == 3.0);
    CHECK(Canonicalize(ResolutionRaw<> {ResolutionUnit::Dpi, 96.0}) == 1.0);
  }

  TEST_CASE("PrimitiveNumeric(symbol replacement)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    SymbolTable symbols {{ValueId::R, UnitType::Number, 128.0}};

    CHECK(ReplaceSymbol(Symbol {SymbolRaw {ValueId::R}}, symbols) == Number<> {128.0});
    CHECK(ReplaceSymbol(Number<> {1.0}, symbols) == Number<> {1.0});

    using Component = Variant<Number<>, Symbol>;
    auto replaced = ReplaceSymbol(Component {Symbol {SymbolRaw {ValueId::R}}}, symbols);
    CHECK(replaced == Number<> {128.0});
  }

  TEST_CASE("PrimitiveNumeric(calc evaluation)", "[CSS][Primitives][PrimitiveNumeric]")
  {
    // calc(1in + 4px) doesn't need conversion data, so it evaluates to a raw value in the canonical unit.
    auto absolute = Length<>::Calc {calc::Value::Create(
      Category::Length, All,
      calc::Tree {.Root = calc::CreateChild(
                    calc::Add(calc::CreateNumeric(1.0, UnitType::in), calc::CreateNumeric(4.0, UnitType::px)),
                    calc::Type::CreateLength()),
                  .Type = calc::Type::CreateLength(),
                  .Stage = calc::Stage::Specified})};
    CHECK(absolute.Evaluate(Category::Length, NoConversionDataRequiredTag {}) == 100.0);
    CHECK(EvaluateCalc(absolute, NoConversionDataRequiredTag {}, {}) == LengthRaw<> {LengthUnit::Px, 100.0});

    auto evaluated = EvaluateCalcIfNoConversionDataRequired(Length<> {absolute}, {});
    CHECK(evaluated == Length<> {LengthRaw<> {LengthUnit::Px, 100.0}});

    // calc(5em) needs conversion data, so it stays a calc.
    auto em = Length<> {Length<>::Calc {CreateEmCalc()}};
    CHECK(EvaluateCalcIfNoConversionDataRequired(em, {}).IsCalc());

    // Symbols are resolved from the table.
    SymbolTable symbols {{ValueId::R, UnitType::Number, 128.0}};
    auto symbol = Number<>::Calc {calc::Value::Create(
      Category::Number, All,
      calc::Tree {.Root = calc::CreateChild(calc::Symbol {.Id = ValueId::R, .Unit = UnitType::Number}),
                  .Type = calc::Type::CreateNumber(),
                  .Stage = calc::Stage::Specified})};
    CHECK(EvaluateCalcIfNoConversionDataRequired(Number<> {symbol}, symbols) == Number<> {128.0});
  }

  TEST_CASE("PrimitiveNumericOrKeyword", "[CSS][Primitives][PrimitiveNumeric]")
  {
    using LengthOrAuto = PrimitiveNumericOrKeyword<Length<>, keywords::Auto>;

    LengthOrAuto automatic {keywords::Auto {}};
    CHECK(automatic.IsKeyword<keywords::Auto>());
    CHECK(automatic.HoldsAlternative<keywords::Auto>());
    CHECK_FALSE(automatic.HoldsAlternative<Length<>>());
    CHECK(automatic == keywords::Auto {});
    CHECK_FALSE(automatic.IsRaw());
    CHECK(SerialiseForTest(automatic) == u8"auto");

    LengthOrAuto length {2_css_px};
    CHECK(length.IsRaw());
    CHECK(length.HoldsAlternative<Length<>>());
    CHECK(length == 2_css_px);
    CHECK(SerialiseForTest(length) == u8"2px");
    CHECK(length.SwitchOn([](const LengthRaw<> &raw) { return raw.Value; }, [](const auto &) { return -1.0; })
          == 2.0);

    // From the numeric type.
    Length<> numeric {7_css_em};
    LengthOrAuto fromNumeric {numeric};
    CHECK(fromNumeric == 7_css_em);
    fromNumeric = keywords::Auto {};
    CHECK(fromNumeric == keywords::Auto {});
    fromNumeric = numeric;
    CHECK(fromNumeric == 7_css_em);

    LengthOrAuto fromVariant {Variant<Length<>, keywords::Auto> {keywords::Auto {}}};
    CHECK(fromVariant == automatic);

    // Calc values are shared, not copied.
    auto value = CreateEmCalc();
    {
      LengthOrAuto calc {Length<>::Calc {*value}};
      CHECK(calc.IsCalc());
      LengthOrAuto copy = calc;
      CHECK(value->GetRefCount() == 3u);
      CHECK(SerialiseForTest(copy) == u8"calc(5em)");
    }
    CHECK(value->HasOneRef());

    CHECK(MarkableTraits<LengthOrAuto>::IsEmptyValue(MarkableTraits<LengthOrAuto>::EmptyValue()));
    CHECK_FALSE(MarkableTraits<LengthOrAuto>::IsEmptyValue(length));
  }

  TEST_CASE("NumberOrPercentage", "[CSS][Primitives][PrimitiveNumeric]")
  {
    NumberOrPercentage<> number {NumberRaw<> {0.5}};
    NumberOrPercentage<> percentage {PercentageRaw<> {50.0}};
    CHECK(number.SwitchOn([](const Number<> &) { return 1; }, [](const Percentage<> &) { return 2; }) == 1);
    CHECK(percentage.SwitchOn([](const Number<> &) { return 1; }, [](const Percentage<> &) { return 2; })
          == 2);
    CHECK_FALSE(number == percentage);
    CHECK(number == NumberOrPercentage<> {Number<> {0.5}});
    CHECK(SerialiseForTest(percentage) == u8"50%");

    NumberOrPercentage<> fromVariant {Variant<Number<>, Percentage<>> {Percentage<> {25.0}}};
    CHECK(SerialiseForTest(fromVariant) == u8"25%");

    using Traits = MarkableTraits<NumberOrPercentage<>>;
    CHECK(Traits::IsEmptyValue(Traits::EmptyValue()));
    CHECK_FALSE(Traits::IsEmptyValue(number));

    // Percentages resolve to numbers when serialised.
    CHECK(SerialiseForTest(NumberOrPercentageResolvedToNumber<> {PercentageRaw<> {50.0}}) == u8"0.5");
    CHECK(SerialiseForTest(NumberOrPercentageResolvedToNumber<> {NumberRaw<> {2.0}}) == u8"2");

    using ResolvedTraits = MarkableTraits<NumberOrPercentageResolvedToNumber<>>;
    CHECK(ResolvedTraits::IsEmptyValue(ResolvedTraits::EmptyValue()));
  }
}
