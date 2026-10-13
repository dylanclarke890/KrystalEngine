#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    CSSURL CursorURL(utf8_stringview url)
    {
      return *CompleteURL(CSSOMString {url},
                          ParserContext {ParserMode::HTMLStandard, url::URL {u8"https://example.com/"}});
    }

    RefPtr<Value> HotSpot(double x, double y)
    {
      return ValuePair::Create(PrimitiveValue::Create(x, UnitType::Number),
                               PrimitiveValue::Create(y, UnitType::Number));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue::Create", "[CSS][Values][CursorImageValue]")
  {
    SECTION("The original URL is the image's URL")
    {
      auto value = CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), nullptr);
      CHECK(value->IsCursorImageValue());
      // Cursor images aren't <image>s.
      CHECK_FALSE(value->IsImage());
      CHECK_FALSE(value->IsImageGeneratorValue());
      CHECK(value->OriginalURL() == CursorURL(u8"a.cur"));
    }

    SECTION("Values other than images have no original URL")
    {
      auto value = CursorImageValue::Create(NamedImageValue::Create(u8"name"), nullptr);
      CHECK(value->OriginalURL().IsNone());
    }

    SECTION("With an explicit original URL")
    {
      auto value =
        CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), nullptr, CursorURL(u8"b.cur"));
      CHECK(value->OriginalURL() == CursorURL(u8"b.cur"));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue serialisation", "[CSS][Values][CursorImageValue]")
  {
    CHECK(CssText(*CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), nullptr))
          == "url(\"a.cur\")");
    CHECK(CssText(*CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), HotSpot(1.0, 2.0)))
          == "url(\"a.cur\") 1 2");

    // Identical hot spot coordinates aren't coalesced.
    CHECK(CssText(*CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), HotSpot(3.0, 3.0)))
          == "url(\"a.cur\") 3 3");
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue equality", "[CSS][Values][CursorImageValue]")
  {
    auto make = [](utf8_stringview url, RefPtr<Value> hotSpot)
    {
      return CursorImageValue::Create(ImageValue::Create(CursorURL(url)), krys::move(hotSpot));
    };

    auto a = make(u8"a.cur", HotSpot(1.0, 2.0));
    CHECK(*a == *make(u8"a.cur", HotSpot(1.0, 2.0)));
    CHECK_FALSE(*a == *make(u8"b.cur", HotSpot(1.0, 2.0)));
    CHECK_FALSE(*a == *make(u8"a.cur", HotSpot(2.0, 1.0)));
    CHECK_FALSE(*a == *make(u8"a.cur", nullptr));
    CHECK(*make(u8"a.cur", nullptr) == *make(u8"a.cur", nullptr));

    // The original URL isn't compared.
    CHECK(
      *make(u8"a.cur", nullptr)
      == *CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), nullptr, CursorURL(u8"b.cur")));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue children", "[CSS][Values][CursorImageValue]")
  {
    // Only the image is visited, not the hot spot.
    auto value = CursorImageValue::Create(ImageValue::Create(CursorURL(u8"a.cur")), HotSpot(1.0, 2.0));
    auto children = Children(*value);
    REQUIRE(children.size() == 1uz);
    CHECK(children[0uz]->IsImageValue());

    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
