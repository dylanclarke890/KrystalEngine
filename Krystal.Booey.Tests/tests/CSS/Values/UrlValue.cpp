#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD CSSURL UrlFor(const CSSOMString &specified) noexcept
    {
      return *CompleteURL(specified,
                          ParserContext {ParserMode::HTMLStandard, url::URL {u8"https://example.com/"}});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "UrlValue::Create", "[CSS][Values][UrlValue]")
  {
    auto url = UrlFor(u8"images/a.png");
    auto value = UrlValue::Create(url);
    CHECK(value->IsUrlValue());
    CHECK(value->URL() == url);
    CHECK(ToNarrow(value->StringValue()) == "images/a.png");
    CHECK(ToNarrow(value->URL().Resolved.String()) == "https://example.com/images/a.png");
  }

  TEST_CASE_METHOD(ValueFixture, "UrlValue serialisation", "[CSS][Values][UrlValue]")
  {
    // The specified URL is written, not the resolved one.
    CHECK(CssText(*UrlValue::Create(UrlFor(u8"images/a.png"))) == "url(\"images/a.png\")");

    auto url = UrlFor(u8"a.png");
    url.Modifiers.ReferrerPolicy = URLReferrerPolicyFunction {.Parameters = {keywords::NoReferrer {}}};
    CHECK(CssText(*UrlValue::Create(url)) == "url(\"a.png\" referrer-policy(no-referrer))");
  }

  TEST_CASE_METHOD(ValueFixture, "UrlValue equality", "[CSS][Values][UrlValue]")
  {
    auto a = UrlValue::Create(UrlFor(u8"a.png"));
    CHECK(*a == *UrlValue::Create(UrlFor(u8"a.png")));
    CHECK_FALSE(*a == *UrlValue::Create(UrlFor(u8"b.png")));

    auto withModifier = UrlFor(u8"a.png");
    withModifier.Modifiers.ReferrerPolicy = URLReferrerPolicyFunction {.Parameters = {keywords::Origin {}}};
    CHECK_FALSE(*a == *UrlValue::Create(withModifier));
  }

  TEST_CASE_METHOD(ValueFixture, "UrlValue has no children", "[CSS][Values][UrlValue]")
  {
    CHECK(Children(*UrlValue::Create(UrlFor(u8"a.png"))).empty());
  }
}
