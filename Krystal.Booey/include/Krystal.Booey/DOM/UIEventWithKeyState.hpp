#pragma once

#include "Krystal.Booey/DOM/Dicts/EventModifierInit.hpp"
#include "Krystal.Booey/DOM/UIEvent.hpp"
#include "Krystal.Booey/Platform/PlatformEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::dom
{
  /// @brief A user interface event that knows which modifier keys were held: the base of mouse and keyboard
  /// events.
  class UIEventWithKeyState : public UIEvent
  {
  public:
    using Modifier = PlatformEvent::Modifier;

  private:
    EnumSet<Modifier> _modifiers;

  public:
    /// @see https://w3c.github.io/uievents/#dom-mouseevent-ctrlkey
    KRYS_NODISCARD bool CtrlKey() const noexcept
    {
      return _modifiers.Contains(Modifier::ControlKey);
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-shiftkey
    KRYS_NODISCARD bool ShiftKey() const noexcept
    {
      return _modifiers.Contains(Modifier::ShiftKey);
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-altkey
    KRYS_NODISCARD bool AltKey() const noexcept
    {
      return _modifiers.Contains(Modifier::AltKey);
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-metakey
    KRYS_NODISCARD bool MetaKey() const noexcept
    {
      return _modifiers.Contains(Modifier::MetaKey);
    }

    KRYS_NODISCARD bool CapsLockKey() const noexcept
    {
      return _modifiers.Contains(Modifier::CapsLockKey);
    }

    KRYS_NODISCARD EnumSet<Modifier> ModifierKeys() const noexcept
    {
      return _modifiers;
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-getmodifierstate
    KRYS_NODISCARD bool GetModifierState(DOMStringView keyIdentifier) const noexcept
    {
      if (keyIdentifier == u8"Control")
      {
        return CtrlKey();
      }
      if (keyIdentifier == u8"Shift")
      {
        return ShiftKey();
      }
      if (keyIdentifier == u8"Alt")
      {
        return AltKey();
      }
      if (keyIdentifier == u8"Meta")
      {
        return MetaKey();
      }
      if (keyIdentifier == u8"AltGraph")
      {
        return _modifiers.Contains(Modifier::AltGraphKey);
      }
      if (keyIdentifier == u8"CapsLock")
      {
        return CapsLockKey();
      }
      // FIXME: The specification also has Fn, FnLock, Hyper, NumLock, Super, ScrollLock, Symbol, SymbolLock.
      return false;
    }

  protected:
    UIEventWithKeyState(DOMStringAtom type, const UIEventInit &eventInitDict,
                        EnumSet<Modifier> modifiers) noexcept
        : UIEvent(type, eventInitDict), _modifiers(modifiers)
    {
    }

    UIEventWithKeyState(DOMStringAtom type, const EventModifierInit &initializer) noexcept
        : UIEvent(type, initializer), _modifiers(ModifiersFromInitializer(initializer))
    {
    }

    void SetModifierKeys(bool ctrlKey, bool altKey, bool shiftKey, bool metaKey) noexcept
    {
      EnumSet<Modifier> result;
      if (ctrlKey)
      {
        result.Add(Modifier::ControlKey);
      }
      if (altKey)
      {
        result.Add(Modifier::AltKey);
      }
      if (shiftKey)
      {
        result.Add(Modifier::ShiftKey);
      }
      if (metaKey)
      {
        result.Add(Modifier::MetaKey);
      }
      _modifiers = result;
    }

  private:
    KRYS_NODISCARD static EnumSet<Modifier>
      ModifiersFromInitializer(const EventModifierInit &initializer) noexcept
    {
      EnumSet<Modifier> result;
      if (initializer.CtrlKey)
      {
        result.Add(Modifier::ControlKey);
      }
      if (initializer.AltKey)
      {
        result.Add(Modifier::AltKey);
      }
      if (initializer.ShiftKey)
      {
        result.Add(Modifier::ShiftKey);
      }
      if (initializer.MetaKey)
      {
        result.Add(Modifier::MetaKey);
      }
      if (initializer.ModifierAltGraph)
      {
        result.Add(Modifier::AltGraphKey);
      }
      if (initializer.ModifierCapsLock)
      {
        result.Add(Modifier::CapsLockKey);
      }
      return result;
    }
  };

  /// @brief The first keyboard or mouse event in the chain of events from @p event through their underlying
  /// events (a simulated click's key press), if any.
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
