#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "NamedImageValue::Create", "[CSS][Values][NamedImageValue]")
  {
    auto value = NamedImageValue::Create(u8"name");
    CHECK(value->IsNamedImageValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
    CHECK_FALSE(value->IsImageValue());
  }

  TEST_CASE_METHOD(ValueFixture, "NamedImageValue serialisation", "[CSS][Values][NamedImageValue]")
  {
    // The name is written as given.
    CHECK(CssText(*NamedImageValue::Create(u8"name")) == "-webkit-named-image(name)");
    CHECK(CssText(*NamedImageValue::Create(u8"")) == "-webkit-named-image()");
  }

  TEST_CASE_METHOD(ValueFixture, "NamedImageValue equality, hashing and children",
                   "[CSS][Values][NamedImageValue]")
  {
    auto a = NamedImageValue::Create(u8"a");
    CHECK(*a == *NamedImageValue::Create(u8"a"));
    CHECK_FALSE(*a == *NamedImageValue::Create(u8"b"));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));

    CHECK(Children(*a).empty());
    CHECK_FALSE(a->MayDependOnBaseUrl());
    CHECK(a->ComputedStyleDependencies().IsComputationallyIndependent());
  }
}
