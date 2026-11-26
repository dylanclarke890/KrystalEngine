#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Html/DragAndDrop/DataTransfer.hpp"

namespace krys::web
{
  /// @see https://w3c.github.io/clipboard-apis/#dictdef-clipboardeventinit
  struct ClipboardEventInit : EventInit
  {
    RefPtr<DataTransfer> ClipboardData;
  };

  /// @brief A clipboard event: `copy`, `cut` and `paste`, and the legacy `beforecopy`, `beforecut` and
  /// `beforepaste`.
  /// @see https://w3c.github.io/clipboard-apis/#clipboard-event-interfaces
  class ClipboardEvent final : public Event
  {
  private:
    RefPtr<DataTransfer> _clipboardData;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = ClipboardEventInit;

    /// @see https://w3c.github.io/clipboard-apis/#dom-clipboardevent-clipboardevent
    ClipboardEvent(DOMStringAtom type, const ClipboardEventInit &eventInitDict = {}) noexcept;

    ~ClipboardEvent() noexcept override;

    /// @brief A trusted clipboard event carrying @p dataTransfer, which bubbles, can be cancelled and crosses
    /// shadow boundaries.
    KRYS_NODISCARD static Ref<ClipboardEvent> Create(const DOMStringAtom &type,
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::ClipboardEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsClipboardEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
