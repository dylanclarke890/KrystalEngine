#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "ValuePair::Create", "[CSS][Values][ValuePair]")
  {
    auto pair =
      ValuePair::Create(PrimitiveValue::Create(1.0, UnitType::px), PrimitiveValue::Create(2.0, UnitType::px));
    CHECK(pair->IsValuePairValue());
    CHECK(CssText(pair->First()) == "1px");
    CHECK(CssText(pair->Second()) == "2px");
    CHECK(CssText(*pair) == "1px 2px");
    CHECK_FALSE(pair->CanBeCoalesced());

    SECTION("Value::First()/Second() forward to the pair")
    {
      const Value &value = *pair;
      CHECK(&value.First() == &pair->First());
      CHECK(&value.Second() == &pair->Second());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ValuePair serialisation", "[CSS][Values][ValuePair]")
  {
    SECTION("Identical values coalesce")
    {
      auto pair =
        ValuePair::Create(PrimitiveValue::Create(ValueId::Auto), PrimitiveValue::Create(ValueId::Auto));
      CHECK(pair->CanBeCoalesced());
      CHECK(CssText(*pair) == "auto");
    }

    SECTION("Non-coalescing pairs keep both values")
    {
      auto pair = ValuePair::CreateNonCoalescing(PrimitiveValue::Create(ValueId::Auto),
                                                 PrimitiveValue::Create(ValueId::Auto));
      CHECK_FALSE(pair->CanBeCoalesced());
      CHECK(CssText(*pair) == "auto auto");
    }

    SECTION("Slash-separated pairs don't coalesce")
    {
      auto pair = ValuePair::CreateSlashSeparated(PrimitiveValue::Create(1.0, UnitType::Number),
                                                  PrimitiveValue::Create(1.0, UnitType::Number));
      CHECK_FALSE(pair->CanBeCoalesced());
      CHECK(CssText(*pair) == "1 / 1");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ValuePair equality and hashing", "[CSS][Values][ValuePair]")
  {
    auto a =
      ValuePair::Create(PrimitiveValue::Create(1.0, UnitType::px), PrimitiveValue::Create(ValueId::Auto));
    auto b =
      ValuePair::Create(PrimitiveValue::Create(1.0, UnitType::px), PrimitiveValue::Create(ValueId::Auto));
    auto c =
      ValuePair::Create(PrimitiveValue::Create(2.0, UnitType::px), PrimitiveValue::Create(ValueId::Auto));
    auto slash = ValuePair::CreateSlashSeparated(PrimitiveValue::Create(1.0, UnitType::px),
                                                 PrimitiveValue::Create(ValueId::Auto));
    auto nonCoalescing = ValuePair::CreateNonCoalescing(PrimitiveValue::Create(1.0, UnitType::px),
                                                        PrimitiveValue::Create(ValueId::Auto));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *c);
    CHECK_FALSE(*a == *slash);
    CHECK_FALSE(*a == *nonCoalescing);

    size_t hashA = 0uz, hashB = 0uz, hashC = 0uz;
    CHECK(a->AddHash(hashA));
    CHECK(b->AddHash(hashB));
    CHECK(c->AddHash(hashC));
    CHECK(hashA == hashB);
    CHECK(hashA != hashC);
  }

  TEST_CASE_METHOD(ValueFixture, "ValuePair visits both values", "[CSS][Values][ValuePair]")
  {
    auto pair =
      ValuePair::Create(PrimitiveValue::Create(1.0, UnitType::px), PrimitiveValue::Create(ValueId::Auto));
    auto children = Children(*pair);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == &pair->First());
    CHECK(children[1uz] == &pair->Second());
  }
}
