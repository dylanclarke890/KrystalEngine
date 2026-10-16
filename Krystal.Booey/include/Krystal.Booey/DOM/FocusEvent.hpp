#pragma once

#include "Krystal.Booey/DOM/Dicts/FocusEventInit.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/UIEvent.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html
{
  class FocusAlgorithms;
}

namespace krys::boo::dom
{
  /// @brief A focus event: `focus`, `blur`, `focusin` and `focusout`.
  /// @see https://w3c.github.io/uievents/#interface-focusevent
  class FocusEvent : public UIEvent
  {
    friend class EventFactory;
    friend class html::FocusAlgorithms;

    using dictionary_type = FocusEventInit;

  private:
    RefPtr<EventTarget> _relatedTarget;

  public:
    FocusEvent(DOMStringAtom type, const FocusEventInit &eventInitDict = {}) noexcept
        : UIEvent(type, eventInitDict), _relatedTarget(eventInitDict.RelatedTarget)
    {
    }

    /// @brief The element losing focus for `focus` and `focusin`, or gaining it for `blur` and `focusout`.
    /// @see https://w3c.github.io/uievents/#dom-focusevent-relatedtarget
    KRYS_NODISCARD RefPtr<EventTarget> RelatedTarget() const noexcept override
    {
      return _relatedTarget;
    }

  protected:
    void RelatedTarget(EventTarget *relatedTarget) noexcept override
    {
      _relatedTarget = ShareRefPtr(relatedTarget);
    }
  };
}
