#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Internals/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/MutationRecord/CharacterDataMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/MutationRecord/ChildListMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include <cassert>

namespace krys::boo::dom
{
  namespace
  {
    struct InterestedObserver
    {
      Ref<MutationObserver> Observer;
      DOMStringAtom OldValue;
    };
  }

  void MutationObserverAlgorithms::QueueMutationObserverMicrotask() noexcept
  {
    // TODO(impl): MUTATION-OBSERVERS - Queue a mutation observer microtask
    // If the surrounding agent’s mutation observer microtask queued is true, then return.
    // Set the surrounding agent’s mutation observer microtask queued to true.
    // Queue a microtask to notify mutation observers.
  }

  void MutationObserverAlgorithms::NotifyMutationObservers() noexcept
  {
    // TODO(impl): MUTATION-OBSERVERS - notify mutation observers
    // Set the surrounding agent’s mutation observer microtask queued to false.
    // Let notifySet be a clone of the surrounding agent’s pending mutation observers.
    // Empty the surrounding agent’s pending mutation observers.
    // Let signalSet be a clone of the surrounding agent’s signal slots.
    // Empty the surrounding agent’s signal slots.
    // For each mo of notifySet:
    //     Let records be a clone of mo’s record queue.
    //     Empty mo’s record queue.
    //     For each node of mo’s node list: remove all transient registered observers whose observer is mo
    //     from node’s registered observer list.
    //     If records is not empty, then invoke mo’s callback with « records, mo » and "report", and with
    //     callback this value mo.
    // For each slot of signalSet: fire an event named slotchange, with its bubbles attribute set to true, at
    // slot.
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
    for (auto &node : nodes)
    {
      for (auto &registered : node.RegisteredObservers())
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

      // TODO(impl): MUTATION-OBSERVERS - Append observer to the surrounding agent’s pending mutation
      // observers.
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
