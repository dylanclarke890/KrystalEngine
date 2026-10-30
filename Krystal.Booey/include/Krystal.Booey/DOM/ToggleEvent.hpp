#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @see https://html.spec.whatwg.org/#toggleeventinit
  struct ToggleEventInit : EventInit
  {
    DOMString OldState;
    DOMString NewState;
    RefPtr<Element> Source;
  };

  /// @brief Fired at a dialog, popover or details element as it opens or closes (`toggle`), or before
  /// (`beforetoggle`), with its old and new states, `"open"` or `"closed"`.
  /// @see https://html.spec.whatwg.org/#the-toggleevent-interface
  class ToggleEvent final : public Event
  {
    friend class EventFactory;
    using dictionary_type = ToggleEventInit;

  private:
    DOMString _oldState;
    DOMString _newState;
    RefPtr<Element> _source;

  public:
    /// @see https://html.spec.whatwg.org/#dom-toggleevent-toggleevent
    ToggleEvent(DOMStringAtom type, const ToggleEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted event of type @p type, which doesn't bubble.
    KRYS_NODISCARD static Ref<ToggleEvent> Create(DOMStringAtom type, const ToggleEventInit &init,
                                                  bool isCancelable) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-toggleevent-oldstate
    KRYS_NODISCARD const DOMString &OldState() const noexcept
    {
      return _oldState;
    }

    /// @see https://html.spec.whatwg.org/#dom-toggleevent-newstate
    KRYS_NODISCARD const DOMString &NewState() const noexcept
    {
      return _newState;
    }

    /// @brief The element that toggled the target (a button), retargeted against the current target.
    /// @see https://html.spec.whatwg.org/#dom-toggleevent-source
    KRYS_NODISCARD Element *Source() const noexcept;

    KRYS_NODISCARD bool IsToggleEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ToggleEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsToggleEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
