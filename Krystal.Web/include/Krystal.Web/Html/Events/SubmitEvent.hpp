#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"

namespace krys::web
{
  class HTMLElement;

  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#submiteventinit
  struct SubmitEventInit : EventInit
  {
    HTMLElement *Submitter {nullptr};
  };

  /// @brief The `submit` event, with the button that submitted the form.
  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#submitevent
  class SubmitEvent final : public Event
  {
  private:
    RefPtr<HTMLElement> _submitter;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = SubmitEventInit;

    /// @see https://html.spec.whatwg.org/#dom-submitevent-submitevent
    SubmitEvent(DOMStringAtom type, const SubmitEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted `submit` event, which bubbles and can be cancelled.
    KRYS_NODISCARD static Ref<SubmitEvent> Create(RefPtr<HTMLElement> &&submitter) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-submitevent-submitter
    KRYS_NODISCARD HTMLElement *Submitter() const noexcept
    {
      return _submitter.get();
    }

    KRYS_NODISCARD bool IsSubmitEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SubmitEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsSubmitEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
