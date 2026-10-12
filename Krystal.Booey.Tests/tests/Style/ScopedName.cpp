#include "Krystal.Booey/Style/ScopedName.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::style::tests
{
  TEST_CASE("ScopeOrdinal", "[Style][ScopeOrdinal]")
  {
    auto ordinal = ScopeOrdinal::Element;
    ++ordinal;
    CHECK(ordinal == ScopeOrdinal::FirstSlot);
    --ordinal;
    --ordinal;
    CHECK(ordinal == ScopeOrdinal::ContainingHost);
    STATIC_CHECK(ScopeOrdinal::ContainingHostLimit < ScopeOrdinal::ContainingHost);
    STATIC_CHECK(ScopeOrdinal::SlotLimit < ScopeOrdinal::Shadow);
  }

  TEST_CASE("ScopedName", "[Style][ScopedName]")
  {
    auto name = ScopedName {.Name = css::CSSOMStringAtom {u8"--a"}};
    CHECK(name.ScopeOrdinal == ScopeOrdinal::Element);
    CHECK(name.IsIdentifier);
    CHECK(name == ScopedName {.Name = css::CSSOMStringAtom {u8"--a"}});
    CHECK_FALSE(name
                == ScopedName {.Name = css::CSSOMStringAtom {u8"--a"}, .ScopeOrdinal = ScopeOrdinal::Shadow});
  }
}
