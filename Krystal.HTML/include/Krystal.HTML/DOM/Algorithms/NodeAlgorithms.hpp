#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class CustomElementRegistry;
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
      CloneNode(Node &node, RawPtr<Document> document = nullptr, bool subtree = false,
                RawPtr<ContainerNode> parent = nullptr,
                RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#clone-a-single-node
    KRYS_NODISCARD static Ref<Node>
      CloneSingleNode(const Node &node, Document &document,
                      RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

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