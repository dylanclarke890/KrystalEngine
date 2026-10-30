#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  Ref<Settings> Settings::Create() noexcept
  {
    return AdoptRef(*new Settings);
  }

  Settings &Settings::Default() noexcept
  {
    static MainThreadNeverDestroyed<Ref<Settings>> settings(Create());
    return *settings.get();
  }
}
