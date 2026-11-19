#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::style::tests
{
  TEST_CASE_METHOD(BuilderFixture, "Style URL", "[Style][Values][URL]")
  {
    SECTION("An unresolved URL is completed against the document")
    {
      Document->SetUrl(net::Url {u8"https://example.com/dir/page.html"});
      auto url = ConvertCSSValueToStyle<Url>(
        *State,
        *css::UrlValue::Create(css::CSSUrl {.Specified = u8"image.png", .Resolved = {}, .Modifiers = {}}));
      CHECK(url.Resolved.String() == u8"https://example.com/dir/image.png");
      CHECK_FALSE(IsInvalid());
    }

    SECTION("A resolved URL is kept")
    {
      auto resolved = net::Url {u8"https://example.org/sheet/image.png"};
      auto url = ConvertCSSValueToStyle<Url>(
        *State, *css::UrlValue::Create(
                  css::CSSUrl {.Specified = u8"image.png", .Resolved = resolved, .Modifiers = {}}));
      CHECK(url.Resolved == resolved);
    }

    SECTION("A non-URL value is invalid at computed-value time")
    {
      auto url = ConvertCSSValueToStyle<Url>(*State, *css::PrimitiveValue::Create(css::ValueId::None));
      CHECK(IsInvalid());
      CHECK_FALSE(url.Resolved.IsNull());
      CHECK(url.Resolved.String().empty());
    }

    SECTION("Creation and serialisation")
    {
      Url url {.Resolved = net::Url {u8"https://example.com/a.png"}, .Modifiers = {}};
      CHECK(Created(url) == "url(\"https://example.com/a.png\")");
      CHECK(Serialised(url) == "url(\"https://example.com/a.png\")");
    }

    SECTION("Markable")
    {
      Markable<Url> url;
      CHECK_FALSE(url);
      url = Url {.Resolved = net::Url {u8"https://example.com/"}, .Modifiers = {}};
      CHECK(url);
    }
  }
}
