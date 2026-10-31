#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Internals/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Internals/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/MutationRecord/CharacterDataMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/MutationRecord/ChildListMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <cassert>
#include <ranges>

namespace krys::boo::dom
{
  namespace
  {
    struct InterestedObserver
    {
      Ref<MutationObserver> Observer;
      DOMStringAtom OldValue;
    };

    /// @brief The surrounding agent's mutation observer state.
    /// @see https://dom.spec.whatwg.org/#mutation-observers
    // FIXME: Process-wide until there are agents.
    struct MutationObserverAgent
    {
      /// @see https://dom.spec.whatwg.org/#mutation-observer-compound-microtask-queued-flag
      bool MutationObserverMicrotaskQueued {false};
      /// @see https://dom.spec.whatwg.org/#mutation-observer-list
      List<Ref<MutationObserver>> PendingMutationObservers;
      /// @see https://dom.spec.whatwg.org/#signal-slot-list
      List<Ref<html::HTMLSlotElement>> SignalSlots;
      /// @see https://dom.spec.whatwg.org/#mutation-observer-counter
      uint64 MutationObserverCounter {0};
    };

    KRYS_NODISCARD MutationObserverAgent &SurroundingAgent() noexcept
    {
      static NeverDestroyed<MutationObserverAgent> agent;
      return agent;
    }

    template <typename T>
    void AppendToSet(List<Ref<T>> &set, T &item) noexcept
    {
      if (std::ranges::none_of(set, [&](const Ref<T> &existing) { return existing.get() == &item; }))
      {
        set.push_back(ShareRef(item));
      }
    }
  }

  uint64 MutationObserverAlgorithms::TakeMutationObserverCounter() noexcept
  {
    return SurroundingAgent().MutationObserverCounter++;
  }

  void MutationObserverAlgorithms::QueueMutationObserverMicrotask() noexcept
  {
    auto &agent = SurroundingAgent();

    // 1. If the surrounding agent's mutation observer microtask queued is true, then return.
    if (agent.MutationObserverMicrotaskQueued)
    {
      return;
    }

    // 2. Set the surrounding agent's mutation observer microtask queued to true.
    agent.MutationObserverMicrotaskQueued = true;

    // 3. Queue a microtask to notify mutation observers.
    html::EventLoop::Current().QueueMicrotask([] { NotifyMutationObservers(); });
  }

  void MutationObserverAlgorithms::NotifyMutationObservers() noexcept
  {
    auto &agent = SurroundingAgent();

    // 1. Set the surrounding agent's mutation observer microtask queued to false.
    agent.MutationObserverMicrotaskQueued = false;

    // 2. Let notifyList be the result of sorting the surrounding agent's pending mutation observers in
    // ascending
    //    order, with a being less than b if a's creation index is less than b's creation index.
    // 3. Empty the surrounding agent's pending mutation observers.
    auto notifyList = std::exchange(agent.PendingMutationObservers, {});
    std::ranges::sort(notifyList, {},
                      [](const Ref<MutationObserver> &observer) { return observer->_creationIndex; });

    // 4. Let signalSet be a clone of the surrounding agent's signal slots.
    // 5. Empty the surrounding agent's signal slots.
    auto signalSet = std::exchange(agent.SignalSlots, {});

    // 6. For each mo of notifyList:
    for (auto &mo : notifyList)
    {
      // 6a. Let records be a clone of mo's record queue.
      // 6b. Empty mo's record queue.
      auto records = std::exchange(mo->_recordQueue, {});

      // 6c. Remove transient registered observers for mo.
      RemoveTransientRegisteredObservers(*mo);

      // 6d. If records is not empty, then invoke mo's callback with « records, mo » and "report", and with
      //     callback this value mo.
      // There's no script, so nothing to report.
      if (!records.empty())
      {
        mo->_callback(records, *mo);
      }
    }

    // https://dom.spec.whatwg.org/#eventdef-htmlslotelement-slotchange
    // 7. For each slot of signalSet: fire an event named slotchange, with its bubbles attribute set to true,
    // at
    //    slot.
    for (auto &slot : signalSet)
    {
      (void)EventDispatcher::FireEvent<Event>(DOMStringAtom(EventNames::SlotChange), *slot,
                                              EventInit {.Bubbles = true});
    }
  }

  void MutationObserverAlgorithms::AppendSignalSlot(html::HTMLSlotElement &slot) noexcept
  {
    AppendToSet(SurroundingAgent().SignalSlots, slot);
  }

  void MutationObserverAlgorithms::AddTransientRegisteredObservers(Node &node, Node &parent) noexcept
  {
    auto add = [&](RegisteredObserver &registered, RegisteredObserver &source)
    {
      // 1a. If registered's options["subtree"] is false, then continue.
      if (!registered.Options().Subtree)
      {
        return;
      }

      // 1b. Let observer be registered's observer.
      Ref<MutationObserver> observer = registered.Observer();

      // 1c. Let source be registered's source if registered is a transient registered observer; otherwise
      //     registered.
      // 1d. If node's registered observer list contains a transient registered observer whose source is
      // source,
      //     then continue.
      auto &transients = node.TransientRegisteredObservers();
      if (std::ranges::any_of(transients, [&](const Ref<TransientRegisteredObserver> &transient)
                              { return transient->Source().get() == &source; }))
      {
        return;
      }

      // 1e. Append a new transient registered observer whose observer is observer, options is source's
      // options,
      //     and source is source to node's registered observer list.
      transients.push_back(
        CreateRef<TransientRegisteredObserver>(Ref {observer}, source.Options(), ShareRef(source)));

      // 1f. Append a weak reference to node to observer's transient node set.
      if (std::ranges::none_of(observer->_transientNodes,
                               [&](const WeakRef<Node> &existing) { return existing.get() == &node; }))
      {
        observer->_transientNodes.emplace_back(CreateWeakRef(node));
      }

      // 1g. Append observer to the surrounding agent's pending mutation observers.
      AppendToSet(SurroundingAgent().PendingMutationObservers, *observer);

      // 1h. Queue a mutation observer microtask.
      QueueMutationObserverMicrotask();
    };

    // 1. For each inclusive ancestor inclusiveAncestor of parent, and then for each registered of
    //    inclusiveAncestor's registered observer list:
    // Registered observers and transient ones are kept in separate lists.
    for (auto &inclusiveAncestor : InclusiveAncestorRange(parent))
    {
      for (auto &registered : List<Ref<RegisteredObserver>>(inclusiveAncestor.RegisteredObservers()))
      {
        add(*registered, *registered);
      }
      for (auto &transient :
           List<Ref<TransientRegisteredObserver>>(inclusiveAncestor.TransientRegisteredObservers()))
      {
        add(*transient, *transient->Source());
      }
    }
  }

  void MutationObserverAlgorithms::RemoveTransientRegisteredObservers(MutationObserver &observer) noexcept
  {
    // 1. For each node of observer's transient node set: remove all transient registered observers whose
    //    observer is observer from node's registered observer list.
    for (auto &weakNode : observer._transientNodes)
    {
      if (auto node = weakNode.lock())
      {
        std::erase_if(node->TransientRegisteredObservers(),
                      [&](const Ref<TransientRegisteredObserver> &transient)
                      { return transient->Observer().get() == &observer; });
      }
    }

    // 2. Empty observer's transient node set.
    observer._transientNodes.clear();
  }

  bool MutationObserverAlgorithms::IsAttributeOldValueRequested(Node &target, DOMStringAtom name) noexcept
  {
    // The interested observers of "queue a mutation record" (steps 2 and 3) for an attribute in the null
    // namespace.
    for (auto &node : InclusiveAncestorRange(target))
    {
      for (auto &registered : node.RegisteredObservers())
      {
        auto options = registered->Options();
        if (&node != &target && !options.Subtree)
        {
          continue;
        }

        if (!options.Attributes.value_or(false))
        {
          continue;
        }

        if (options.AttributeFilter.has_value() && !std::ranges::contains(*options.AttributeFilter, name))
        {
          continue;
        }

        if (options.AttributeOldValue == true)
        {
          return true;
        }
      }
    }

    return false;
  }

  void MutationObserverAlgorithms::QueueMutationRecord(MutationRecordType type, Ref<Node> &&target,
                                                       DOMStringAtom name, DOMStringAtom namespaceURI,
                                                       DOMStringAtom oldValue, SmallNodeList &&addedNodes,
                                                       SmallNodeList &&removedNodes,
                                                       RefPtr<Node> &&previousSibling,
                                                       RefPtr<Node> &&nextSibling) noexcept
  {
    // 1. Let interestedObservers be an empty map.
    List<InterestedObserver> interestedObservers;

    // 2. Let nodes be the inclusive ancestors of target.
    auto nodes = InclusiveAncestorRange(*target);

    // 3. For each node in nodes, and then for each registered of node's registered observer list:
    // NOTE: a node keeps its transient registered observers in a list of their own.
    for (auto &node : nodes)
    {
      List<Ref<RegisteredObserver>> registeredObservers = node.RegisteredObservers();
      for (auto &transient : node.TransientRegisteredObservers())
      {
        registeredObservers.push_back(Ref<RegisteredObserver>(transient));
      }
      for (auto &registered : registeredObservers)
      {
        // 3.1. Let options be registered's options.
        auto options = registered->Options();

        // 3.2. If none of the following are true, then:
        //   - node is not target and options["subtree"] is false
        //   - type is "attributes" and options["attributes"] either does not exist or is false
        //   - type is "attributes", options["attributeFilter"] exists, and options["attributeFilter"] does
        //   not
        //     contain name or namespace is non-null
        //   - type is "characterData" and options["characterData"] either does not exist or is false
        //   - type is "childList" and options["childList"] is false
        if (&node != target.get() && !options.Subtree)
        {
          continue;
        }

        if (type == MutationRecordType::Attributes)
        {
          if (!options.Attributes.value_or(false))
          {
            continue;
          }

          if (options.AttributeFilter.has_value()
              && (!std::ranges::contains(*options.AttributeFilter, name)
                  || namespaceURI != DOMStringAtom::Null()))
          {
            continue;
          }
        }
        else if (type == MutationRecordType::CharacterData && !options.CharacterData.value_or(false))
        {
          continue;
        }
        else if (type == MutationRecordType::ChildList && !options.ChildList)
        {
          continue;
        }

        // 3.2.1. Let mo be registered's observer.
        Ref<MutationObserver> observer = registered->Observer();

        // 3.2.2. If interestedObservers[mo] does not exist, then set interestedObservers[mo] to null.
        auto it = std::ranges::find_if(interestedObservers, [&](const InterestedObserver &interested)
                                       { return interested.Observer.get() == observer.get(); });
        if (it == interestedObservers.end())
        {
          interestedObservers.push_back({krys::move(observer), DOMStringAtom::Null()});
          it = std::prev(interestedObservers.end());
        }

        // 3.2.3. If either type is "attributes" and options["attributeOldValue"] is true, or type is
        // "characterData" and options["characterDataOldValue"] is true, then set interestedObservers[mo] to
        // oldValue.
        if ((type == MutationRecordType::Attributes && options.AttributeOldValue == true)
            || (type == MutationRecordType::CharacterData && options.CharacterDataOldValue == true))
        {
          it->OldValue = oldValue;
        }
      }
    }

    // 4. For each observer -> mappedOldValue of interestedObservers:
    for (auto &[observer, mappedOldValue] : interestedObservers)
    {
      // 4.1. Let record be a new MutationRecord object whose type is type, target is target, attributeName is
      // name, attributeNamespace is namespace, oldValue is mappedOldValue, addedNodes is addedNodes,
      // removedNodes is removedNodes, previousSibling is previousSibling, and nextSibling is nextSibling.
      RefPtr<MutationRecord> record = nullptr;
      switch (type)
      {
        case MutationRecordType::Attributes:
        {
          record = CreateRef<AttributesMutationRecord>(Ref<Node>(target), name, namespaceURI, mappedOldValue);
          break;
        }
        case MutationRecordType::CharacterData:
        {
          record = CreateRef<CharacterDataMutationRecord>(Ref<Node>(target), mappedOldValue);
          break;
        }
        case MutationRecordType::ChildList:
        {
          record = CreateRef<ChildListMutationRecord>(
            Ref<Node>(target), CreateRef<StaticNodeList>(SmallNodeList(addedNodes)),
            CreateRef<StaticNodeList>(SmallNodeList(removedNodes)), RefPtr<Node>(previousSibling),
            RefPtr<Node>(nextSibling));
          break;
        }
        default:
        {
          assert(false && "Unknown MutationRecordType");
          break;
        }
      }

      // 4.2. Enqueue record to observer's record queue.
      observer->_recordQueue.push_back(krys::move(record));

      // 4.3. Append observer to the surrounding agent's pending mutation observers.
      AppendToSet(SurroundingAgent().PendingMutationObservers, *observer);
    }

    // 5. Queue a mutation observer microtask.
    QueueMutationObserverMicrotask();
  }

  void MutationObserverAlgorithms::QueueTreeMutationRecord(Ref<Node> &&target, SmallNodeList &&addedNodes,
                                                           SmallNodeList &&removedNodes,
                                                           RefPtr<Node> &&previousSibling,
                                                           RefPtr<Node> &&nextSibling) noexcept
  {
    assert(addedNodes.size() > 0 || removedNodes.size() > 0);
    QueueMutationRecord(MutationRecordType::ChildList, krys::move(target), DOMStringAtom::Null(),
                        DOMStringAtom::Null(), DOMStringAtom::Null(), krys::move(addedNodes),
                        krys::move(removedNodes), krys::move(previousSibling), krys::move(nextSibling));
  }
}
