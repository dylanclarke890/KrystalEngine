#pragma once

#include "Krystal.Booey/DOM/Dicts/CompositionEventInit.hpp"
#include "Krystal.Booey/DOM/UIEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief An event about composing text with an input method: `compositionstart`, `compositionupdate` and
  /// `compositionend`.
  /// @see https://w3c.github.io/uievents/#interface-compositionevent
  class CompositionEvent final : public UIEvent
  {
    friend class EventFactory;
    using dictionary_type = CompositionEventInit;

  private:
    DOMString _data;

  public:
    /// @see https://w3c.github.io/uievents/#dom-compositionevent-compositionevent
    CompositionEvent(DOMStringAtom type, const CompositionEventInit &initializer = {}) noexcept;
    ~CompositionEvent() noexcept override;

    /// @brief A trusted composition event with @p data, which bubbles, can be cancelled and crosses shadow
    /// boundaries.
    KRYS_NODISCARD static Ref<CompositionEvent> Create(DOMStringAtom type, DOMStringView data) noexcept;

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-compositionevent-initcompositionevent
    void InitCompositionEvent(DOMStringAtom type, bool canBubble = false, bool cancelable = false,
                              DOMStringView data = {}) noexcept;

    /// @see https://w3c.github.io/uievents/#dom-compositionevent-data
    KRYS_NODISCARD const DOMString &Data() const noexcept
    {
      return _data;
    }

    KRYS_NODISCARD bool IsCompositionEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::CompositionEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsCompositionEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
