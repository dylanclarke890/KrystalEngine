#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class HTMLElement;

  /// @see https://html.spec.whatwg.org/#submiteventinit
  struct SubmitEventInit : dom::EventInit
  {
    RefPtr<HTMLElement> Submitter;
  };

  /// @brief The `submit` event, with the button that submitted the form.
  /// @see https://html.spec.whatwg.org/#submitevent
  class SubmitEvent final : public dom::Event
  {
    friend class dom::EventFactory;
    using dictionary_type = SubmitEventInit;

  private:
    RefPtr<HTMLElement> _submitter;

  public:
    /// @see https://html.spec.whatwg.org/#dom-submitevent-submitevent
    SubmitEvent(dom::DOMStringAtom type, const SubmitEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted `submit` event, which bubbles and can be cancelled.
    KRYS_NODISCARD static Ref<SubmitEvent> Create(RefPtr<HTMLElement> &&submitter) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-submitevent-submitter
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SubmitEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsSubmitEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
