#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"

namespace krys::web
{
  class Window;

  /// @see https://w3c.github.io/uievents/#dictdef-uieventinit
  struct UIEventInit : EventInit
  {
    Window *View {nullptr};
    int32 Detail {0};
  };

  /// @see https://w3c.github.io/uievents/#dictdef-eventmodifierinit
  // FIXME: The specification also has modifierFn, modifierFnLock, modifierHyper, modifierNumLock,
  // modifierScrollLock, modifierSuper, modifierSymbol and modifierSymbolLock.
  struct EventModifierInit : UIEventInit
  {
    bool CtrlKey {false};
    bool ShiftKey {false};
    bool AltKey {false};
    bool MetaKey {false};

    bool ModifierAltGraph {false};
    bool ModifierCapsLock {false};
  };

  /// @brief An event about the user interface: the base of focus, mouse and keyboard events.
  /// @see https://w3c.github.io/uievents/#interface-uievent
  class UIEvent : public Event
  {
  private:
    RefPtr<Window> _view;
    int32 _detail {0};

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = UIEventInit;

    UIEvent(DOMStringAtom type, const UIEventInit &eventInitDict = {}) noexcept;

    ~UIEvent() noexcept override;

    /// @see https://w3c.github.io/uievents/#dom-uievent-view
    KRYS_NODISCARD Window *View() const noexcept;

    /// @see https://w3c.github.io/uievents/#dom-uievent-detail
    KRYS_NODISCARD int32 Detail() const noexcept
    {
      return _detail;
    }

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-uievent-inituievent
    void InitUIEvent(DOMStringAtom type, bool bubbles = false, bool cancelable = false,
                     Window *view = nullptr, int32 detail = 0) noexcept;

    KRYS_NODISCARD bool IsUIEvent() const noexcept final
    {
      return true;
    }

  protected:
    void SetView(Window *view) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::UIEvent)
  KRYS_NODISCARD static bool IsType(const krys::web::Event &event) noexcept
  {
    return event.IsUIEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
