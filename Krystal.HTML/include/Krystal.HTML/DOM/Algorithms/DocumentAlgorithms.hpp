#pragma once

#include "Krystal.HTML/DOM/Dicts/ElementCreationOptions.hpp"
#include "Krystal.HTML/HTML/Enums/DocumentReadyState.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  class Document;
  class Element;
  class Node;

  class DocumentAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#xml-document
    KRYS_NODISCARD static bool IsXMLTypeDocument(const Document &document) noexcept;

    /// @see https://dom.spec.whatwg.org/#internal-createelementns-steps
    KRYS_NODISCARD static ExceptionOr<Ref<Element>>
      InternalCreateElementNS(Document &document, DOMStringAtom namespaceUri, DOMStringAtom qualifiedName,
                              const ElementCreationOptionsOrString &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#flatten-element-creation-options
    KRYS_NODISCARD static ExceptionOr<ElementCreationOptions>
      FlattenElementCreationOptions(const ElementCreationOptionsOrString &options,
                                    Document &document) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-adopt
    KRYS_NODISCARD static ExceptionOr<void> AdoptNode(Node &node, Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#update-the-current-document-readiness
    void UpdateCurrentDocumentReadiness(Document &document, DocumentReadyState readiness) noexcept;
  };
}