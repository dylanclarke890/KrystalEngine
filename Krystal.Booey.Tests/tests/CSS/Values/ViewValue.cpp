#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    RefPtr<Value> Keyword(ValueId id)
    {
      return PrimitiveValue::Create(id);
    }

    RefPtr<Value> Px(double value)
    {
      return PrimitiveValue::Create(value, UnitType::px);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ViewValue::Create", "[CSS][Values][ViewValue]")
  {
    SECTION("Empty")
    {
      auto value = ViewValue::Create();
      CHECK(value->IsViewValue());
      CHECK_FALSE(value->Axis());
      CHECK_FALSE(value->StartInset());
      CHECK_FALSE(value->EndInset());
    }

    SECTION("With an axis and insets")
    {
      auto value = ViewValue::Create(Keyword(ValueId::X), Px(1.0), Px(2.0));
      REQUIRE(value->Axis());
      REQUIRE(value->StartInset());
      REQUIRE(value->EndInset());
      CHECK(value->Axis()->ValueId() == ValueId::X);
      CHECK(CssText(*value->StartInset()) == "1px");
      CHECK(CssText(*value->EndInset()) == "2px");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ViewValue serialisation", "[CSS][Values][ViewValue]")
  {
    CHECK(CssText(*ViewValue::Create()) == "view()");
    CHECK(CssText(*ViewValue::Create(Keyword(ValueId::Inline), nullptr, nullptr)) == "view(inline)");
    CHECK(CssText(*ViewValue::Create(Keyword(ValueId::X), Px(1.0), Px(2.0))) == "view(x 1px 2px)");
    CHECK(CssText(*ViewValue::Create(nullptr, Px(1.0), nullptr)) == "view(1px)");
    CHECK(CssText(*ViewValue::Create(nullptr, Px(1.0), Px(2.0))) == "view(1px 2px)");

    // `block` is the default axis, so it's omitted.
    CHECK(CssText(*ViewValue::Create(Keyword(ValueId::Block), nullptr, nullptr)) == "view()");
    CHECK(CssText(*ViewValue::Create(Keyword(ValueId::Block), Px(1.0), nullptr)) == "view(1px)");

    // A lone `auto` start inset is omitted, but kept when an end inset follows it.
    CHECK(CssText(*ViewValue::Create(nullptr, Keyword(ValueId::Auto), nullptr)) == "view()");
    CHECK(CssText(*ViewValue::Create(Keyword(ValueId::Y), Keyword(ValueId::Auto), nullptr)) == "view(y)");
    CHECK(CssText(*ViewValue::Create(nullptr, Keyword(ValueId::Auto), Px(2.0))) == "view(auto 2px)");

    // An end inset that's the same value object as the start inset is omitted.
    RefPtr<Value> inset = Px(3.0);
    CHECK(CssText(*ViewValue::Create(nullptr, RefPtr<Value> {inset}, RefPtr<Value> {inset})) == "view(3px)");
  }

  TEST_CASE_METHOD(ValueFixture, "ViewValue equality and hashing", "[CSS][Values][ViewValue]")
  {
    auto a = ViewValue::Create(Keyword(ValueId::X), Px(1.0), Px(2.0));
    auto b = ViewValue::Create(Keyword(ValueId::X), Px(1.0), Px(2.0));

    CHECK(*a == *b);
    CHECK(*ViewValue::Create() == *ViewValue::Create());
    CHECK_FALSE(*a == *ViewValue::Create(Keyword(ValueId::Y), Px(1.0), Px(2.0)));
    CHECK_FALSE(*a == *ViewValue::Create(Keyword(ValueId::X), Px(5.0), Px(2.0)));
    CHECK_FALSE(*a == *ViewValue::Create(Keyword(ValueId::X), Px(1.0), Px(5.0)));
    CHECK_FALSE(*a == *ViewValue::Create(Keyword(ValueId::X), Px(1.0), nullptr));

    // `ViewValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ViewValue visitation", "[CSS][Values][ViewValue]")
  {
    CHECK(Children(*ViewValue::Create()).empty());

    auto value = ViewValue::Create(Keyword(ValueId::X), Px(1.0), Px(2.0));
    auto children = Children(*value);
    REQUIRE(children.size() == 3uz);
    CHECK(children[0uz] == value->Axis().get());
    CHECK(children[1uz] == value->StartInset().get());
    CHECK(children[2uz] == value->EndInset().get());

    auto insetsOnly = ViewValue::Create(nullptr, Px(1.0), Px(2.0));
    CHECK(Children(*insetsOnly).size() == 2uz);
  }
}
