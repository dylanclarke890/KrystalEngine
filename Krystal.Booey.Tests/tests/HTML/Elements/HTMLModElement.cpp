#include "Krystal.Booey/HTML/Elements/HTMLModElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE(
    "HTMLModElement: cite is a URL parsed against the document's base URL, and dateTime is as written",
    "[HTML][HTMLModElement][html:attr-mod-cite][html:attr-mod-datetime]")
  {
    auto document = CreateRef<HTMLDocument>();
    document->SetURL(net::URL(u8"https://example.com/dir/page.html"));
    for (auto localName : {u8"ins", u8"del"})
    {
      auto element = document->CreateElement(DOMStringAtom(localName)).ReleaseValue();
      auto &mod = Downcast<HTMLModElement>(Downcast<HTMLElement>(*element));
      CHECK(mod.Cite().empty());
      CHECK(mod.DateTime().empty());

      mod.Cite(u8"changes.html");
      CHECK(element->GetAttribute(u8"cite") == u8"changes.html");
      CHECK(mod.Cite() == u8"https://example.com/dir/changes.html");

      // The date isn't parsed.
      mod.DateTime(u8"not a date");
      CHECK(mod.DateTime() == u8"not a date");
    }
  }
}
