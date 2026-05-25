#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/SmallNodeList.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;

  class TreeMutationDispatcher
  {
  public:
    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-record
    static void QueueMutationRecord(DOMString &&type, Node &target, Maybe<DOMStringAtom> name,
                                    Maybe<DOMStringAtom> namespaceURI, Maybe<DOMStringView> oldValue,
                                    const SmallNodeList &addedNodes, const SmallNodeList &removedNodes,
                                    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-tree-mutation-record
    static void QueueTreeMutationRecord(Node &target, const SmallNodeList &addedNodes,
                                        const SmallNodeList &removedNodes, RefPtr<Node> &&previousSibling,
                                        RefPtr<Node> &&nextSibling) noexcept;

  };
}