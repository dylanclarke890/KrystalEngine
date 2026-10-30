#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XHTMLDocument.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::dom
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
                                                                  DocumentType *docType) noexcept
  {
    Ref<XMLDocument> document = [&] -> Ref<XMLDocument>
    {
      RefPtr<XMLDocument> doc = nullptr;
      if (namespaceUri == infra::Namespaces::HTML)
      {
        doc = CreateRefPtr<XHTMLDocument>();
        doc->_contentType = u8"application/xhtml+xml";
      }
      else if (namespaceUri == infra::Namespaces::SVG)
      {
        doc = CreateRefPtr<svg::SVGDocument>();
        doc->_contentType = u8"image/svg+xml";
      }
      else
      {
        doc = CreateRefPtr<XMLDocument>();
        doc->_contentType = u8"application/xml";
      }

      return AdoptRef<XMLDocument>(*doc.release());
    }();
    document->SetSettings(ShareRef(_document->GetSettings()));

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
    document->SetSettings(ShareRef(_document->GetSettings()));

    auto documentType = AdoptRef(*new DocumentType(*document, u8"html", u8"", u8""));
    if (auto appendResult = document->AppendChild(*documentType); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    auto htmlElement =
      ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"html"});
    if (auto appendResult = document->AppendChild(*htmlElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    auto headElement =
      ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"head"});
    if (auto appendResult = htmlElement->AppendChild(*headElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    if (title.has_value())
    {
      auto titleElement =
        ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"title"});
      if (auto appendResult = headElement->AppendChild(*titleElement); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      DOMString titleData = title.value();
      auto textNode = CreateRef<Text>(*document, krys::move(titleData));
      if (auto appendResult = titleElement->AppendChild(*textNode); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    auto bodyElement =
      ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"body"});
    if (auto appendResult = htmlElement->AppendChild(*bodyElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    // FIXME: Doc’s origin is this’s associated document’s origin.

    return AdoptRef<Document>(*document.release());
  }

  Ref<Document> DOMImplementation::CreateDocument(utf8_stringview contentType, const url::URL &url) noexcept
  {
    auto create = [&]() -> Ref<Document>
    {
      if (text::IsASCIICaselessEqual(contentType, utf8_stringview(u8"text/html")))
      {
        return CreateRef<HTMLDocument>();
      }
      if (text::IsASCIICaselessEqual(contentType, utf8_stringview(u8"application/xhtml+xml")))
      {
        auto document = CreateRef<XHTMLDocument>();
        document->_contentType = u8"application/xhtml+xml";
        return document;
      }
      if (text::IsASCIICaselessEqual(contentType, utf8_stringview(u8"image/svg+xml")))
      {
        auto document = CreateRef<svg::SVGDocument>();
        document->_contentType = u8"image/svg+xml";
        return document;
      }
      if (MIMETypeRegistry::IsXMLMIMEType(contentType))
      {
        auto document = CreateRef<XMLDocument>();
        document->_contentType = DOMString(contentType);
        return document;
      }

      return CreateRef<HTMLDocument>();
    };

    auto document = create();
    if (!url.IsNull())
    {
      document->SetURL(url::URL(url));
    }
    return document;
  }
}
