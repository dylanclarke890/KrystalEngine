#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace krys::boo::dom::tests
{
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