#pragma once

#include "Krystal.HTML/Events/EventListenerOptions.hpp"
#include "Krystal.Lib/Pointers/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class AbortSignal;

  struct AddEventListenerOptions : public EventListenerOptions
  {
    bool Once = false;
    Maybe<bool> Passive {};
    RefPtr<AbortSignal> Signal;
  };
}