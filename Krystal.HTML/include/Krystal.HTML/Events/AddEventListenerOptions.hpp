#pragma once

#include "Krystal.HTML/Events/EventListenerOptions.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  struct AddEventListenerOptions : public EventListenerOptions
  {
    bool Once = false;
    bool Passive = false;
    RefPtr<AbortSignal> Signal;
  };
}