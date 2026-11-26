#pragma once

#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Html/Types/DocumentReadyState.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class Document;
  class Element;
  class Node;
}

namespace krys::boo::dom
{
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
    static void UpdateCurrentDocumentReadiness(Document &document,
                                               html::DocumentReadyState readiness) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#an-iframe-srcdoc-document
    KRYS_NODISCARD static bool IsIFrameSrcdocDocument(const Document &document) noexcept;
  };
}
