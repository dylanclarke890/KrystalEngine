#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css
{
  const SerialisationContext &DefaultSerialisationContext() noexcept
  {
    constexpr static SerialisationContext context;
    return context;
  }
}
