#include "Krystal.Booey/HTML/Elements/HTMLBaseElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> ParseWithBase(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
      auto parser = CreateRef<DocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD HTMLBaseElement *AsBase(const RefPtr<dom::Element> &element) noexcept
    {
      REQUIRE(element);
      return &Downcast<HTMLBaseElement>(Downcast<HTMLElement>(*element));
    }
  }

  TEST_CASE("HTMLBaseElement.href: the href parsed against the fallback base URL, or as it is",
            "[HTML][HTMLBaseElement][html:dom-base-href][html:attr-base-href]")
  {
    auto document = ParseWithBase(u8"<base href=first/><base href=second/><base><base href='https://[x'>");
    auto *base = AsBase(document->Head()->FirstElementChild());
    REQUIRE(base);

    // Not affected by itself or the other base elements.
    CHECK(base->Href() == u8"https://example.com/dir/first/");
    auto *second = AsBase(base->NextElementSibling());
    CHECK(second->Href() == u8"https://example.com/dir/second/");

    // Without an href, the fallback base URL; a URL that doesn't parse is returned as it is.
    auto *none = AsBase(second->NextElementSibling());
    CHECK(none->Href() == u8"https://example.com/dir/page.html");
    CHECK(AsBase(none->NextElementSibling())->Href() == u8"https://[x");

    base->Href(u8"other/");
    CHECK(base->GetAttribute(u8"href") == dom::DOMString(u8"other/"));
  }

  TEST_CASE("HTMLBaseElement.target: reflected, and the first base target is the document's",
            "[HTML][HTMLBaseElement][html:attr-base-target]")
  {
    auto document = ParseWithBase(u8"<base href=a/><base target=_top><base target=_blank>");
    CHECK(document->BaseTarget() == u8"_top");

    auto *base = AsBase(document->Head()->FirstElementChild());
    CHECK(base->Target() == u8"");
    base->Target(u8"frame");
    CHECK(base->Target() == u8"frame");
    CHECK(document->BaseTarget() == u8"frame");
  }
}
