#include "Krystal.Booey/HTML/Elements/HTMLQuoteElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLQuoteElement.cite: the cite attribute parsed against the document's base URL",
            "[HTML][HTMLQuoteElement][html:attr-blockquote-cite][html:attr-q-cite]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
    for (auto localName : {u8"blockquote", u8"q"})
    {
      auto element = document->CreateElement(dom::DOMStringAtom(localName)).ReleaseValue();
      auto &quote = Downcast<HTMLQuoteElement>(Downcast<HTMLElement>(*element));
      CHECK(quote.Cite() == u8"");

      quote.Cite(u8"source.html");
      CHECK(element->GetAttribute(u8"cite") == u8"source.html");
      CHECK(quote.Cite() == u8"https://example.com/dir/source.html");
    }
  }
}
