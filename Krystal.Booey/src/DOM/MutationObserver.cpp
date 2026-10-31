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

    if ((options.AttributeOldValue == true || options.AttributeFilter.has_value())
        && !options.Attributes.has_value())
    {
      options.Attributes = true;
    }

    if (options.CharacterDataOldValue == true && !options.CharacterData.has_value())
    {
      options.CharacterData = true;
    }

    if (!options.ChildList && !options.Attributes.value_or(false) && !options.CharacterData.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.AttributeOldValue == true && !options.Attributes.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.AttributeFilter.has_value() && !options.Attributes.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.CharacterDataOldValue == true && !options.CharacterData.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    for (auto &registered : target.RegisteredObservers())
    {
      if (registered->Observer() == this)
      {
        for (auto &n : _transientNodes)
        {
          if (auto node = n.lock())
          {
            std::erase_if(node->TransientRegisteredObservers(), [source = registered.get()](auto &observer)
                          { return observer->Source() == source; });
          }
        }

        registered->Options(options);
        return {};
      }
    }

    auto registered = CreateRef<RegisteredObserver>(ShareRef(*this), options);
    target.RegisteredObservers().emplace_back(krys::move(registered));
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
    List<Ref<MutationRecord>> records = _recordQueue;
    _recordQueue.clear();
    return records;
  }
}