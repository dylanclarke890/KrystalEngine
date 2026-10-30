#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("ElementFactory: HTML's local names are case-sensitive", "[DOM][ElementFactory]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("An uppercase name in the HTML namespace is unknown")
    {
      auto element =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"DIV"));
      REQUIRE_FALSE(element.HasException());
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
      REQUIRE(htmlElement != nullptr);
      CHECK(Is<html::HTMLUnknownElement>(*htmlElement));
    }

    SECTION("A lowercase name in the HTML namespace is its element")
    {
      auto element =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"div"));
      REQUIRE_FALSE(element.HasException());
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
      REQUIRE(htmlElement != nullptr);
      CHECK(Is<html::HTMLDivElement>(*htmlElement));
    }

    SECTION("createElement lowercases the name in an HTML document")
    {
      auto element = document->CreateElement(DOMStringAtom(u8"DIV"));
      REQUIRE_FALSE(element.HasException());
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
      REQUIRE(htmlElement != nullptr);
      CHECK(Is<html::HTMLDivElement>(*htmlElement));
      CHECK(element.Value()->LocalName() == u8"div");
    }
  }
}
