#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class HTMLCollection;
  class Node;

  /// @see https://dom.spec.whatwg.org/#mutation-algorithms
  class NodeAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#string-replace-all
    KRYS_NODISCARD static ExceptionOr<void> StringReplaceAll(DOMString &&string,
                                                             ContainerNode &parent) noexcept;

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
                                                              DOMStringAtom namespaceURI) noexcept;

    /// @see https://dom.spec.whatwg.org/#locate-a-namespace
    KRYS_NODISCARD static DOMStringAtom LocateNamespace(const Node &node, DOMStringAtom prefix) noexcept;

    /// https://dom.spec.whatwg.org/#concept-getelementsbytagname
    KRYS_NODISCARD static Ref<HTMLCollection> GetElementsByTagName(ContainerNode &root,
                                                                   DOMStringAtom qualifiedName) noexcept;

    /// https://dom.spec.whatwg.org/#concept-getelementsbytagnamens
    KRYS_NODISCARD static Ref<HTMLCollection> GetElementsByTagNameNS(ContainerNode &root,
                                                                     DOMStringAtom namespaceUri,
                                                                     DOMStringAtom localName) noexcept;

    /// https://dom.spec.whatwg.org/#concept-getelementsbyclassname
    KRYS_NODISCARD static Ref<HTMLCollection> GetElementsByClassName(ContainerNode &root,
                                                                     DOMStringAtom classNames) noexcept;
  };
}