#pragma once

#include "Krystal.HTML/Events/CustomEventInit.hpp"
#include "Krystal.HTML/Events/Event.hpp"

namespace Krys::HTML
{
  template <typename T>
  class CustomEvent : public Event
  {
  private:
    T _detail;

  public:
    CustomEvent(const CustomEventInit<T> &eventInitDict = {}, StringAtom type = {}) noexcept
        : Event(type, eventInitDict), _detail(eventInitDict.Detail)
    {
    }
  };
}