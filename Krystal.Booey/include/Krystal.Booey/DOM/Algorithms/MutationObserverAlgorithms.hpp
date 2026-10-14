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

    /// @brief Whether a mutation record for a change to `target`'s attribute `name` (in the null namespace)
    /// would carry the old value, i.e. an interested observer set `attributeOldValue`. Lets callers skip
    /// computing an old value nobody reads.
    KRYS_NODISCARD static bool IsAttributeOldValueRequested(Node &target, DOMStringAtom name) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-tree-mutation-record
    static void QueueTreeMutationRecord(Ref<Node> &&target, SmallNodeList &&addedNodes,
                                        SmallNodeList &&removedNodes, RefPtr<Node> &&previousSibling,
                                        RefPtr<Node> &&nextSibling) noexcept;
  };
}