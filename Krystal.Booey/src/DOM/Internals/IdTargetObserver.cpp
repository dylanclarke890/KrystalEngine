#include "Krystal.Booey/DOM/Internals/IdTargetObserver.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"

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
