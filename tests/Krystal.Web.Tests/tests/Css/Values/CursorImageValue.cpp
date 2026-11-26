#include "Krystal.Web/Css/Values/CursorImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Values/CrossFadeValue.hpp"
#include "Krystal.Web/Css/Values/ImageValue.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/ValuePair.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    Url CursorUrl(utf8_stringview url)
    {
      return *CompleteUrl(CSSOMString {url},
                          ParserContext {ParserMode::HTMLStandard, net::Url {u8"https://example.com/"}});
    }

    RefPtr<ValuePair> HotSpot(double x, double y)
    {
      return ValuePair::Create(PrimitiveValue::Create(x, UnitType::Number),
                               PrimitiveValue::Create(y, UnitType::Number));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue::Create", "[CSS][Values][CursorImageValue]")
  {
    auto value = CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), nullptr);
    CHECK(Is<CursorImageValue>(*value));
    // Cursor images aren't <image>s.
    CHECK_FALSE(value->IsImage());
    CHECK_FALSE(value->IsImageGeneratorValue());

    // The original URL is only observable through the style image; see the style Cursor test.
    CHECK(Is<CursorImageValue>(
      *CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), nullptr, CursorUrl(u8"b.cur"))));
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue serialisation", "[CSS][Values][CursorImageValue]")
  {
    CHECK(CssText(*CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), nullptr))
          == "url(\"a.cur\")");
    CHECK(CssText(*CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), HotSpot(1.0, 2.0)))
          == "url(\"a.cur\") 1 2");

    // Identical hot spot coordinates aren't coalesced.
    CHECK(CssText(*CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), HotSpot(3.0, 3.0)))
          == "url(\"a.cur\") 3 3");
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue equality", "[CSS][Values][CursorImageValue]")
  {
    auto make = [](utf8_stringview url, RefPtr<ValuePair> hotSpot)
    {
      return CursorImageValue::Create(ImageValue::Create(CursorUrl(url)), krys::move(hotSpot));
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
      == *CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), nullptr, CursorUrl(u8"b.cur")));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "CursorImageValue children", "[CSS][Values][CursorImageValue]")
  {
    // The image, then the hot spot.
    auto value = CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), HotSpot(1.0, 2.0));
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(Is<ImageValue>(*children[0uz]));
    CHECK(Is<ValuePair>(*children[1uz]));

    CHECK(Children(*CursorImageValue::Create(ImageValue::Create(CursorUrl(u8"a.cur")), nullptr)).size()
          == 1uz);

    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
