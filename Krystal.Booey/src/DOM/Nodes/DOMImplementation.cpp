#include "Krystal.Booey/DOM/Nodes/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Nodes/XHTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
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
      // 1. If name is not a valid doctype name, then throw an "InvalidCharacterError" DOMException.
      if (!NameValidation::IsValidDocTypeName(name))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }

      // 2. Return the result of creating a doctype given the associated document of this, name, publicId, and
      //    systemId.
      return AdoptRef<DocumentType>(*new DocumentType(*document, name, publicId, systemId));
    }

    return Exception {ExceptionCode::InvalidStateError};
  }

  ExceptionOr<Ref<XMLDocument>> DOMImplementation::CreateDocument(DOMStringAtom namespaceUri,
                                                                  DOMStringAtom qualifiedName,
                                                                  DocumentType *docType) noexcept
  {
    // 1. Let document be the result of creating a document that implements XMLDocument, given this's relevant
    //    realm.
    // 7. document's content type is determined by namespace:
    //    - HTML namespace: application/xhtml+xml
    //    - SVG namespace: image/svg+xml
    //    - Any other namespace: application/xml
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

    // 2. Let element be null.
    RefPtr<Element> element = nullptr;

    // 3. If qualifiedName is not the empty string, then set element to the result of running the internal
    //    createElementNS steps, given document, namespace, qualifiedName, and an empty dictionary.
    if (qualifiedName != DOMStringAtom::Empty() && qualifiedName != DOMStringAtom::Null())
    {
      auto elementCreateResult =
        DocumentAlgorithms::InternalCreateElementNS(*document, namespaceUri, qualifiedName, {});
      if (elementCreateResult.HasException())
      {
        return elementCreateResult.ReleaseException();
      }

      element = AdoptRefPtr(elementCreateResult.Value().release());
    }

    // 4. If doctype is non-null, append doctype to document.
    if (docType != nullptr)
    {
      if (auto appendResult = document->AppendChild(*docType); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    // 5. If element is non-null, append element to document.
    if (element != nullptr)
    {
      if (auto appendResult = document->AppendChild(*element); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    // 6. (Skipped: documents have no origin.)

    // 8. Return document.
    return document;
  }

  ExceptionOr<Ref<Document>> DOMImplementation::CreateHTMLDocument(const Maybe<DOMString> &title) noexcept
  {
    // 1. Let doc be the result of creating a document that implements Document, given this's relevant realm.
    // 2. Set doc's type to "html" and content type to "text/html".
    Ref<HTMLDocument> document = CreateRef<HTMLDocument>();
    document->_contentType = u8"text/html";
    document->SetSettings(ShareRef(_document->GetSettings()));

    // 3. Let doctype be the result of creating a doctype given doc and "html".
    auto documentType = AdoptRef(*new DocumentType(*document, u8"html", u8"", u8""));

    // 4. Append doctype to doc.
    if (auto appendResult = document->AppendChild(*documentType); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    // 5. Append the result of creating an element given doc, "html", and the HTML namespace, to doc.
    auto htmlElement =
      ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"html"});
    if (auto appendResult = document->AppendChild(*htmlElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    // 6. Append the result of creating an element given doc, "head", and the HTML namespace, to the html
    // element
    //    created earlier.
    auto headElement =
      ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"head"});
    if (auto appendResult = htmlElement->AppendChild(*headElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    // 7. If title is given:
    if (title.has_value())
    {
      // 7a. Append the result of creating an element given doc, "title", and the HTML namespace, to the head
      //     element created earlier.
      auto titleElement =
        ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"title"});
      if (auto appendResult = headElement->AppendChild(*titleElement); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      // 7b. Let text be the result of creating a text node given doc and title (which could be the empty
      //     string).
      DOMString titleData = title.value();
      auto textNode = CreateRef<Text>(*document, krys::move(titleData));

      // 7c. Append text to the title element created earlier.
      if (auto appendResult = titleElement->AppendChild(*textNode); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    // 8. Append the result of creating an element given doc, "body", and the HTML namespace, to the html
    // element
    //    created earlier.
    auto bodyElement =
      ElementFactory::Create(*document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"body"});
    if (auto appendResult = htmlElement->AppendChild(*bodyElement); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    // 9. (Skipped: documents have no origin.)

    // 10. Return doc.
    return AdoptRef<Document>(*document.release());
  }

  Ref<Document> DOMImplementation::CreateDocument(utf8_stringview contentType, const url::URL &url) noexcept
  {
    auto create = [&]() -> Ref<Document>
    {
      if (text::IsASCIICaselessEqual(contentType, u8"text/html"))
      {
        return CreateRef<HTMLDocument>();
      }
      if (text::IsASCIICaselessEqual(contentType, u8"application/xhtml+xml"))
      {
        auto document = CreateRef<XHTMLDocument>();
        document->_contentType = u8"application/xhtml+xml";
        return document;
      }
      if (text::IsASCIICaselessEqual(contentType, u8"image/svg+xml"))
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

      // A text document is an HTML document of its type.
      // https://html.spec.whatwg.org/#navigate-text
      auto document = CreateRef<HTMLDocument>();
      if (!contentType.empty())
      {
        document->_contentType = DOMString(contentType);
      }
      return document;
    };

    auto document = create();
    if (!url.IsNull())
    {
      document->SetURL(url);
    }
    return document;
  }
}
