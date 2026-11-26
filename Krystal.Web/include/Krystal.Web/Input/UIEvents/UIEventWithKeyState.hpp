#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Web/Input/UIEvents/UIEvent.hpp"
#include "Krystal.Web/Platform/Events/PlatformEvent.hpp"

namespace krys::web
{
  /// @brief The base of the events that carry the state of the modifier keys: mouse (and so wheel and
  /// pointer) events and keyboard events.
  /// @see https://w3c.github.io/uievents/#dictdef-eventmodifierinit
  class UIEventWithKeyState : public UIEvent
  {
  public:
    using Modifier = PlatformEvent::Modifier;

  private:
    EnumSet<Modifier> _modifiers;

  public:
    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-ctrlkey
    KRYS_NODISCARD bool CtrlKey() const noexcept
    {
      return _modifiers.contains(Modifier::ControlKey);
    }

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-shiftkey
    KRYS_NODISCARD bool ShiftKey() const noexcept
    {
      return _modifiers.contains(Modifier::ShiftKey);
    }

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-altkey
    KRYS_NODISCARD bool AltKey() const noexcept
    {
      return _modifiers.contains(Modifier::AltKey);
    }

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-metakey
    KRYS_NODISCARD bool MetaKey() const noexcept
    {
      return _modifiers.contains(Modifier::MetaKey);
    }

    /// @brief Whether caps lock was on; the same as `GetModifierState("CapsLock")`.
    KRYS_NODISCARD bool CapsLockKey() const noexcept
    {
      return _modifiers.contains(Modifier::CapsLockKey);
    }

    /// @brief Every modifier the event carries.
    KRYS_NODISCARD EnumSet<Modifier> ModifierKeys() const noexcept
    {
      return _modifiers;
    }

    /// @brief Whether the modifier named by the key value @p key was active.
    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-getmodifierstate
    KRYS_NODISCARD bool GetModifierState(DOMStringView key) const noexcept
    {
      struct NamedModifier
      {
        DOMStringView Key;
        Modifier Value;
      };

      constexpr static NamedModifier modifiers[] {
        {u8"Alt", Modifier::AltKey},           {u8"AltGraph", Modifier::AltGraphKey},
        {u8"CapsLock", Modifier::CapsLockKey}, {u8"Control", Modifier::ControlKey},
        {u8"Meta", Modifier::MetaKey},         {u8"Shift", Modifier::ShiftKey},
      };
      for (const auto &modifier : modifiers)
      {
        if (modifier.Key == key)
        {
          return _modifiers.contains(modifier.Value);
        }
      }
      // FIXME: The specification also has Fn, FnLock, Hyper, NumLock, Super, ScrollLock, Symbol, SymbolLock.
      return false;
    }

  protected:
    /// @brief An event with the modifiers of a platform event.
    UIEventWithKeyState(DOMStringAtom type, const UIEventInit &eventInitDict,
                        EnumSet<Modifier> modifiers) noexcept
        : UIEvent(krys::move(type), eventInitDict), _modifiers(modifiers)
    {
    }

    /// @brief An event with the modifiers of an `EventModifierInit`.
    UIEventWithKeyState(DOMStringAtom type, const EventModifierInit &initializer) noexcept
        : UIEvent(krys::move(type), initializer),
          _modifiers(
            KeyModifiers(initializer.CtrlKey, initializer.AltKey, initializer.ShiftKey, initializer.MetaKey))
    {
      if (initializer.ModifierAltGraph)
      {
        _modifiers.Add(Modifier::AltGraphKey);
      }
      if (initializer.ModifierCapsLock)
      {
        _modifiers.Add(Modifier::CapsLockKey);
      }
    }

    /// @brief Replaces the modifiers with the four that the legacy `init*Event` methods take.
    void SetModifierKeys(bool ctrlKey, bool altKey, bool shiftKey, bool metaKey) noexcept
    {
      _modifiers = KeyModifiers(ctrlKey, altKey, shiftKey, metaKey);
    }

  private:
    KRYS_NODISCARD static EnumSet<Modifier> KeyModifiers(bool ctrlKey, bool altKey, bool shiftKey,
                                                         bool metaKey) noexcept
    {
      EnumSet<Modifier> modifiers;
      if (ctrlKey)
      {
        modifiers.Add(Modifier::ControlKey);
      }
      if (altKey)
      {
        modifiers.Add(Modifier::AltKey);
      }
      if (shiftKey)
      {
        modifiers.Add(Modifier::ShiftKey);
      }
      if (metaKey)
      {
        modifiers.Add(Modifier::MetaKey);
      }
      return modifiers;
    }
  };
}

namespace krys::web::ui_events
{
  /// @brief The nearest event, starting from @p event and following underlying events, that carries modifier
  /// keys (a keyboard or mouse event): a click simulated for a key press finds the key press. Null if there
  /// is none.
  KRYS_NODISCARD inline UIEventWithKeyState *FindEventWithKeyState(Event *event) noexcept
  {
    for (auto *current = event; current; current = current->UnderlyingEvent())
    {
      if (current->IsKeyboardEvent() || current->IsMouseEvent())
      {
        return static_cast<UIEventWithKeyState *>(current);
      }
    }
    return nullptr;
  }
}
