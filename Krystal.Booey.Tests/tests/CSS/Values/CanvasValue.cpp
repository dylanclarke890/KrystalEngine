#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "CanvasValue::Create", "[CSS][Values][CanvasValue]")
  {
    auto value = CanvasValue::Create(u8"name");
    CHECK(value->IsCanvasValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
    CHECK_FALSE(value->IsImageValue());
  }

  TEST_CASE_METHOD(ValueFixture, "CanvasValue serialisation", "[CSS][Values][CanvasValue]")
  {
    // The name is written as given.
    CHECK(CssText(*CanvasValue::Create(u8"name")) == "-webkit-canvas(name)");
    CHECK(CssText(*CanvasValue::Create(u8"")) == "-webkit-canvas()");
  }

  TEST_CASE_METHOD(ValueFixture, "CanvasValue equality, hashing and children", "[CSS][Values][CanvasValue]")
  {
    auto a = CanvasValue::Create(u8"a");
    CHECK(*a == *CanvasValue::Create(u8"a"));
    CHECK_FALSE(*a == *CanvasValue::Create(u8"b"));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));

    CHECK(Children(*a).empty());
    CHECK_FALSE(a->MayDependOnBaseUrl());
    CHECK(a->ComputedStyleDependencies().IsComputationallyIndependent());
  }
}
