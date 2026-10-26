#include "Krystal.Booey/SVG/SVGURIReference.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/Element.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGURIReference: references", "[SVG][SVGURIReference]")
  {
    auto document = ResolverFixture::Parse(u8"<svg><g id=target /></svg>");
    auto documentURL = dom::DOMString {document->GetURL().String()};

    CHECK(SVGURIReference::FragmentIdentifierFromIRIString(u8"#target", *document) == u8"target");
    CHECK(SVGURIReference::FragmentIdentifierFromIRIString(documentURL + u8"#target", *document)
          == u8"target");
    CHECK(SVGURIReference::FragmentIdentifierFromIRIString(u8"target", *document) == u8"");
    CHECK(SVGURIReference::FragmentIdentifierFromIRIString(u8"https://example.com/a.svg#target", *document)
          == u8"");

    CHECK_FALSE(SVGURIReference::IsExternalURIReference(u8"#target", *document));
    CHECK_FALSE(SVGURIReference::IsExternalURIReference(documentURL + u8"#target", *document));
    CHECK(SVGURIReference::IsExternalURIReference(u8"https://example.com/a.svg#target", *document));

    auto &scope = document->TreeScope();
    auto result = SVGURIReference::TargetElementFromIRIString(u8"#target", scope);
    CHECK(result.Element == document->GetElementById(u8"target"));
    CHECK(result.Identifier == u8"target");

    result = SVGURIReference::TargetElementFromIRIString(u8"#missing", scope);
    CHECK(result.Element == nullptr);
    CHECK(result.Identifier == u8"missing");

    result = SVGURIReference::TargetElementFromIRIString(u8"https://example.com/a.svg#target", scope);
    CHECK(result.Element == nullptr);
    CHECK(result.Identifier == u8"target");

    result = SVGURIReference::TargetElementFromIRIString(u8"target", scope);
    CHECK(result.Element == nullptr);
    CHECK(result.Identifier == dom::DOMStringAtom::Null());
  }
}
