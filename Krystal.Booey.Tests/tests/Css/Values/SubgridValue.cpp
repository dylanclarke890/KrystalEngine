#include "Krystal.Booey/Css/Values/SubgridValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/Css/Values/TransformListValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "SubgridValue::Create", "[CSS][Values][SubgridValue]")
  {
    auto value = SubgridValue::Create({LineNames({u8"a"}), LineNames({u8"b", u8"c"})});
    CHECK(Is<SubgridValue>(*value));
    CHECK(value->ContainsValueList());
    REQUIRE(value->size() == 2u);
    CHECK(CssText((*value)[0u]) == "[a]");
    CHECK(CssText((*value)[1u]) == "[b c]");
  }

  TEST_CASE_METHOD(ValueFixture, "SubgridValue serialisation", "[CSS][Values][SubgridValue]")
  {
    CHECK(CssText(*SubgridValue::Create({})) == "subgrid");
    CHECK(CssText(*SubgridValue::Create({LineNames({u8"a"})})) == "subgrid [a]");
    CHECK(CssText(*SubgridValue::Create({LineNames({u8"a"}), LineNames({u8"b", u8"c"})}))
          == "subgrid [a] [b c]");
  }

  TEST_CASE_METHOD(ValueFixture, "SubgridValue equality and hashing", "[CSS][Values][SubgridValue]")
  {
    auto a = SubgridValue::Create({LineNames({u8"a"}), LineNames({u8"b"})});
    auto b = SubgridValue::Create({LineNames({u8"a"}), LineNames({u8"b"})});
    auto reordered = SubgridValue::Create({LineNames({u8"b"}), LineNames({u8"a"})});
    auto shorter = SubgridValue::Create({LineNames({u8"a"})});
    auto transformList =
      TransformListValue::Create(ValueListBuilder {LineNames({u8"a"}), LineNames({u8"b"})});

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
    auto value = SubgridValue::Create({LineNames({u8"a"}), LineNames({u8"b"})});
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == &(*value)[0u]);
    CHECK(children[1uz] == &(*value)[1u]);
  }
}
