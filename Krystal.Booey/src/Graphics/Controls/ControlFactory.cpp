#include "Krystal.Booey/Graphics/Controls/ControlFactory.hpp"

namespace krys::boo
{
  ControlFactory &ControlFactory::Singleton() noexcept
  {
    static Ref<ControlFactory> shared = Create();
    return *shared;
  }
}
