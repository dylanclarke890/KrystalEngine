#include "Krystal.Web/Css/Values/ImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    Url ImageUrl(utf8_stringview url)
    {
      return *CompleteUrl(CSSOMString {url},
                          ParserContext {ParserMode::HTMLStandard, net::Url {u8"https://example.com/"}});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue::Create", "[CSS][Values][ImageValue]")
  {
    SECTION("From a CSS URL")
    {
      auto url = ImageUrl(u8"images/a.png");
      auto value = ImageValue::Create(url);
      CHECK(Is<ImageValue>(*value));
      CHECK(value->IsImage());
      CHECK_FALSE(value->IsImageGeneratorValue());
      CHECK(value->Url() == url);
      CHECK(CssText(*value) == "url(\"images/a.png\")");
    }

    SECTION("From a resolved URL")
    {
      auto value = ImageValue::Create(net::Url {u8"https://example.com/a.png"});
      CHECK(ToNarrow(value->Url().Specified) == "https://example.com/a.png");
      CHECK(ToNarrow(value->Url().Resolved.String()) == "https://example.com/a.png");
      CHECK(CssText(*value) == "url(\"https://example.com/a.png\")");
    }

    SECTION("Invalid images serialise as an empty string")
    {
      auto value = ImageValue::CreateInvalid();
      CHECK(Is<ImageValue>(*value));
      CHECK(CssText(*value).empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue equality", "[CSS][Values][ImageValue]")
  {
    auto a = ImageValue::Create(ImageUrl(u8"a.png"));
    CHECK(*a == *ImageValue::Create(ImageUrl(u8"a.png")));
    CHECK_FALSE(*a == *ImageValue::Create(ImageUrl(u8"b.png")));

    // The initiator type isn't compared.
    CHECK(*a == *ImageValue::Create(ImageUrl(u8"a.png"), u8"css"));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue::CopyForComputedStyle", "[CSS][Values][ImageValue]")
  {
    auto value = ImageValue::Create(ImageUrl(u8"a.png"));
    CHECK(Children(*value).empty());

    SECTION("The same URL shares the value")
    {
      auto copy = value->CopyForComputedStyle(ImageUrl(u8"a.png"));
      CHECK(copy.get() == value.get());
    }

    SECTION("A different URL creates a value that visits the unresolved one")
    {
      auto resolved = ImageUrl(u8"b.png");
      auto copy = value->CopyForComputedStyle(resolved);
      CHECK(copy.get() != value.get());
      CHECK(copy->Url() == resolved);
      CHECK_FALSE(*copy == *value);

      auto children = Children(*copy);
      REQUIRE(children.size() == 1uz);
      CHECK(children[0uz] == value.get());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue base URL and subresources", "[CSS][Values][ImageValue]")
  {
    CHECK(ImageValue::Create(ImageUrl(u8"a.png"))->MayDependOnBaseUrl());
    CHECK_FALSE(ImageValue::Create(ImageUrl(u8"#image"))->MayDependOnBaseUrl());
    CHECK_FALSE(ImageValue::Create(ImageUrl(u8"data:image/png;base64,AAAA"))->MayDependOnBaseUrl());
    CHECK_FALSE(ImageValue::CreateInvalid()->MayDependOnBaseUrl());

    // No image is loaded, so there are no subresources.
    bool called = false;
    CHECK_FALSE(ImageValue::Create(ImageUrl(u8"a.png"))
                  ->TraverseSubresources(
                    [&](const CachedResource &)
                    {
                      called = true;
                      return true;
                    }));
    CHECK_FALSE(called);
  }
}
