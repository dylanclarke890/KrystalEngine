#pragma once

#include "Krystal.HTML/DOM/Enums/MutationRecordType.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
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