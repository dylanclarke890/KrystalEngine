#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"

namespace krys::web::html
{
  class CustomElementRegistry;
}

namespace krys::web
{
  class ContainerNode;
  class Document;
  class Element;
  class HTMLCollection;
  class Node;
}

namespace krys::web::dom
{
  /// @see https://dom.spec.whatwg.org/#mutation-algorithms
  class NodeAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#string-replace-all
    KRYS_NODISCARD static ExceptionOr<void> StringReplaceAll(DOMString string,
                                                             ContainerNode &parent) noexcept;

    /// @param document The copy's node document; null for @p node's.
    /// @see https://dom.spec.whatwg.org/#clone-a-node-document
    /// @param subtree Whether to clone @p node's children too.
    /// @see https://dom.spec.whatwg.org/#clone-a-node-subtree
    /// @param parent Where to append the copy, if anywhere.
    /// @see https://dom.spec.whatwg.org/#clone-a-node-parent
    /// @param fallbackRegistry The registry for copied elements that have none.
    /// @see https://dom.spec.whatwg.org/#clone-a-node-fallbackregistry
    /// @see https://dom.spec.whatwg.org/#concept-node-clone
    KRYS_NODISCARD static ExceptionOr<Ref<Node>>
      CloneNode(Node &node, Document *document = nullptr, bool subtree = false,
                ContainerNode *parent = nullptr,
                html::CustomElementRegistry *fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#clone-a-single-node
    KRYS_NODISCARD static Ref<Node>
      CloneSingleNode(const Node &node, Document &document,
                      html::CustomElementRegistry *fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-equals
    KRYS_NODISCARD static bool Equals(const Node &A, const Node &B) noexcept;

    /// @see https://dom.spec.whatwg.org/#locate-a-namespace-prefix
    KRYS_NODISCARD static DOMStringAtom LocateNamespacePrefix(const Element &element,
                                                              const DOMStringAtom &namespaceURI) noexcept;

    /// @see https://dom.spec.whatwg.org/#locate-a-namespace
    KRYS_NODISCARD static DOMStringAtom LocateNamespace(const Node &node,
                                                        const DOMStringAtom &prefix) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-getelementsbytagname
    KRYS_NODISCARD static Ref<HTMLCollection>
      GetElementsByTagName(ContainerNode &root, const DOMStringAtom &qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-getelementsbytagnamens
    KRYS_NODISCARD static Ref<HTMLCollection> GetElementsByTagNameNS(ContainerNode &root,
                                                                     DOMStringAtom namespaceUri,
                                                                     const DOMStringAtom &localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-getelementsbyclassname
    KRYS_NODISCARD static Ref<HTMLCollection>
      GetElementsByClassName(ContainerNode &root, const DOMStringAtom &classNames) noexcept;
  };
}
