#include "Krystal.Booey/CSS/Values/SubgridValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<Value> Names(List<CSSOMString> names)
    {
      return GridLineNamesValue::Create(names);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "SubgridValue::Create", "[CSS][Values][SubgridValue]")
  {
    auto value = SubgridValue::Create({Names({u8"a"}), Names({u8"b", u8"c"})});
    CHECK(value->IsSubgridValue());
    CHECK(value->ContainsValueList());
    REQUIRE(value->Length() == 2u);
    CHECK(CssText(*value->Item(0u)) == "[a]");
    CHECK(CssText(*value->Item(1u)) == "[b c]");
  }

  TEST_CASE_METHOD(ValueFixture, "SubgridValue serialisation", "[CSS][Values][SubgridValue]")
  {
    CHECK(CssText(*SubgridValue::Create({})) == "subgrid");
    CHECK(CssText(*SubgridValue::Create({Names({u8"a"})})) == "subgrid [a]");
    CHECK(CssText(*SubgridValue::Create({Names({u8"a"}), Names({u8"b", u8"c"})})) == "subgrid [a] [b c]");
  }

  TEST_CASE_METHOD(ValueFixture, "SubgridValue equality and hashing", "[CSS][Values][SubgridValue]")
  {
    auto a = SubgridValue::Create({Names({u8"a"}), Names({u8"b"})});
    auto b = SubgridValue::Create({Names({u8"a"}), Names({u8"b"})});
    auto reordered = SubgridValue::Create({Names({u8"b"}), Names({u8"a"})});
    auto shorter = SubgridValue::Create({Names({u8"a"})});
    auto transformList = TransformListValue::Create(ValueListBuilder {Names({u8"a"}), Names({u8"b"})});

    CHECK(*a == *b);
    CHECK(*SubgridValue::Create({}) == *SubgridValue::Create({}));
    CHECK_FALSE(*a == *reordered);
    CHECK_FALSE(*a == *shorter);
    CHECK_FALSE(*a == *transformList);

    // `GridLineNamesValue` has no hash of its own, so lists containing it aren't hashable.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));

    size_t emptyA = 0uz, emptyB = 0uz;
    CHECK(SubgridValue::Create({})->AddHash(emptyA));
    CHECK(SubgridValue::Create({})->AddHash(emptyB));
    CHECK(emptyA == emptyB);
  }

  TEST_CASE_METHOD(ValueFixture, "SubgridValue visits its items", "[CSS][Values][SubgridValue]")
  {
    auto value = SubgridValue::Create({Names({u8"a"}), Names({u8"b"})});
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == value->Item(0u));
    CHECK(children[1uz] == value->Item(1u));
  }
}
