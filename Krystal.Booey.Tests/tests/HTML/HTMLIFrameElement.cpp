#include "Krystal.Booey/HTML/HTMLIFrameElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD HTMLIFrameElement &AsIFrame(dom::Element &element) noexcept
    {
      return Downcast<HTMLIFrameElement>(Downcast<HTMLElement>(element));
    }
  }

  TEST_CASE("HTMLIFrameElement: srcdoc is the attribute's value, and loading reflects its known values",
            "[HTML][HTMLIFrameElement][html:dom-iframe-srcdoc][html:dom-iframe-loading]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"iframe").ReleaseValue();
    auto &iframe = AsIFrame(*element);

    CHECK(iframe.SrcDoc() == u8"");
    iframe.SrcDoc(u8"<p>Hello");
    CHECK(element->GetAttribute(u8"srcdoc") == u8"<p>Hello");
    CHECK(iframe.SrcDoc() == u8"<p>Hello");

    CHECK(css::tests::ToNarrow(iframe.Loading()) == "eager");
    iframe.Loading(u8"LAZY");
    CHECK(css::tests::ToNarrow(iframe.Loading()) == "lazy");
    iframe.Loading(u8"soon");
    CHECK(css::tests::ToNarrow(iframe.Loading()) == "eager");
    CHECK(element->GetAttribute(u8"loading") == u8"soon");
  }
}
