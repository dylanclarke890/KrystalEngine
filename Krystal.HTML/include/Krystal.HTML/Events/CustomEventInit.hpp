#pragma once

#include "Krystal.HTML/Events/EventInit.hpp"

namespace Krys::HTML
{
  template <typename T>
  struct CustomEventInit : public EventInit
  {
    T Detail;
  };
}