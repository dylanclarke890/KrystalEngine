#pragma once

#include "Krystal.Booey/DOM/Dicts/UIEventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  /// @brief An event about the user interface: the base of focus, mouse and keyboard events.
  /// @see https://w3c.github.io/uievents/#interface-uievent
  // FIXME: The view (a Window).
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

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-uievent-inituievent
    // FIXME: The view (a Window).
    void InitUIEvent(DOMStringAtom type, bool bubbles = false, bool cancelable = false,
                     int32 detail = 0) noexcept
    {
      if (_dispatched)
      {
        return;
      }

      InitEvent(type, bubbles, cancelable);
      _detail = detail;
    }
  };
}
