#include "Krystal.Web/Css/Values/SerialisationContext.hpp"

namespace krys::web::css
{
  const SerialisationContext &DefaultSerialisationContext() noexcept
  {
    constexpr static SerialisationContext context;
    return context;
  }
}
