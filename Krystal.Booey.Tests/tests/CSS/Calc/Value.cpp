#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"

namespace krys::boo::css::calc::tests
{
  namespace
  {
    Child Num(double value, UnitType unit = UnitType::Number)
    {
      return CreateNumeric(value, unit);
    }

    Ref<Value> Create(css::Category category, css::Range range, Child &&root, Type type,
                      bool requiresConversionData = false)
    {
      return Value::Create(category, range,
                           Tree {.Root = krys::move(root),
                                 .Type = type,
                                 .Stage = Stage::Specified,
                                 .RequiresConversionData = requiresConversionData});
    }
  }

  TEST_CASE("Value(DoubleValue)", "[CSS][Calc][Value]")
  {
    auto length =
      Create(css::Category::Length, All,
             CreateChild(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px)), Type::CreateLength()),
             Type::CreateLength());
    CHECK(length->DoubleValue(NoConversionDataRequiredTag {}) == 100.0);
    CHECK(length->DoubleValueDeprecated() == 100.0);
    CHECK(length->DoubleValue(ToLengthConversionData {}) == 100.0);

    // Results are clamped to the value's range, and failures evaluate to 0.
    auto negative = Create(css::Category::Length, NonNegative, Num(-5.0, UnitType::px), Type::CreateLength());
    CHECK(negative->DoubleValue(NoConversionDataRequiredTag {}) == 0.0);

    auto em = Create(css::Category::Length, All, Num(2.0, UnitType::em), Type::CreateLength(), true);
    CHECK(em->DoubleValue(NoConversionDataRequiredTag {}) == 0.0);

    SymbolTable symbols {{ValueId::Alpha, UnitType::Number, 0.25}};
    auto symbol =
      Create(css::Category::Number, All, CreateChild(Symbol {.Id = ValueId::Alpha, .Unit = UnitType::Number}),
             Type::CreateNumber());
    CHECK(symbol->DoubleValue(NoConversionDataRequiredTag {}, symbols) == 0.25);
  }

  TEST_CASE("Value(ComputeLengthPx)", "[CSS][Calc][Value]")
  {
    auto length = Create(css::Category::Length, All, Num(10.0, UnitType::px), Type::CreateLength());
    CHECK(length->ComputeLengthPx(ToLengthConversionData {}) == 10.0);

    // Absolute lengths are scaled by the conversion data's zoom.
    CHECK(length->ComputeLengthPx(ToLengthConversionData {}.CopyWithAdjustedZoom(2.0f)) == 20.0);
  }

  TEST_CASE("Value(CopySimplified)", "[CSS][Calc][Value]")
  {
    auto length =
      Create(css::Category::Length, All,
             CreateChild(Add(Num(1.0, UnitType::in), Num(4.0, UnitType::px)), Type::CreateLength()),
             Type::CreateLength());
    auto simplified = length->CopySimplified(NoConversionDataRequiredTag {});
    CHECK(simplified.get() != length.get());
    CHECK(simplified->CssText(DefaultSerialisationContext()) == u8"calc(100px)");

    // A value that is already simplified is shared rather than copied.
    auto again = simplified->CopySimplified(NoConversionDataRequiredTag {});
    CHECK(again.get() == simplified.get());

    // Symbols are substituted during simplification.
    SymbolTable symbols {{ValueId::Alpha, UnitType::Number, 0.25}};
    auto symbol =
      Create(css::Category::Number, All, CreateChild(Symbol {.Id = ValueId::Alpha, .Unit = UnitType::Number}),
             Type::CreateNumber());
    CHECK(
      symbol->CopySimplified(NoConversionDataRequiredTag {}, symbols)->CssText(DefaultSerialisationContext())
      == u8"calc(0.25)");
  }

  TEST_CASE("Value(CreateCalculationValue)", "[CSS][Calc][Value]")
  {
    // calc(10deg + 50%) as an <angle-percentage>: the percentage stays unresolved in the style tree.
    auto angle =
      Create(css::Category::AnglePercentage, All,
             CreateChild(Add(Num(10.0, UnitType::deg), Num(50.0, UnitType::Percentage)), Type::CreateAngle()),
             Type::CreateAngle());
    auto calculation = angle->CreateCalculationValue(NoConversionDataRequiredTag {});
    CHECK(calculation->GetCategory() == css::Category::AnglePercentage);
    CHECK(calculation->Evaluate(360.0, style::ZoomNeeded {}) == 190.0);
    CHECK(calculation->Evaluate(0.0, style::ZoomNeeded {}) == 10.0);
  }
}
