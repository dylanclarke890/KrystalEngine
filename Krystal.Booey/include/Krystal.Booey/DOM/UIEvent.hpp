#pragma once

#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  /// @brief An event about the user interface: the base of focus, mouse and keyboard events.
  /// @see https://w3c.github.io/uievents/#interface-uievent
  // FIXME: The view, once there's a Window.
  class UIEvent : public Event
  {
    friend class EventFactory;

    using dictionary_type = UIEventInit;

  private:
    int32 _detail {0};

  public:
    UIEvent(DOMStringAtom type, const UIEventInit &eventInitDict = {}) noexcept
        : Event(type, eventInitDict), _detail(eventInitDict.Detail)
    {
    }

    /// @see https://w3c.github.io/uievents/#dom-uievent-detail
    KRYS_NODISCARD int32 Detail() const noexcept
    {
      return _detail;
    }
  };
}
