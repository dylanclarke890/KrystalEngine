#include "Krystal.HTML/DOM/DOMImplementation.hpp"
#include "Krystal.HTML/Constants/Namespaces.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/SVGDocument.hpp"
#include "Krystal.HTML/DOM/XHTMLDocument.hpp"
#include "Krystal.HTML/DOM/XMLDocument.hpp"
#include "Krystal.HTML/Factories/ElementFactory.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"

namespace Krys::HTML
{
  ExceptionOr<Ref<DocumentType>> DOMImplementation::CreateDocumentType(const DOMString &name,
                                                                       const DOMString &publicId,
                                                                       const DOMString &systemId) noexcept
  {
    if (auto document = _document.lock())
    {
      if (!NameValidation::IsValidDocTypeName(name))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }

      return AdoptRef<DocumentType>(*new DocumentType(*document, name, publicId, systemId));
    }

    return Exception {ExceptionCode::InvalidStateError};
  }

  ExceptionOr<Ref<XMLDocument>> DOMImplementation::CreateDocument(DOMStringAtom namespaceUri,
                                                                  DOMStringAtom qualifiedName,
                                                                  RawPtr<DocumentType> docType) noexcept
  {
    Ref<XMLDocument> document = [&] -> Ref<XMLDocument>
    {
      RefPtr<XMLDocument> doc = nullptr;
      if (namespaceUri == Namespace::HTML)
      {
        doc = CreateRefPtr<XHTMLDocument>();
        doc->_contentType = u8"application/xhtml+xml";
      }
      else if (namespaceUri == Namespace::SVG)
      {
        doc = CreateRefPtr<SVGDocument>();
        doc->_contentType = u8"image/svg+xml";
      }
      else
      {
        doc = CreateRefPtr<XMLDocument>();
        doc->_contentType = u8"application/xml";
      }

      return AdoptRef<XMLDocument>(*doc.release());
    }();

    RefPtr<Element> element = nullptr;
    if (qualifiedName != DOMStringAtom::Empty())
    {
      auto elementCreateResult =
        DocumentAlgorithms::InternalCreateElementNS(*document, namespaceUri, qualifiedName, {});
      if (elementCreateResult.HasException())
      {
        return elementCreateResult.ReleaseException();
      }

      element = AdoptRefPtr(elementCreateResult.Value().release());
    }

    if (docType != nullptr)
    {
      if (auto appendResult = document->AppendChild(*docType); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    if (element != nullptr)
    {
      if (auto appendResult = document->AppendChild(*element); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    // SPEC-VIOLATION(ORIGIN) - doc’s origin is this’s associated document’s origin.

    return document;
  }

  ExceptionOr<Ref<Document>> DOMImplementation::CreateHTMLDocument(const Maybe<DOMString> &title) noexcept
  {
    Ref<HTMLDocument> document = CreateRef<HTMLDocument>();
    document->_contentType = u8"text/html";

    auto documentType = AdoptRef(*new DocumentType(*document, u8"html", u8"", u8""));
    if (auto appendResult = document->AppendChild(*documentType); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    auto qualifiedName = QualifiedName {Namespace::HTML, DOMStringAtom::Null(), u8"html"};
    auto htmlElement = ElementFactory::Create(*document, qualifiedName);
    if (auto appendResult = document->AppendChild(*htmlElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    qualifiedName.LocalName = u8"head";
    auto headElement = ElementFactory::Create(*document, qualifiedName);
    if (auto appendResult = document->AppendChild(*headElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    if (title.has_value())
    {
      qualifiedName.LocalName = u8"title";
      auto titleElement = ElementFactory::Create(*document, qualifiedName);
      if (auto appendResult = headElement->AppendChild(*titleElement); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      DOMString titleData = title.value();
      auto textNode = CreateRef<Text>(*document, Krys::Move(titleData));
      if (auto appendResult = titleElement->AppendChild(*textNode); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    qualifiedName.LocalName = u8"body";
    auto bodyElement = ElementFactory::Create(*document, qualifiedName);
    if (auto appendResult = document->AppendChild(*bodyElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    // TODO(impl): ORIGIN - doc’s origin is this’s associated document’s origin.

    return AdoptRef<Document>(*document.release());
  }
}