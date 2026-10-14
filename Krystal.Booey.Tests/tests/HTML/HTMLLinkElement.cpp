#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLLinkElement::RelList", "[HTML][HTMLLinkElement]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto link = CreateRef<HTMLLinkElement>(*document);
    REQUIRE_FALSE(link->SetAttribute(u8"rel", u8"stylesheet icon").HasException());

    auto &relList = link->RelList();
    REQUIRE(relList.Length() == 2uz);
    REQUIRE(relList.Contains(u8"icon"));

    REQUIRE_FALSE(relList.Remove(dom::DOMString(u8"icon")).HasException());
    REQUIRE(link->GetAttribute(u8"rel") == u8"stylesheet");
  }
}
