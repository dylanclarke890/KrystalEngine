#pragma once

#include "Krystal.Booey/DOM/Dicts/ClipboardEventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A clipboard event: `copy`, `cut` and `paste`, and the legacy `beforecopy`, `beforecut` and
  /// `beforepaste`.
  /// @see https://w3c.github.io/clipboard-apis/#clipboard-event-interfaces
  // FIXME: The clipboard data (clipboardData) the event's listeners read and write, with DataTransfer.
  class ClipboardEvent final : public Event
  {
    friend class EventFactory;

    using dictionary_type = ClipboardEventInit;

  public:
    /// @see https://w3c.github.io/clipboard-apis/#dom-clipboardevent-clipboardevent
    ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted clipboard event, which bubbles, can be cancelled and crosses shadow boundaries.
    KRYS_NODISCARD static Ref<ClipboardEvent> Create(DOMStringAtom type) noexcept;

    KRYS_NODISCARD bool IsClipboardEvent() const noexcept final
    {
      return true;
    }
  };
}
