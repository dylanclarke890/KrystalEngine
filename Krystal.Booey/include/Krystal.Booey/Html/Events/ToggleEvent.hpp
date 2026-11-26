#pragma once

#include "Krystal.Booey/Dom/Events/Event.hpp"
#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class Element;

  /// @see https://html.spec.whatwg.org/multipage/interaction.html#toggleeventinit
  struct ToggleEventInit : EventInit
  {
    DOMString OldState;
    DOMString NewState;
    /// @note A plain pointer: the event takes its own reference.
    Element *Source {nullptr};
  };

  /// @brief Fired at a dialog, popover or details element as it opens or closes (`toggle`), or before
  /// (`beforetoggle`), with its old and new states, `"open"` or `"closed"`.
  /// @see https://html.spec.whatwg.org/multipage/interaction.html#the-toggleevent-interface
  class ToggleEvent final : public Event
  {
  private:
    DOMString _oldState;
    DOMString _newState;
    RefPtr<Element> _source;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = ToggleEventInit;

    /// @see https://html.spec.whatwg.org/#dom-toggleevent-toggleevent
    ToggleEvent(DOMStringAtom type, const ToggleEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted event of type @p type, which doesn't bubble; composed if @p init says so.
    KRYS_NODISCARD static Ref<ToggleEvent> Create(const DOMStringAtom &type, const ToggleEventInit &init,
                                                  bool isCancelable) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-toggleevent-oldstate
    KRYS_NODISCARD const DOMString &OldState() const noexcept
    {
      return _oldState;
    }

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-toggleevent-newstate
    KRYS_NODISCARD const DOMString &NewState() const noexcept
    {
      return _newState;
    }

    /// @brief The element that toggled the target (a button), retargeted against the current target.
    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-toggleevent-source
    KRYS_NODISCARD Element *Source() const noexcept;

    KRYS_NODISCARD bool IsToggleEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToggleEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::Event &event) noexcept
  {
    return event.IsToggleEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
