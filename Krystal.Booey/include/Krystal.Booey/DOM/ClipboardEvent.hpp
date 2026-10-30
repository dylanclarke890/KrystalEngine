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
  class ClipboardEvent final : public Event
  {
    friend class EventFactory;

    using dictionary_type = ClipboardEventInit;

  private:
    RefPtr<DataTransfer> _clipboardData;

  public:
    /// @see https://w3c.github.io/clipboard-apis/#dom-clipboardevent-clipboardevent
    ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict = {}) noexcept;
    ~ClipboardEvent() noexcept;

    /// @brief A trusted clipboard event carrying @p dataTransfer, which bubbles, can be cancelled and crosses
    /// shadow boundaries.
    KRYS_NODISCARD static Ref<ClipboardEvent> Create(DOMStringAtom type,
                                                     Ref<DataTransfer> &&dataTransfer) noexcept;

    /// @see https://w3c.github.io/clipboard-apis/#dom-clipboardevent-clipboarddata
    KRYS_NODISCARD DataTransfer *ClipboardData() const noexcept
    {
      return _clipboardData.get();
    }

    KRYS_NODISCARD bool IsClipboardEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ClipboardEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsClipboardEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
