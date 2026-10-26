#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom::tests
{
  // FIXME: ElementFactory looks HTML elements up by their lowercased local name, so `DIV` in the HTML
  // namespace is a div; the element interface for a local name is case-sensitive. (createElement lowercases
  // names in HTML documents before they get here.)
  TEST_CASE("ElementFactory: HTML's local names are case-sensitive", "[DOM][ElementFactory][!shouldfail]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), DOMStringAtom(u8"DIV"));
    REQUIRE_FALSE(element.HasException());

    auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element.Value());
    REQUIRE(htmlElement != nullptr);
    CHECK(Is<html::HTMLUnknownElement>(*htmlElement));
  }
}
