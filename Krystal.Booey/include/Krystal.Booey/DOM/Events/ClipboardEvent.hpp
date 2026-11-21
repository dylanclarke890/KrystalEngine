#pragma once

#include "Krystal.Booey/DOM/DataTransfer/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  /// @see https://w3c.github.io/clipboard-apis/#dictdef-clipboardeventinit
  struct ClipboardEventInit : EventInit
  {
    RefPtr<dom::DataTransfer> ClipboardData;
  };

  /// @brief A clipboard event: `copy`, `cut` and `paste`, and the legacy `beforecopy`, `beforecut` and
  /// `beforepaste`.
  /// @see https://w3c.github.io/clipboard-apis/#clipboard-event-interfaces
  class ClipboardEvent final : public Event
  {
  private:
    RefPtr<dom::DataTransfer> _clipboardData;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = ClipboardEventInit;

    /// @see https://w3c.github.io/clipboard-apis/#dom-clipboardevent-clipboardevent
    ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict = {}) noexcept;

    ~ClipboardEvent() noexcept override;

    /// @brief A trusted clipboard event carrying @p dataTransfer, which bubbles, can be cancelled and crosses
    /// shadow boundaries.
    KRYS_NODISCARD static Ref<ClipboardEvent> Create(const DOMStringAtom &type,
                                                     Ref<dom::DataTransfer> &&dataTransfer) noexcept;

    /// @see https://w3c.github.io/clipboard-apis/#dom-clipboardevent-clipboarddata
    KRYS_NODISCARD dom::DataTransfer *ClipboardData() const noexcept
    {
      return _clipboardData.get();
    }

    KRYS_NODISCARD bool IsClipboardEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ClipboardEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::Event &event) noexcept
  {
    return event.IsClipboardEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
