#include "Krystal.HTML/DOM/DOMImplementation.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/XMLDocument.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
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