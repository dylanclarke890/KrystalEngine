#include "Krystal.Booey/Html/Elements/HTMLBaseElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Html/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/Html/Parser/DocumentParser.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<HTMLDocument> ParseWithBase(utf8_string &&input) noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      document->SetUrl(net::Url(u8"https://example.com/dir/page.html"));
      auto parser = CreateRef<DocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF::Yes);
      parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD HTMLBaseElement *AsBase(const RefPtr<Element> &element) noexcept
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
    CHECK(base->GetAttribute(u8"href") == DOMString(u8"other/"));
  }

  TEST_CASE("HTMLBaseElement.target: reflected, and the first base target is the document's",
            "[HTML][HTMLBaseElement][html:attr-base-target]")
  {
    auto document = ParseWithBase(u8"<base href=a/><base target=_top><base target=_blank>");
    CHECK(document->BaseTarget() == u8"_top");

    auto *base = AsBase(document->Head()->FirstElementChild());
    CHECK(base->Target().empty());
    base->Target(u8"frame");
    CHECK(base->Target() == u8"frame");
    CHECK(document->BaseTarget() == u8"frame");
  }
}
