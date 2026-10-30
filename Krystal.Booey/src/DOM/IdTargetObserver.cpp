#include "Krystal.Booey/DOM/IdTargetObserver.hpp"
#include "Krystal.Booey/DOM/IdTargetObserverRegistry.hpp"

namespace krys::boo::dom
{
  IdTargetObserver::IdTargetObserver(IdTargetObserverRegistry &registry, DOMStringAtom id) noexcept
      : _registry(&registry), _id(id)
  {
    _registry->AddObserver(_id, *this);
  }

  IdTargetObserver::~IdTargetObserver() noexcept
  {
    if (_registry)
    {
      _registry->RemoveObserver(_id, *this);
    }
  }
}
