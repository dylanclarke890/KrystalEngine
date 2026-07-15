#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
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
#pragma region DOMImplementation - https://dom.spec.whatwg.org/#dom-domimplementation

    /// @brief Returns a doctype, with the given `name`, `publicId`, and `systemId`.
    /// @throws InvalidCharacterError if `name` is not a valid doctype name.
    KRYS_NODISCARD ExceptionOr<Ref<DocumentType>> CreateDocumentType(const DOMString &name,
                                                                     const DOMString &publicId,
                                                                     const DOMString &systemId) noexcept;

    /// @brief Returns an XMLDocument, with a document element whose local name is `qualifiedName` and whose
    /// namespace is `namespaceUri` (unless `qualifiedName` is the empty string), and with `doctype`, if it is
    /// given, as its doctype.
    /// @throws This method throws the same exceptions as the createElementNS() method, when invoked with
    /// `namespaceUri` and `qualifiedName`.
    KRYS_NODISCARD ExceptionOr<Ref<XMLDocument>>
      CreateDocument(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName = DOMStringAtom::Null(),
                     RawPtr<DocumentType> doctype = nullptr) noexcept;

    /// @brief Returns a document, with a basic tree already constructed including a title element, unless the
    /// `title` is omitted.
    KRYS_NODISCARD ExceptionOr<Ref<Document>>
      CreateHTMLDocument(const Maybe<DOMString> &title = Null) noexcept;

    KRYS_NODISCARD bool HasFeature() const noexcept
    {
      return true;
    }

#pragma endregion
  };
}