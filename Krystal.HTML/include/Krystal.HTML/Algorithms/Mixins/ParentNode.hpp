#pragma once

#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.HTML/Utils/NodeOrString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;
  class NodeList;
}

namespace Krys::HTML::Mixins
{
  class ParentNode
  {
  public:
    /// @see https://dom.spec.whatwg.org/#convert-nodes-into-a-node
    KRYS_NODISCARD static ExceptionOr<Ref<Node>> ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                                      Document &document) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> Prepend(ContainerNode &parent,
                                                    const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> Append(ContainerNode &parent,
                                                   const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> ReplaceChildren(ContainerNode &parent,
                                                            const List<NodeOrString> &nodes) noexcept;

    KRYS_NODISCARD static ExceptionOr<void> MoveBefore(ContainerNode &parent, Node &node,
                                                       RawPtr<Node> refChild) noexcept;

    KRYS_NODISCARD static ExceptionOr<RefPtr<Element>> QuerySelector(ContainerNode &parent,
                                                                     DOMStringView selectors) noexcept;

    KRYS_NODISCARD static ExceptionOr<Ref<NodeList>> QuerySelectorAll(ContainerNode &parent,
                                                                      DOMStringView selectors) noexcept;
  };
}