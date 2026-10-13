#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    CSSURL ImageURL(utf8_stringview url)
    {
      return *CompleteURL(CSSOMString {url},
                          ParserContext {ParserMode::HTMLStandard, url::URL {u8"https://example.com/"}});
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue::Create", "[CSS][Values][ImageValue]")
  {
    SECTION("From a CSS URL")
    {
      auto url = ImageURL(u8"images/a.png");
      auto value = ImageValue::Create(url);
      CHECK(value->IsImageValue());
      CHECK(value->IsImage());
      CHECK_FALSE(value->IsImageGeneratorValue());
      CHECK(value->URL() == url);
      CHECK_FALSE(value->IsLoadedFromOpaqueSource());
      CHECK(CssText(*value) == "url(\"images/a.png\")");
    }

    SECTION("From a resolved URL")
    {
      auto value = ImageValue::Create(url::URL {u8"https://example.com/a.png"});
      CHECK(ToNarrow(value->URL().Specified) == "https://example.com/a.png");
      CHECK(ToNarrow(value->URL().Resolved.String()) == "https://example.com/a.png");
      CHECK(CssText(*value) == "url(\"https://example.com/a.png\")");
    }

    SECTION("Invalid images serialise as an empty string")
    {
      auto value = ImageValue::Create();
      CHECK(value->IsImageValue());
      CHECK(CssText(*value).empty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue::IsLoadedFromOpaqueSource", "[CSS][Values][ImageValue]")
  {
    auto url = ImageURL(u8"a.png");
    url.Modifiers.LoadedFromOpaqueSource = LoadedFromOpaqueSource::Yes;
    CHECK(ImageValue::Create(url)->IsLoadedFromOpaqueSource());
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue equality", "[CSS][Values][ImageValue]")
  {
    auto a = ImageValue::Create(ImageURL(u8"a.png"));
    CHECK(*a == *ImageValue::Create(ImageURL(u8"a.png")));
    CHECK_FALSE(*a == *ImageValue::Create(ImageURL(u8"b.png")));

    // The initiator type isn't compared.
    CHECK(*a == *ImageValue::Create(ImageURL(u8"a.png"), CSSOMStringAtom {u8"css"}));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue::CopyForComputedStyle", "[CSS][Values][ImageValue]")
  {
    auto value = ImageValue::Create(ImageURL(u8"a.png"));
    CHECK(Children(*value).empty());

    SECTION("The same URL shares the value")
    {
      auto copy = value->CopyForComputedStyle(ImageURL(u8"a.png"));
      CHECK(copy.get() == value.get());
    }

    SECTION("A different URL creates a value that visits the unresolved one")
    {
      auto resolved = ImageURL(u8"b.png");
      auto copy = value->CopyForComputedStyle(resolved);
      CHECK(copy.get() != value.get());
      CHECK(copy->URL() == resolved);
      CHECK_FALSE(*copy == *value);

      auto children = Children(*copy);
      REQUIRE(children.size() == 1uz);
      CHECK(children[0uz] == value.get());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageValue base URL and subresources", "[CSS][Values][ImageValue]")
  {
    CHECK(ImageValue::Create(ImageURL(u8"a.png"))->MayDependOnBaseUrl());
    CHECK_FALSE(ImageValue::Create(ImageURL(u8"#image"))->MayDependOnBaseUrl());
    CHECK_FALSE(ImageValue::Create(ImageURL(u8"data:image/png;base64,AAAA"))->MayDependOnBaseUrl());
    CHECK_FALSE(ImageValue::Create()->MayDependOnBaseUrl());

    // No image is loaded, so there are no subresources.
    bool called = false;
    CHECK_FALSE(ImageValue::Create(ImageURL(u8"a.png"))
                  ->TraverseSubresources(
                    [&](const CachedResource &)
                    {
                      called = true;
                      return true;
                    }));
    CHECK_FALSE(called);
  }
}
