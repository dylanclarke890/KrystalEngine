#include "Krystal.Booey/DOM/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/IdTargetObserver.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  void IdTargetObserverRegistry::AddObserver(DOMStringAtom id, IdTargetObserver &observer) noexcept
  {
    if (id.View().empty())
    {
      return;
    }

    auto &set = _registry[id];
    if (!set)
    {
      set = CreateUnique<ObserverSet>();
    }
    set->Observers.insert(&observer);
  }

  void IdTargetObserverRegistry::RemoveObserver(DOMStringAtom id, IdTargetObserver &observer) noexcept
  {
    if (id.View().empty() || _registry.empty())
    {
      return;
    }

    auto iterator = _registry.find(id);
    krys_debug_assert(iterator != _registry.end());
    auto *set = iterator->second.get();
    set->Observers.erase(&observer);
    if (set->Observers.empty() && set != _notifyingObserversInSet)
    {
      _registry.erase(iterator);
    }
  }

  void IdTargetObserverRegistry::NotifyObserversInternal(Element &element, DOMStringAtom id) noexcept
  {
    krys_debug_assert(!_registry.empty());

    auto iterator = _registry.find(id);
    if (iterator == _registry.end())
    {
      return;
    }
    _notifyingObserversInSet = iterator->second.get();

    List<IdTargetObserver *> observers(_notifyingObserversInSet->Observers.begin(),
                                       _notifyingObserversInSet->Observers.end());
    for (auto *observer : observers)
    {
      if (_notifyingObserversInSet->Observers.contains(observer))
      {
        observer->IdTargetChanged(element);
      }
    }

    bool hasRemainingObservers = !_notifyingObserversInSet->Observers.empty();
    _notifyingObserversInSet = nullptr;

    if (!hasRemainingObservers)
    {
      _registry.erase(id);
    }
  }
}
