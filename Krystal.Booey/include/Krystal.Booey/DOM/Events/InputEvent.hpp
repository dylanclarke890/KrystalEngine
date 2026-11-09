/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/DataTransfer/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Events/UIEvent.hpp"
#include "Krystal.Booey/DOM/Ranges/StaticRange.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  enum class IsInputMethodComposing : bool
  {
    No,
    Yes
  };

  /// @see https://w3c.github.io/uievents/#dictdef-inputeventinit
  struct InputEventInit : UIEventInit
  {
    Maybe<DOMString> Data;
    RefPtr<dom::DataTransfer> DataTransfer;
    bool IsComposing {false};
    DOMString InputType;
  };

  /// @brief An event about an edit: `beforeinput` before it's made, which can cancel it, and `input` after.
  /// @see https://w3c.github.io/uievents/#interface-inputevent
  class InputEvent final : public UIEvent
  {
  private:
    DOMString _inputType;
    Maybe<DOMString> _data;
    RefPtr<DataTransfer> _dataTransfer;
    List<RefPtr<StaticRange>> _targetRanges;
    bool _isInputMethodComposing {false};

  public:
    /// @brief The dictionary its constructor takes.
    using dictionary_type = InputEventInit;

    /// @see https://w3c.github.io/uievents/#dom-inputevent-inputevent
    InputEvent(DOMStringAtom type, const InputEventInit &eventInitDict = {}) noexcept;
    ~InputEvent() noexcept;

    /// @brief A trusted input event, which bubbles and crosses shadow boundaries.
    KRYS_NODISCARD static Ref<InputEvent> Create(DOMStringAtom type, LocalDOMWindow *view,
                                                 DOMStringView inputType, bool cancelable,
                                                 Maybe<DOMString> data, RefPtr<DataTransfer> &&dataTransfer,
                                                 const List<RefPtr<StaticRange>> &targetRanges, int32 detail,
                                                 IsInputMethodComposing isInputMethodComposing) noexcept;

    KRYS_NODISCARD bool IsInputEvent() const noexcept final
    {
      return true;
    }

    /// @see https://w3c.github.io/uievents/#dom-inputevent-inputtype
    KRYS_NODISCARD const DOMString &InputType() const noexcept
    {
      return _inputType;
    }

    /// @see https://w3c.github.io/uievents/#dom-inputevent-data
    KRYS_NODISCARD const Maybe<DOMString> &Data() const noexcept
    {
      return _data;
    }

    /// @see https://w3c.github.io/input-events/#dom-inputevent-datatransfer
    KRYS_NODISCARD const RefPtr<DataTransfer> &GetDataTransfer() const noexcept
    {
      return _dataTransfer;
    }

    /// @see https://w3c.github.io/uievents/#dom-inputevent-iscomposing
    KRYS_NODISCARD bool IsComposing() const noexcept
    {
      return _isInputMethodComposing;
    }

    /// @see https://w3c.github.io/input-events/#dom-inputevent-gettargetranges
    KRYS_NODISCARD const List<RefPtr<StaticRange>> &GetTargetRanges() const noexcept
    {
      return _targetRanges;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::InputEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsInputEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
