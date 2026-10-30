#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @see https://html.spec.whatwg.org/#commandeventinit
  struct CommandEventInit : EventInit
  {
    RefPtr<Element> Source;
    DOMString Command;
  };

  /// @brief Fired at a button's `commandfor` element with the button's command, before the element carries
  /// out a built-in one; canceling it stops that.
  /// @see https://html.spec.whatwg.org/#the-commandevent-interface
  class CommandEvent final : public Event
  {
    friend class EventFactory;
    using dictionary_type = CommandEventInit;

  private:
    RefPtr<Element> _source;
    DOMString _command;

  public:
    /// @see https://html.spec.whatwg.org/#dom-commandevent-commandevent
    CommandEvent(DOMStringAtom type, const CommandEventInit &eventInitDict = {}) noexcept;

    /// @brief An event of type @p type; trusted if @p isTrusted.
    KRYS_NODISCARD static Ref<CommandEvent> Create(DOMStringAtom type, const CommandEventInit &init,
                                                   bool isTrusted = false) noexcept;

    /// @brief The button that invoked the command, retargeted against the current target.
    /// @see https://html.spec.whatwg.org/#dom-commandevent-source
    KRYS_NODISCARD Element *Source() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-commandevent-command
    KRYS_NODISCARD const DOMString &Command() const noexcept
    {
      return _command;
    }

    KRYS_NODISCARD bool IsCommandEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::CommandEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsCommandEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
