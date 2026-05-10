#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/ElementCreationOptions.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
{
  class Document;
  class Element;
  class Node;

  class DocumentAlgorithms
  {
  public:
    KRYS_NODISCARD static ExceptionOr<ElementCreationOptions>
      FlattenElementCreationOptions(const ElementCreationOptionsOrString &options,
                                    Document &document) noexcept;

    KRYS_NODISCARD static ExceptionOr<Ref<Element>>
      InternalCreateElementNS(Document &document, DOMStringAtom namespaceUri, DOMStringAtom qualifiedName,
                              const ElementCreationOptionsOrString &options) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> AdoptNode(Node &node, Document &document) noexcept;
  };
}