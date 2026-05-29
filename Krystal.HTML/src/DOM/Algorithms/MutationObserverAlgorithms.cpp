#include "Krystal.HTML/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Internals/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.HTML/DOM/Internals/MutationRecord/CharacterDataMutationRecord.hpp"
#include "Krystal.HTML/DOM/Internals/MutationRecord/ChildListMutationRecord.hpp"
#include "Krystal.HTML/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.HTML/DOM/MutationObserver.hpp"
#include <cassert>

namespace Krys::HTML
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

  void MutationObserverAlgorithms::QueueMutationRecord(MutationRecordType type, Ref<Node> &&target,
                                                       DOMStringAtom name, DOMStringAtom namespaceURI,
                                                       DOMStringAtom oldValue, SmallNodeList &&addedNodes,
                                                       SmallNodeList &&removedNodes,
                                                       RefPtr<Node> &&previousSibling,
                                                       RefPtr<Node> &&nextSibling) noexcept
  {
    List<InterestedObserver> interestedObservers;

    auto nodes = InclusiveAncestorRange(*target);
    for (auto &node : nodes)
    {
      for (auto &registered : node.RegisteredObservers())
      {
        auto options = registered->Options();

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

          if (namespaceURI == DOMStringAtom::Null())
          {
            continue;
          }

          if (!options.AttributeFilter.has_value() || !std::ranges::contains(*options.AttributeFilter, name))
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

        InterestedObserver interestedObserver {registered->Observer(), DOMStringAtom::Null()};

        bool isAttrChange = type == MutationRecordType::Attributes && options.Attributes == true;
        bool isCharDataChange =
          type == MutationRecordType::CharacterData && options.CharacterDataOldValue == true;
        if (isAttrChange || isCharDataChange)
        {
          interestedObserver.OldValue = oldValue;
        }

        interestedObservers.push_back(std::move(interestedObserver));
      }
    }

    for (auto &[observer, mappedOldValue] : interestedObservers)
    {
      RefPtr<MutationRecord> record = nullptr;
      switch (type)
      {
        case MutationRecordType::Attributes:
        {
          record = CreateRef<AttributesMutationRecord>(Krys::Move(target), name, namespaceURI, oldValue);
          break;
        }
        case MutationRecordType::CharacterData:
        {
          record = CreateRef<CharacterDataMutationRecord>(Krys::Move(target), oldValue);
          break;
        }
        case MutationRecordType::ChildList:
        {
          record = CreateRef<ChildListMutationRecord>(Krys::Move(target),
                                                      CreateRef<StaticNodeList>(Krys::Move(addedNodes)),
                                                      CreateRef<StaticNodeList>(Krys::Move(removedNodes)),
                                                      Krys::Move(previousSibling), Krys::Move(nextSibling));
          break;
        }
        default:
        {
          assert(false && "Unknown MutationRecordType");
          break;
        }
      }

      observer->_recordQueue.push_back(Krys::Move(record));
      // TODO(impl): MUTATION-OBSERVERS - Append observer to the surrounding agent’s pending mutation
      // observers.
    }

    QueueMutationObserverMicrotask();
  }

  void MutationObserverAlgorithms::QueueTreeMutationRecord(Ref<Node> &&target, SmallNodeList &&addedNodes,
                                                           SmallNodeList &&removedNodes,
                                                           RefPtr<Node> &&previousSibling,
                                                           RefPtr<Node> &&nextSibling) noexcept
  {
    assert(addedNodes.size() > 0 || removedNodes.size() > 0);
    QueueMutationRecord(MutationRecordType::ChildList, Krys::Move(target), DOMStringAtom::Null(),
                        DOMStringAtom::Null(), DOMStringAtom::Null(), Krys::Move(addedNodes),
                        Krys::Move(removedNodes), Krys::Move(previousSibling), Krys::Move(nextSibling));
  }
}
