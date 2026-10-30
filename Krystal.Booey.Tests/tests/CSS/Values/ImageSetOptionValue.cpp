#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<Value> Image(utf8_stringview url)
    {
      return ImageValue::Create(*CompleteURL(
        CSSOMString {url}, ParserContext {ParserMode::HTMLStandard, url::URL {u8"https://example.com/"}}));
    }

    Ref<PrimitiveValue> X(double value)
    {
      return PrimitiveValue::Create(value, UnitType::x);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetOptionValue::Create", "[CSS][Values][ImageSetOptionValue]")
  {
    SECTION("The resolution defaults to 1x")
    {
      auto option = ImageSetOptionValue::Create(Image(u8"a.png"));
      CHECK(option->IsImageSetOptionValue());
      CHECK_FALSE(option->IsImage());
      CHECK(option->Image().IsImageValue());
      CHECK(CssText(option->Resolution()) == "1x");
      CHECK_FALSE(option->Type());
      CHECK(CssText(*option) == "url(\"a.png\") 1x");
    }

    SECTION("With a resolution")
    {
      auto option = ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0));
      CHECK(CssText(option->Resolution()) == "2x");
      CHECK(option->ProtectedResolution().get() == &option->Resolution());
      CHECK_FALSE(option->Type());
      CHECK(CssText(*option) == "url(\"a.png\") 2x");
    }

    SECTION("With a type")
    {
      auto option = ImageSetOptionValue::Create(Image(u8"a.avif"), X(2.0), u8"image/avif");
      REQUIRE(option->Type());
      CHECK(ToNarrow(*option->Type()) == "image/avif");
      CHECK(CssText(*option) == "url(\"a.avif\") 2x type(\"image/avif\")");
    }

    SECTION("An empty type is still written")
    {
      auto option = ImageSetOptionValue::Create(Image(u8"a.png"), X(1.0), u8"");
      REQUIRE(option->Type());
      CHECK(CssText(*option) == "url(\"a.png\") 1x type(\"\")");
    }

    SECTION("Image generators are accepted")
    {
      auto option = ImageSetOptionValue::Create(NamedImageValue::Create(u8"name"));
      CHECK(CssText(*option) == "-webkit-named-image(name) 1x");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetOptionValue setters", "[CSS][Values][ImageSetOptionValue]")
  {
    auto option = ImageSetOptionValue::Create(Image(u8"a.png"));
    option->SetResolution(PrimitiveValue::Create(96.0, UnitType::dpi));
    CHECK(CssText(option->Resolution()) == "96dpi");

    option->SetType(u8"image/png");
    REQUIRE(option->Type());
    CHECK(ToNarrow(*option->Type()) == "image/png");
    CHECK(CssText(*option) == "url(\"a.png\") 96dpi type(\"image/png\")");
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetOptionValue equality", "[CSS][Values][ImageSetOptionValue]")
  {
    auto a = ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0), u8"image/png");
    CHECK(*a == *ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0), u8"image/png"));
    CHECK_FALSE(*a == *ImageSetOptionValue::Create(Image(u8"b.png"), X(2.0), u8"image/png"));
    CHECK_FALSE(*a == *ImageSetOptionValue::Create(Image(u8"a.png"), X(1.0), u8"image/png"));
    CHECK_FALSE(*a == *ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0), u8"image/avif"));

    // No type isn't the same as an empty type.
    CHECK_FALSE(*ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0))
                == *ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0), u8""));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "ImageSetOptionValue children and subresources",
                   "[CSS][Values][ImageSetOptionValue]")
  {
    auto option = ImageSetOptionValue::Create(Image(u8"a.png"), X(2.0));
    auto children = Children(*option);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == &option->Image());
    CHECK(children[1uz] == &option->Resolution());

    // Only the values themselves are asked, not their children.
    CHECK_FALSE(option->MayDependOnBaseUrl());

    // Nothing is loaded, so there are no subresources.
    CHECK_FALSE(option->TraverseSubresources([](const CachedResource &) { return true; }));
  }
}
