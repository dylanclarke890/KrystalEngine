#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Element;
  class IdTargetObserver;

  /// @brief The observers of a tree scope's IDs, told when the element with an ID changes.
  class IdTargetObserverRegistry final
  {
    friend class IdTargetObserver;
    KRYS_NON_COPY_MOVABLE(IdTargetObserverRegistry);

  private:
    struct ObserverSet final
    {
      HashSet<IdTargetObserver *> Observers;
    };

    using IdToObserverSetMap = HashMap<DOMStringAtom, UniquePtr<ObserverSet>>;

    IdToObserverSetMap _registry;
    ObserverSet *_notifyingObserversInSet {};

  public:
    IdTargetObserverRegistry() noexcept = default;
    ~IdTargetObserverRegistry() noexcept = default;

    /// @brief Tells the observers of @p id that @p element gained or lost it.
    void NotifyObservers(Element &element, DOMStringAtom id) noexcept;

  private:
    void AddObserver(DOMStringAtom id, IdTargetObserver &observer) noexcept;
    void RemoveObserver(DOMStringAtom id, IdTargetObserver &observer) noexcept;
    void NotifyObserversInternal(Element &element, DOMStringAtom id) noexcept;
  };

  inline void IdTargetObserverRegistry::NotifyObservers(Element &element, DOMStringAtom id) noexcept
  {
    krys_debug_assert(!id.View().empty());
    krys_debug_assert(!_notifyingObserversInSet);
    if (_registry.empty())
    {
      return;
    }
    NotifyObserversInternal(element, id);
  }
}
