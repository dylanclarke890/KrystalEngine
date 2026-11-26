#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class Element;

  /// @see https://html.spec.whatwg.org/multipage/interaction.html#commandeventinit
  struct CommandEventInit : EventInit
  {
    /// @note A plain pointer: the event takes its own reference.
    Element *Source {nullptr};
    DOMString Command;
  };

  /// @brief Fired at a button's `commandfor` element with the button's command, before the element carries
  /// out a built-in one; canceling it stops that.
  /// @see https://html.spec.whatwg.org/multipage/interaction.html#the-commandevent-interface
  class CommandEvent final : public Event
  {
  private:
    RefPtr<Element> _source;
    DOMString _command;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = CommandEventInit;

    /// @see https://html.spec.whatwg.org/#dom-commandevent-commandevent
    CommandEvent(DOMStringAtom type, const CommandEventInit &eventInitDict = {}) noexcept;

    /// @brief An event of type @p type; trusted if @p isTrusted.
    KRYS_NODISCARD static Ref<CommandEvent> Create(const DOMStringAtom &type, const CommandEventInit &init,
                                                   bool isTrusted = false) noexcept;

    /// @brief The button that invoked the command, retargeted against the current target.
    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-commandevent-source
    KRYS_NODISCARD Element *Source() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-commandevent-command
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CommandEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsCommandEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
