#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class HTMLSlotElement;
  class Node;
}

namespace krys::boo::dom
{
  class MutationObserver;
  enum class MutationRecordType : uint8;
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
    static void AppendSignalSlot(HTMLSlotElement &slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#add-transient-registered-observers
    static void AddTransientRegisteredObservers(Node &node, Node &parent) noexcept;

    /// @see https://dom.spec.whatwg.org/#remove-transient-registered-observers
    static void RemoveTransientRegisteredObservers(MutationObserver &observer) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-record
    static void QueueMutationRecord(MutationRecordType type, const Ref<Node> &target,
                                    const DOMStringAtom &name, const DOMStringAtom &namespaceURI,
                                    const DOMStringAtom &oldValue, const SmallNodeList &addedNodes,
                                    const SmallNodeList &removedNodes, const RefPtr<Node> &previousSibling,
                                    const RefPtr<Node> &nextSibling) noexcept;

    /// @brief Whether a mutation record for a change to `target`'s attribute `name` (in the null namespace)
    /// would carry the old value, i.e. an interested observer set `attributeOldValue`. Lets callers skip
    /// computing an old value nobody reads.
    KRYS_NODISCARD static bool IsAttributeOldValueRequested(Node &target, const DOMStringAtom &name) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-tree-mutation-record
    static void QueueTreeMutationRecord(Ref<Node> target, SmallNodeList addedNodes,
                                        SmallNodeList removedNodes, RefPtr<Node> previousSibling,
                                        RefPtr<Node> nextSibling) noexcept;

  private:
    /// @brief @p node's registered observer list: its registered observers, then its transient registered
    /// observers, which a node keeps in a list of their own.
    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    KRYS_NODISCARD static List<Ref<RegisteredObserver>> RegisteredObserverList(Node &node) noexcept;
  };
}
