#pragma once

#include "Krystal.HTML/Events/EventInit.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  template <typename T>
  struct CustomEventInit : public EventInit
  {
    Maybe<T> Detail;
  };
}