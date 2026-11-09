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

#include "Krystal.Booey/DOM/Events/InputEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer/DataTransfer.hpp"

namespace krys::boo::dom
{
  InputEvent::InputEvent(DOMStringAtom type, const InputEventInit &eventInitDict) noexcept
      : UIEvent(type, eventInitDict), _inputType(eventInitDict.InputType), _data(eventInitDict.Data),
        _dataTransfer(eventInitDict.DataTransfer), _isInputMethodComposing(eventInitDict.IsComposing)
  {
  }

  InputEvent::~InputEvent() noexcept = default;

  Ref<InputEvent> InputEvent::Create(DOMStringAtom type, LocalDOMWindow *view, DOMStringView inputType,
                                     bool cancelable, Maybe<DOMString> data,
                                     RefPtr<DataTransfer> &&dataTransfer,
                                     const List<RefPtr<StaticRange>> &targetRanges, int32 detail,
                                     IsInputMethodComposing isInputMethodComposing) noexcept
  {
    InputEventInit init;
    init.View = view;
    init.Bubbles = true;
    init.Cancelable = cancelable;
    init.Composed = true;
    init.Detail = detail;
    init.Data = krys::move(data);
    init.DataTransfer = krys::move(dataTransfer);
    init.IsComposing = isInputMethodComposing == IsInputMethodComposing::Yes;
    init.InputType = DOMString(inputType);
    auto event = CreateRef<InputEvent>(type, init);
    event->_targetRanges = targetRanges;
    event->_isTrusted = true;
    return event;
  }
}
