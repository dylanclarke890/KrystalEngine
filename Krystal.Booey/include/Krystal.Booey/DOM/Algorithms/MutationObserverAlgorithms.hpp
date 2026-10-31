#pragma once

#include "Krystal.Booey/DOM/Enums/MutationRecordType.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class HTMLSlotElement;
}

namespace krys::boo::dom
{
  class MutationObserver;
  class Node;
  class RegisteredObserver;

  /// @see https://dom.spec.whatwg.org/#mutation-observers
  /// @see https://dom.spec.whatwg.org/#queueing-a-mutation-record
  class MutationObserverAlgorithms
  {
  public:
    /// @brief Returns the surrounding agent's mutation observer counter and increases it by 1, for a new
    /// observer's creation index.
    /// @see https://dom.spec.whatwg.org/#mutation-observer-counter
    KRYS_NODISCARD static uint64 TakeMutationObserverCounter() noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-observer-compound-microtask
    static void QueueMutationObserverMicrotask() noexcept;

    /// @see https://dom.spec.whatwg.org/#notify-mutation-observers
    static void NotifyMutationObservers() noexcept;

    /// @brief Appends @p slot to the surrounding agent's signal slots, which get `slotchange` when mutation
    /// observers are next notified.
    /// @see https://dom.spec.whatwg.org/#signal-slot-list
    static void AppendSignalSlot(html::HTMLSlotElement &slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#add-transient-registered-observers
    static void AddTransientRegisteredObservers(Node &node, Node &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#remove-transient-registered-observers
    static void RemoveTransientRegisteredObservers(MutationObserver &observer) noexcept;

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

  private:
    /// @brief @p node's registered observer list: its registered observers, then its transient registered
    /// observers, which a node keeps in a list of their own.
    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    KRYS_NODISCARD static List<Ref<RegisteredObserver>> RegisteredObserverList(Node &node) noexcept;
  };
}