#include "Krystal.Booey/Css/Values/ScrollValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "ScrollValue::Create", "[CSS][Values][ScrollValue]")
  {
    SECTION("Empty")
    {
      auto value = ScrollValue::Create();
      CHECK(Is<ScrollValue>(*value));
      CHECK_FALSE(value->Scroller());
      CHECK_FALSE(value->Axis());
    }

    SECTION("With a scroller and an axis")
    {
      auto value = ScrollValue::Create(IdentValue(ValueId::Root), IdentValue(ValueId::Inline));
      REQUIRE(value->Scroller());
      REQUIRE(value->Axis());
      CHECK(value->Scroller()->ValueId() == ValueId::Root);
      CHECK(value->Axis()->ValueId() == ValueId::Inline);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ScrollValue serialisation", "[CSS][Values][ScrollValue]")
  {
    CHECK(CssText(*ScrollValue::Create()) == "scroll()");
    CHECK(CssText(*ScrollValue::Create(IdentValue(ValueId::Root), nullptr)) == "scroll(root)");
    CHECK(CssText(*ScrollValue::Create(nullptr, IdentValue(ValueId::Inline))) == "scroll(inline)");
    CHECK(CssText(*ScrollValue::Create(IdentValue(ValueId::Self), IdentValue(ValueId::X)))
          == "scroll(self x)");

    // `nearest` and `block` are the defaults, so they're omitted.
    CHECK(CssText(*ScrollValue::Create(IdentValue(ValueId::Nearest), IdentValue(ValueId::Block)))
          == "scroll()");
    CHECK(CssText(*ScrollValue::Create(IdentValue(ValueId::Nearest), IdentValue(ValueId::Y))) == "scroll(y)");
    CHECK(CssText(*ScrollValue::Create(IdentValue(ValueId::Root), IdentValue(ValueId::Block)))
          == "scroll(root)");
  }

  TEST_CASE_METHOD(ValueFixture, "ScrollValue equality and hashing", "[CSS][Values][ScrollValue]")
  {
    auto a = ScrollValue::Create(IdentValue(ValueId::Root), IdentValue(ValueId::Inline));
    auto b = ScrollValue::Create(IdentValue(ValueId::Root), IdentValue(ValueId::Inline));

    CHECK(*a == *b);
    CHECK(*ScrollValue::Create() == *ScrollValue::Create());
    CHECK_FALSE(*a == *ScrollValue::Create(IdentValue(ValueId::Self), IdentValue(ValueId::Inline)));
    CHECK_FALSE(*a == *ScrollValue::Create(IdentValue(ValueId::Root), IdentValue(ValueId::X)));
    CHECK_FALSE(*a == *ScrollValue::Create(IdentValue(ValueId::Root), nullptr));
    // Equality compares the values, not the serialisation: an explicit default is a different value.
    CHECK_FALSE(*ScrollValue::Create() == *ScrollValue::Create(IdentValue(ValueId::Nearest), nullptr));

    // `ScrollValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ScrollValue visitation", "[CSS][Values][ScrollValue]")
  {
    CHECK(Children(*ScrollValue::Create()).empty());

    auto value = ScrollValue::Create(IdentValue(ValueId::Root), IdentValue(ValueId::Inline));
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == value->Scroller().get());
    CHECK(children[1uz] == value->Axis().get());

    auto axisOnly = ScrollValue::Create(nullptr, IdentValue(ValueId::Inline));
    auto axisChildren = Children(*axisOnly);
    REQUIRE(axisChildren.size() == 1uz);
    CHECK(axisChildren[0uz] == axisOnly->Axis().get());
  }
}
