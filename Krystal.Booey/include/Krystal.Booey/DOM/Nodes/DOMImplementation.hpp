#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo::dom
{
  class Document;
  class DocumentType;
  class XMLDocument;

  /// @see https://dom.spec.whatwg.org/#interface-domimplementation
  class DOMImplementation
  {
    friend class Document;

  private:
    WeakRef<Document> _document;

  private:
    DOMImplementation(Document &document) noexcept : _document(CreateWeakRef(document))
    {
    }

  public:
#pragma region DOMImplementation - https://dom.spec.whatwg.org/#interface-domimplementation

    /// @brief Returns a doctype, with the given `name`, `publicId`, and `systemId`.
    /// @throws InvalidCharacterError if `name` is not a valid doctype name.
    /// @see https://dom.spec.whatwg.org/#dom-domimplementation-createdocumenttype
    KRYS_NODISCARD ExceptionOr<Ref<DocumentType>> CreateDocumentType(const DOMString &name,
                                                                     const DOMString &publicId,
                                                                     const DOMString &systemId) noexcept;

    /// @brief Returns an XMLDocument, with a document element whose local name is `qualifiedName` and whose
    /// namespace is `namespaceUri` (unless `qualifiedName` is the empty string), and with `doctype`, if it is
    /// given, as its doctype.
    /// @throws This method throws the same exceptions as the createElementNS() method, when invoked with
    /// `namespaceUri` and `qualifiedName`. A null `qualifiedName` is the empty string.
    /// @see https://dom.spec.whatwg.org/#dom-domimplementation-createdocument
    KRYS_NODISCARD ExceptionOr<Ref<XMLDocument>>
      CreateDocument(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName = DOMStringAtom::Empty(),
                     DocumentType *doctype = nullptr) noexcept;

    /// @brief Returns a document, with a basic tree already constructed including a title element, unless the
    /// `title` is omitted.
    /// @see https://dom.spec.whatwg.org/#dom-domimplementation-createhtmldocument
    KRYS_NODISCARD ExceptionOr<Ref<Document>>
      CreateHTMLDocument(const Maybe<DOMString> &title = null) noexcept;

    /// @note Useless; always returns true.
    /// @see https://dom.spec.whatwg.org/#dom-domimplementation-hasfeature
    KRYS_NODISCARD bool HasFeature() const noexcept
    {
      return true;
    }

#pragma endregion

    /// @brief Makes the document a load of @p contentType at @p url shows: XHTML, SVG and other XML
    /// documents for their types, and an HTML document of the type for the rest (a text document's, whose
    /// parser the writer starts in plain text).
    // FIXME: Image and media documents, once there are those.
    KRYS_NODISCARD static Ref<Document> CreateDocument(utf8_stringview contentType,
                                                       const url::URL &url) noexcept;
  };
}