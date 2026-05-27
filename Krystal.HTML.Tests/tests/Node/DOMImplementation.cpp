#include "Krystal.HTML/Node/DOMImplementation.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/XMLDocument.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("DOMImplementation::CreateDocumentType", "[Node][DOMImplementation]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto &implementation = document->Implementation();
    auto doctype = implementation.CreateDocumentType(u8"html", u8"", u8"");
    REQUIRE(doctype->Name() == u8"html");
    REQUIRE(doctype->PublicId() == u8"");
    REQUIRE(doctype->SystemId() == u8"");
    REQUIRE(implementation.CreateDocumentType(u8"invalid name", u8"", u8"")
            == ExceptionCode::InvalidCharacterError);
  }

  // TODO(test): add tests for CreateDocument and CreateHTMLDocument
}