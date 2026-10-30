#include "Krystal.Booey/Style/Values/Content/Content.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/Style/Values/Content/Quotes.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  TEST_CASE_METHOD(BuilderFixture, "Style Content", "[Style][Values][Content]")
  {
    CHECK(Convert<Content>(PropertyId::Content, u8"normal").IsNormal());
    CHECK(Convert<Content>(PropertyId::Content, u8"none").IsNone());
    CHECK(Serialised(Convert<Content>(PropertyId::Content, u8"none")) == "none");

    auto content = Convert<Content>(PropertyId::Content,
                                    u8"\"a\" open-quote counter(item) counters(item, \".\", upper-roman) "
                                    u8"url(https://example.com/a.png) / \"alt\"");
    REQUIRE(content.IsData());
    const auto &data = *content.TryData();
    REQUIRE(data.List.size() == 5);
    CHECK(std::get<Content::Text>(data.List[0]).Value == u8"a");
    CHECK(std::get<Content::Quote>(data.List[1]).Value == QuoteType::OpenQuote);
    CHECK(std::holds_alternative<Content::Counter>(data.List[2]));
    CHECK(std::holds_alternative<Content::Image>(data.List[4]));
    CHECK(data.AltText == u8"alt");
    CHECK(Serialised(content)
          == "\"a\" open-quote counter(item) counters(item, \".\", upper-roman) "
             "url(\"https://example.com/a.png\") / "
             "\"alt\"");

    SECTION("attr()")
    {
      auto element = Document->CreateElement(u8"div").ReleaseValue();
      (void)element->SetAttribute(u8"title", u8"from attribute");
      Reset(element.get());

      auto withAttr = Convert<Content>(PropertyId::Content, u8"attr(title) / attr(title)");
      REQUIRE(withAttr.IsData());
      CHECK(std::get<Content::Text>(withAttr.TryData()->List[0]).Value == u8"from attribute");
      CHECK(withAttr.TryData()->AltText == u8"from attribute");
      CHECK(Style.HasAttrContent());

      // A missing attribute is empty.
      auto missing = Convert<Content>(PropertyId::Content, u8"attr(lang)");
      CHECK(std::get<Content::Text>(missing.TryData()->List[0]).Value.empty());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style Quotes", "[Style][Values][Content]")
  {
    CHECK(Convert<Quotes>(PropertyId::Quotes, u8"auto").IsAuto());
    CHECK(Convert<Quotes>(PropertyId::Quotes, u8"none").IsNone());

    auto quotes = Convert<Quotes>(PropertyId::Quotes, u8"\"<\" \">\" \"[\" \"]\"");
    REQUIRE(quotes.IsQuotes());
    CHECK(quotes.OpenQuote(0) == u8"<");
    CHECK(quotes.CloseQuote(0) == u8">");
    CHECK(quotes.OpenQuote(1) == u8"[");
    // Deeper levels reuse the last pair.
    CHECK(quotes.CloseQuote(5) == u8"]");
    CHECK(Serialised(quotes) == "\"<\" \">\" \"[\" \"]\"");
    CHECK(Convert<Quotes>(PropertyId::Quotes, u8"none").OpenQuote(0).empty());

    // Copies share their strings.
    auto copy = quotes;
    CHECK((copy == quotes));
  }

  TEST_CASE_METHOD(BuilderFixture, "Style Cursor", "[Style][Values][UI]")
  {
    auto pointer = Convert<Cursor>(PropertyId::Cursor, u8"pointer");
    CHECK(pointer.Predefined == CursorType::Pointer);
    CHECK_FALSE(pointer.Images);
    CHECK(Serialised(pointer) == "pointer");

    auto cursor = Convert<Cursor>(
      PropertyId::Cursor, u8"url(https://example.com/a.png) 3 4, url(https://example.com/b.png), auto");
    CHECK(cursor.Predefined == CursorType::Auto);
    REQUIRE(cursor.Images);
    REQUIRE(cursor.Images->size() == 2);
    CHECK((*cursor.Images)[0].HotSpot == IntPoint {3, 4});
    // No hot spot.
    CHECK((*cursor.Images)[1].HotSpot == IntPoint {-1, -1});
    CHECK((*cursor.Images)[0].Image->IsCursorImage());
    CHECK(Serialised(cursor)
          == "url(\"https://example.com/a.png\") 3 4, url(\"https://example.com/b.png\"), auto");
  }
}
