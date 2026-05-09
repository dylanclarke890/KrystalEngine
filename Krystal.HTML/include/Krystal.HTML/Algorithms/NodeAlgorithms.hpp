#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.HTML/Utils/NodeOrString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class CustomElementRegistry;
  class Document;
  class Node;

  /// @see https://dom.spec.whatwg.org/#mutation-algorithms
  class NodeAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#string-replace-all
    KRYS_NODISCARD static ExceptionOr<void> StringReplaceAll(DOMString &&string,
                                                             ContainerNode &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone
    static Ref<Node> CloneNode(const Node &node, RawPtr<Document> document = nullptr, bool subtree = false,
                               RawPtr<ContainerNode> parent = nullptr,
                               RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#clone-a-single-node
    KRYS_NODISCARD static Ref<Node>
      CloneSingleNode(const Node &node, Document &document,
                      RawPtr<CustomElementRegistry> fallbackRegistry = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#convert-nodes-into-a-node
    KRYS_NODISCARD static ExceptionOr<Ref<Node>> ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                                      Document &document) noexcept;
  };
}