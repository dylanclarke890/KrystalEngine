#include "Krystal.Booey/DOM/Document.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    Ref<HTMLDocument> Parse(utf8_string &&input,
                            utf8_stringview url = u8"https://example.com/dir/page.html") noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      document->SetURL(url::URL(url));
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    // NOTE: `Document::Head()` returns null for a parsed document, so the head is found by position.
    RefPtr<Element> HeadOf(Document &document) noexcept
    {
      return document.DocumentElement()->FirstElementChild();
    }

    Ref<Element> CreateBase(Document &document, DOMString &&href) noexcept
    {
      auto base = document.CreateElement(u8"base").ReleaseValue();
      REQUIRE_FALSE(base->SetAttribute(u8"href", krys::move(href)).HasException());
      return base;
    }
  }

  TEST_CASE("Document::URL", "[DOM][Document]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("A new document's URL is about:blank")
    {
      REQUIRE(document->URL() == u8"about:blank");
      REQUIRE(document->DocumentURI() == u8"about:blank");
      REQUIRE(document->BaseURL() == url::AboutBlankURL());
      REQUIRE(document->BaseURI() == u8"about:blank");
    }

    SECTION("Setting the URL changes the base URL")
    {
      document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
      REQUIRE(document->URL() == u8"https://example.com/dir/page.html");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");

      auto element = CreateRef<TestElement>(*document);
      REQUIRE(element->BaseURI() == u8"https://example.com/dir/page.html");
    }

    SECTION("An empty URL becomes about:blank")
    {
      document->SetURL(url::URL(u8"https://example.com/"));
      document->SetURL(url::URL());
      REQUIRE(document->URL() == u8"about:blank");
    }

    SECTION("CompleteURL resolves against the base URL")
    {
      document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
      REQUIRE(document->CompleteURL(u8"a.css") == u8"https://example.com/dir/a.css");
      REQUIRE(document->CompleteURL(u8"a.css", url::URL(u8"https://other.com/x/"))
              == u8"https://other.com/x/a.css");
    }

    SECTION("A cloned document keeps the URL")
    {
      document->SetURL(url::URL(u8"https://example.com/"));
      auto clone = document->CloneNode().ReleaseValue();
      REQUIRE(Downcast<Document>(clone.get())->URL() == u8"https://example.com/");
    }
  }

  TEST_CASE("Document base URL from base elements", "[DOM][Document]")
  {
    SECTION("The first base element with an href sets the base URL, relative to the document's URL")
    {
      auto document = Parse(u8"<base target=_top><base href=sub/><base href=https://other.com/>");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/sub/");
      REQUIRE(document->CompleteURL(u8"a.css") == u8"https://example.com/dir/sub/a.css");
    }

    SECTION("data: and javascript: base URLs are ignored")
    {
      REQUIRE(Parse(u8"<base href='data:text/plain,a'>")->BaseURL() == u8"https://example.com/dir/page.html");
      REQUIRE(Parse(u8"<base href='javascript:void(0)'>")->BaseURL()
              == u8"https://example.com/dir/page.html");
    }

    SECTION("An unparsable base URL is ignored")
    {
      REQUIRE(Parse(u8"<base href='https://[invalid'>")->BaseURL() == u8"https://example.com/dir/page.html");
    }

    SECTION("Changing or removing the href updates the base URL")
    {
      auto document = Parse(u8"<base href=a/>");
      auto base = HeadOf(*document)->FirstElementChild();
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/a/");

      REQUIRE_FALSE(base->SetAttribute(u8"href", u8"b/").HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/b/");

      base->RemoveAttribute(u8"href");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");
    }

    SECTION("Inserting and removing base elements updates the base URL")
    {
      auto document = Parse(u8"<base href=a/>");
      auto head = HeadOf(*document);
      auto first = head->FirstElementChild();

      auto earlier = CreateBase(*document, u8"b/");
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/a/");

      REQUIRE_FALSE(head->Prepend({Ref<Node>(earlier)}).HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/b/");

      REQUIRE_FALSE(earlier->Remove().HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/a/");

      REQUIRE_FALSE(first->Remove().HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");
    }

    SECTION("Base elements in shadow trees are ignored")
    {
      auto document = Parse(u8"<div></div>");
      auto host = document->Body()->FirstElementChild();
      auto shadow = host->AttachShadow({}).ReleaseValue();

      auto base = CreateBase(*document, u8"a/");
      REQUIRE_FALSE(shadow->Append({Ref<Node>(base)}).HasException());
      REQUIRE(document->BaseURL() == u8"https://example.com/dir/page.html");
    }
  }

  TEST_CASE("Document::CssParserContext", "[DOM][Document][CSS]")
  {
    SECTION("Follows the base URL and mode")
    {
      auto document = Parse(u8"<base href=sub/>");
      auto &context = document->CssParserContext();
      REQUIRE(context.BaseURL == u8"https://example.com/dir/sub/");
      REQUIRE(context.Mode == css::ParserMode::HTMLQuirks);
      REQUIRE(context.IsHTMLDocument);
      REQUIRE(context.HasDocumentSecurityOrigin);

      REQUIRE(Parse(u8"<!DOCTYPE html>")->CssParserContext().Mode == css::ParserMode::HTMLStandard);
    }

    SECTION("Is recreated when the base URL changes")
    {
      auto document = Parse(u8"<!DOCTYPE html>");
      REQUIRE(document->CssParserContext().BaseURL == u8"https://example.com/dir/page.html");

      document->SetURL(url::URL(u8"https://other.com/"));
      REQUIRE(document->CssParserContext().BaseURL == u8"https://other.com/");
    }

    SECTION("Isn't an HTML document's context for other documents")
    {
      auto document = CreateRef<Document>();
      REQUIRE_FALSE(document->CssParserContext().IsHTMLDocument);
    }

    SECTION("The Document constructors")
    {
      auto document = Parse(u8"<p>");

      REQUIRE(css::ParserContext(*document) == document->CssParserContext());
      REQUIRE(css::MediaQueryParserContext(*document).Context == document->CssParserContext());
      REQUIRE(css::SelectorParserContext(*document).Mode == css::ParserMode::HTMLQuirks);
      REQUIRE(css::SelectorParserContext(*document)
              == css::SelectorParserContext(document->CssParserContext()));

      css::ParserContext sheetContext(*document, url::URL(u8"https://cdn.com/a.css"), u8"utf-8");
      REQUIRE(sheetContext.BaseURL == u8"https://cdn.com/a.css");
      REQUIRE(sheetContext.Charset == u8"utf-8");
      REQUIRE(sheetContext.Mode == css::ParserMode::HTMLQuirks);
    }
  }
}
