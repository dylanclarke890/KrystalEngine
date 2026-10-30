#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/AllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("SymbolTable", "[CSS][Calc][SymbolTable]")
  {
    SymbolTable empty;
    CHECK_FALSE(empty.Contains(ValueId::R));
    CHECK_FALSE(empty.Get(ValueId::R));

    SymbolTable table {
      {ValueId::R, UnitType::Number, 255.0},
      {ValueId::Alpha, UnitType::Percentage, 50.0},
    };
    CHECK(table.Contains(ValueId::R));
    CHECK_FALSE(table.Contains(ValueId::G));

    auto alpha = table.Get(ValueId::Alpha);
    REQUIRE(alpha);
    CHECK(alpha->Unit == UnitType::Percentage);
    CHECK(alpha->Value == 50.0);
  }

  TEST_CASE("AllowedSymbols", "[CSS][Calc][SymbolTable]")
  {
    AllowedSymbols empty;
    CHECK_FALSE(empty.Contains(ValueId::R));

    AllowedSymbols allowed {{
      {ValueId::R, UnitType::Number},
      {ValueId::G, UnitType::Number},
      {ValueId::B, UnitType::Number},
      {ValueId::Alpha, UnitType::Percentage},
    }};
    CHECK(allowed.Contains(ValueId::B));
    CHECK_FALSE(allowed.Contains(ValueId::Pi));
    CHECK(allowed.Get(ValueId::Alpha) == UnitType::Percentage);
    CHECK_FALSE(allowed.Get(ValueId::Infinity));

    AllowedSymbols copy = allowed;
    CHECK(copy.Get(ValueId::R) == UnitType::Number);
  }
}
