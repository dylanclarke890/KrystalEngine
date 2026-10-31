#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Internals/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::dom
{
  MutationObserver::MutationObserver(MutationCallback &&callback) noexcept
      // 1. Set this's callback to callback.
      : _callback(krys::move(callback)),
        // 2. Set this's creation index to this's relevant agent's mutation observer counter.
        // 3. Increase this's relevant agent's mutation observer counter by 1.
        _creationIndex(MutationObserverAlgorithms::TakeMutationObserverCounter())
  {
  }

  ExceptionOr<void> MutationObserver::Observe(Node &target, const MutationObserverInit &initOptions) noexcept
  {
    MutationObserverInit options = initOptions;

    // 1. If either options["attributeOldValue"] or options["attributeFilter"] exists, and
    // options["attributes"]
    //    does not exist, then set options["attributes"] to true.
    if ((options.AttributeOldValue.has_value() || options.AttributeFilter.has_value())
        && !options.Attributes.has_value())
    {
      options.Attributes = true;
    }

    // 2. If options["characterDataOldValue"] exists and options["characterData"] does not exist, then set
    //    options["characterData"] to true.
    if (options.CharacterDataOldValue.has_value() && !options.CharacterData.has_value())
    {
      options.CharacterData = true;
    }

    // 3. If none of options["childList"], options["attributes"], and options["characterData"] is true, then
    // throw
    //    a TypeError.
    if (!options.ChildList && !options.Attributes.value_or(false) && !options.CharacterData.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    // 4. If options["attributeOldValue"] is true and options["attributes"] is false, then throw a TypeError.
    if (options.AttributeOldValue == true && !options.Attributes.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    // 5. If options["attributeFilter"] exists and options["attributes"] is false, then throw a TypeError.
    if (options.AttributeFilter.has_value() && !options.Attributes.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    // 6. If options["characterDataOldValue"] is true and options["characterData"] is false, then throw a
    //    TypeError.
    if (options.CharacterDataOldValue == true && !options.CharacterData.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    // 7. Let registered be the registered observer in target's registered observer list that is not a
    // transient
    //    registered observer and whose observer is this, if any; otherwise null.
    for (auto &registered : target.RegisteredObservers())
    {
      // 8. If registered is non-null:
      if (registered->Observer() == this)
      {
        // 8a. For each node of this's transient node set: remove all transient registered observers whose
        //     source is registered from node's registered observer list.
        for (auto &n : _transientNodes)
        {
          if (auto node = n.lock())
          {
            std::erase_if(node->TransientRegisteredObservers(), [source = registered.get()](auto &observer)
                          { return observer->Source() == source; });
          }
        }

        // 8b. Set registered's options to options.
        registered->Options(options);
        return {};
      }
    }

    // 9. Otherwise:
    // 9a. Append a new registered observer whose observer is this and options is options to target's
    // registered
    //     observer list.
    auto registered = CreateRef<RegisteredObserver>(ShareRef(*this), options);
    target.RegisteredObservers().emplace_back(krys::move(registered));

    // 9b. Append a weak reference to target to this's node list.
    _nodes.emplace_back(CreateWeakRef(target));

    return {};
  }

  void MutationObserver::Disconnect() noexcept
  {
    // 1. For each node of this's node list: remove any registered observer from node's registered observer
    // list
    //    for which this is the observer.
    for (auto &n : _nodes)
    {
      if (auto node = n.lock())
      {
        std::erase_if(node->RegisteredObservers(), [self = this](Ref<RegisteredObserver> &observer)
                      { return observer->Observer() == self; });
      }
    }

    // 2. Remove transient registered observers for this.
    MutationObserverAlgorithms::RemoveTransientRegisteredObservers(*this);

    // 3. Empty this's node list.
    _nodes.clear();

    // 4. Empty this's record queue.
    _recordQueue.clear();
  }

  List<Ref<MutationRecord>> MutationObserver::TakeRecords() noexcept
  {
    // 1. Let records be a clone of this's record queue.
    List<Ref<MutationRecord>> records = _recordQueue;

    // 2. Empty this's record queue.
    _recordQueue.clear();

    // 3. Return records.
    return records;
  }
}