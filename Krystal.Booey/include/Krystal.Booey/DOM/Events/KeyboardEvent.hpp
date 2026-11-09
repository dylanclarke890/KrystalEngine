/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2017 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Events/UIEvent.hpp"
#include "Krystal.Booey/DOM/Events/UIEventWithKeyState.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
}

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/uievents/#dictdef-keyboardeventinit
  struct KeyboardEventInit : EventModifierInit
  {
    DOMString Key;
    DOMString Code;
    uint32 Location {0};
    bool Repeat {false};
    bool IsComposing {false};

    // Legacy.
    DOMString KeyIdentifier;
    /// @see https://w3c.github.io/uievents/#dom-keyboardeventinit-charcode
    uint32 CharCode {0};
    /// @see https://w3c.github.io/uievents/#dom-keyboardeventinit-keycode
    uint32 KeyCode {0};
    /// @see https://w3c.github.io/uievents/#dom-uieventinit-which
    uint32 Which {0};
  };

  /// @brief An event from a keyboard: `keydown`, `keypress` and `keyup`.
  /// @see https://w3c.github.io/uievents/#interface-keyboardevent
  // FIXME: Key commands.
  class KeyboardEvent final : public UIEventWithKeyState
  {
  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = KeyboardEventInit;

    enum KeyLocationCode : uint32
    {
      DOM_KEY_LOCATION_STANDARD = 0x00,
      DOM_KEY_LOCATION_LEFT = 0x01,
      DOM_KEY_LOCATION_RIGHT = 0x02,
      DOM_KEY_LOCATION_NUMPAD = 0x03
    };

  private:
    UniquePtr<PlatformKeyboardEvent> _underlyingPlatformEvent;
    DOMString _key;
    DOMString _code;
    DOMString _keyIdentifier;
    uint32 _location {DOM_KEY_LOCATION_STANDARD};
    bool _repeat {false};
    bool _isComposing {false};
    Maybe<uint32> _charCode;
    Maybe<uint32> _keyCode;
    Maybe<uint32> _which;

  public:
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-keyboardevent
    KeyboardEvent(DOMStringAtom type, const KeyboardEventInit &initializer = {}) noexcept;

    /// @brief A trusted event from @p key, which must be a raw key down, a char or a key up (a combined key
    /// down is disambiguated first); it's composing if @p view's editor has a composition.
    KRYS_NODISCARD static Ref<KeyboardEvent> Create(const PlatformKeyboardEvent &key,
                                                    LocalDOMWindow *view = nullptr) noexcept;

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-initkeyboardevent
    void InitKeyboardEvent(DOMStringAtom type, bool canBubble, bool cancelable, LocalDOMWindow *view,
                           DOMStringView keyIdentifier, uint32 location, bool ctrlKey, bool altKey,
                           bool shiftKey, bool metaKey) noexcept;

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-key
    KRYS_NODISCARD const DOMString &Key() const noexcept
    {
      return _key;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-code
    KRYS_NODISCARD const DOMString &Code() const noexcept
    {
      return _code;
    }

    KRYS_NODISCARD const DOMString &KeyIdentifier() const noexcept
    {
      return _keyIdentifier;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-location
    KRYS_NODISCARD uint32 Location() const noexcept
    {
      return _location;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-repeat
    KRYS_NODISCARD bool Repeat() const noexcept
    {
      return _repeat;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-iscomposing
    KRYS_NODISCARD bool IsComposing() const noexcept
    {
      return _isComposing;
    }

    KRYS_NODISCARD const PlatformKeyboardEvent *UnderlyingPlatformEvent() const noexcept
    {
      return _underlyingPlatformEvent.get();
    }

    /// @brief The key code for `keydown` and `keyup`, the character for `keypress`.
    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-keycode
    KRYS_NODISCARD int32 KeyCode() const noexcept;
    /// @brief The key code for the `keydown` that matches a `keypress`.
    KRYS_NODISCARD int32 KeyCodeForKeyDown() const noexcept;
    /// @brief The character code for `keypress`, 0 for `keydown` and `keyup`.
    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-charcode
    KRYS_NODISCARD int32 CharCode() const noexcept;
    /// @note Legacy: the same as `KeyCode()`.
    /// @see https://w3c.github.io/uievents/#dom-uievent-which
    KRYS_NODISCARD uint32 Which() const noexcept;

    KRYS_NODISCARD bool IsKeyboardEvent() const noexcept final
    {
      return true;
    }

  private:
    KeyboardEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                  const PlatformKeyboardEvent &key) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::KeyboardEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsKeyboardEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
