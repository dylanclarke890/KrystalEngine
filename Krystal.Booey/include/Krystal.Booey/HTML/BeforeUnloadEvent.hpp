#pragma once

#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  /// @brief `beforeunload`: the document is about to be left. A listener that cancels it (or sets its
  /// `returnValue`) asks for the user to confirm leaving, if the user has interacted with the page.
  /// @see https://html.spec.whatwg.org/#beforeunloadevent
  class BeforeUnloadEvent final : public dom::Event
  {
  private:
    dom::DOMString _returnValue;

  public:
    BeforeUnloadEvent() noexcept;

    /// @brief A trusted, cancelable `beforeunload` event.
    KRYS_NODISCARD static Ref<BeforeUnloadEvent> Create() noexcept;

    /// @note Legacy: setting it to a non-empty string asks for confirmation, as cancelling does.
    /// @see https://html.spec.whatwg.org/#dom-beforeunloadevent-returnvalue
    KRYS_NODISCARD const dom::DOMString &ReturnValue() const noexcept
    {
      return _returnValue;
    }
    void SetReturnValue(dom::DOMString &&value) noexcept
    {
      _returnValue = krys::move(value);
    }

    KRYS_NODISCARD bool IsBeforeUnloadEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::BeforeUnloadEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsBeforeUnloadEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
