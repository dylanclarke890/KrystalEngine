#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Events/EventTarget.hpp"
#include "Krystal.Web/Events/UI/UIEvent.hpp"

namespace krys::web
{
  /// @see https://w3c.github.io/uievents/#dictdef-focuseventinit
  struct FocusEventInit : UIEventInit
  {
    RefPtr<EventTarget> RelatedTarget;
  };

  /// @brief A focus event: `focus`, `blur`, `focusin` and `focusout`.
  /// @see https://w3c.github.io/uievents/#interface-focusevent
  class FocusEvent : public UIEvent
  {
  private:
    RefPtr<EventTarget> _relatedTarget;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = FocusEventInit;

    FocusEvent(DOMStringAtom type, const FocusEventInit &eventInitDict = {}) noexcept
        : UIEvent(krys::move(type), eventInitDict), _relatedTarget(eventInitDict.RelatedTarget)
    {
    }

    /// @brief The element losing focus for `focus` and `focusin`, or gaining it for `blur` and `focusout`.
    /// @see https://w3c.github.io/uievents/#dom-focusevent-relatedtarget
    KRYS_NODISCARD RefPtr<EventTarget> RelatedTarget() const noexcept override
    {
      return _relatedTarget;
    }

    KRYS_NODISCARD bool IsFocusEvent() const noexcept final
    {
      return true;
    }

  protected:
    void RelatedTarget(EventTarget *relatedTarget) noexcept override
    {
      _relatedTarget = ShareRefPtr(relatedTarget);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::FocusEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsFocusEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
