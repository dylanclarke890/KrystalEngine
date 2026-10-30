#include "Krystal.Booey/Platform/PlatformStrategies.hpp"

namespace krys::boo
{
  PlatformStrategies &PlatformStrategies::Singleton() noexcept
  {
    static PlatformStrategies strategies;
    return strategies;
  }
}
