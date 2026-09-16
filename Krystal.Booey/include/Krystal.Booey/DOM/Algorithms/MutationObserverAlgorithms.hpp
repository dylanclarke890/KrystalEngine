#pragma once

#include "Krystal.Booey/DOM/Enums/MutationRecordType.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#mutation-observers
  /// @see https://dom.spec.whatwg.org/#queueing-a-mutation-record
  class MutationObserverAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-observer-compound-microtask
    static void QueueMutationObserverMicrotask() noexcept;

    /// @see https://dom.spec.whatwg.org/#notify-mutation-observers
    static void NotifyMutationObservers() noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-record
    static void QueueMutationRecord(MutationRecordType type, Ref<Node> &&target, DOMStringAtom name,
                                    DOMStringAtom namespaceURI, DOMStringAtom oldValue,
                                    SmallNodeList &&addedNodes, SmallNodeList &&removedNodes,
                                    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-tree-mutation-record
    static void QueueTreeMutationRecord(Ref<Node> &&target, SmallNodeList &&addedNodes,
                                        SmallNodeList &&removedNodes, RefPtr<Node> &&previousSibling,
                                        RefPtr<Node> &&nextSibling) noexcept;
  };
}