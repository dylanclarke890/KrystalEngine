#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<Value> Option(utf8_stringview url, double resolution)
    {
      return ImageSetOptionValue::Create(
        ImageValue::Create(*CompleteURL(
          CSSOMString {url}, ParserContext {ParserMode::HTMLStandard, url::URL {u8"https://example.com/"}})),
        PrimitiveValue::Create(resolution, UnitType::x));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetValue::Create", "[CSS][Values][ImageSetValue]")
  {
    auto set = ImageSetValue::Create({Option(u8"a.png", 1.0), Option(u8"b.png", 2.0)});
    CHECK(set->IsImageSetValue());
    CHECK(set->IsImage());
    CHECK_FALSE(set->IsImageGeneratorValue());
    CHECK(set->ContainsValueList());
    CHECK(set->Length() == 2u);
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetValue serialisation", "[CSS][Values][ImageSetValue]")
  {
    // Options are written in the authored order, not sorted by resolution.
    CHECK(CssText(*ImageSetValue::Create({Option(u8"b.png", 2.0), Option(u8"a.png", 1.0)}))
          == "image-set(url(\"b.png\") 2x, url(\"a.png\") 1x)");
    CHECK(CssText(*ImageSetValue::Create({Option(u8"a.png", 1.0)})) == "image-set(url(\"a.png\") 1x)");
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetValue equality", "[CSS][Values][ImageSetValue]")
  {
    auto a = ImageSetValue::Create({Option(u8"a.png", 1.0), Option(u8"b.png", 2.0)});
    CHECK(*a == *ImageSetValue::Create({Option(u8"a.png", 1.0), Option(u8"b.png", 2.0)}));
    CHECK_FALSE(*a == *ImageSetValue::Create({Option(u8"b.png", 2.0), Option(u8"a.png", 1.0)}));
    CHECK_FALSE(*a == *ImageSetValue::Create({Option(u8"a.png", 1.0)}));
    CHECK_FALSE(*a == *ImageSetValue::Create({Option(u8"a.png", 1.0), Option(u8"b.png", 3.0)}));

    // Options aren't hashable, so neither is the set.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetValue children and dependencies", "[CSS][Values][ImageSetValue]")
  {
    auto set = ImageSetValue::Create({Option(u8"a.png", 1.0), Option(u8"b.png", 2.0)});
    auto children = Children(*set);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == set->Item(0u));
    CHECK(children[1uz] == set->Item(1u));

    CHECK(set->ComputedStyleDependencies().IsComputationallyIndependent());
    // Only the values themselves are asked, not their children.
    CHECK_FALSE(set->MayDependOnBaseUrl());
    CHECK_FALSE(set->TraverseSubresources([](const CachedResource &) { return true; }));
  }
}
