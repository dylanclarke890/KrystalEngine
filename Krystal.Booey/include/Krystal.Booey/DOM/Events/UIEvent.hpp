#pragma once

#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-uieventinit
  // FIXME: The view (a Window).
  struct UIEventInit : EventInit
  {
    int32 Detail {0};
  };

  /// @see https://w3c.github.io/uievents/#dictdef-eventmodifierinit
  // FIXME: The specification also has modifierFn, modifierFnLock, modifierHyper, modifierNumLock,
  // modifierScrollLock, modifierSuper, modifierSymbol and modifierSymbolLock.
  struct EventModifierInit : UIEventInit
  {
    bool CtrlKey {false};
    bool ShiftKey {false};
    bool AltKey {false};
    bool MetaKey {false};

    bool ModifierAltGraph {false};
    bool ModifierCapsLock {false};
  };

  /// @brief An event about the user interface: the base of focus, mouse and keyboard events.
  /// @see https://w3c.github.io/uievents/#interface-uievent
  // FIXME: The view (a Window).
  class UIEvent : public Event
  {
  private:
    int32 _detail {0};

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = UIEventInit;

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
