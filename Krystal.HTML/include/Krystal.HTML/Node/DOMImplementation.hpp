#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Document;
  class XMLDocument;

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
    KRYS_NODISCARD ExceptionOr<Ref<DocumentType>> CreateDocumentType(const DOMString &name,
                                                                     const DOMString &publicId,
                                                                     const DOMString &systemId) noexcept;

    KRYS_NODISCARD ExceptionOr<Ref<XMLDocument>>
      CreateDocument(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName = DOMStringAtom::Null(),
                     RefPtr<DocumentType> &&doctype = nullptr) noexcept;

    KRYS_NODISCARD ExceptionOr<Ref<Document>>
      CreateHTMLDocument(const Maybe<DOMString> &title = Null) noexcept;

    KRYS_NODISCARD bool HasFeature() const noexcept
    {
      return true;
    }
  };
}