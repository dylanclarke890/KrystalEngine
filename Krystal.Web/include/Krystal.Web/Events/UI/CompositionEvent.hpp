#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Events/UI/UIEvent.hpp"

namespace krys::web
{
  /// @see https://w3c.github.io/uievents/#dictdef-compositioneventinit
  struct CompositionEventInit : UIEventInit
  {
    DOMString Data;
  };

  /// @brief An event about composing text with an input method: `compositionstart`, `compositionupdate` and
  /// `compositionend`.
  /// @see https://w3c.github.io/uievents/#interface-compositionevent
  class CompositionEvent final : public UIEvent
  {
  private:
    DOMString _data;

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = CompositionEventInit;

    /// @see https://w3c.github.io/uievents/#dom-compositionevent-compositionevent
    CompositionEvent(DOMStringAtom type, const CompositionEventInit &initializer = {}) noexcept;

    ~CompositionEvent() noexcept override;

    /// @brief A trusted composition event with @p data, which bubbles, can be cancelled and crosses shadow
    /// boundaries.
    KRYS_NODISCARD static Ref<CompositionEvent> Create(const DOMStringAtom &type, Window *view,
                                                       DOMStringView data) noexcept;

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-compositionevent-initcompositionevent
    void InitCompositionEvent(DOMStringAtom type, bool canBubble = false, bool cancelable = false,
                              Window *view = nullptr, DOMStringView data = {}) noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::CompositionEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsCompositionEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
