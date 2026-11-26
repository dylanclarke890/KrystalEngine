/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2011-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Html/InputTypes/BaseCheckableInputType.hpp"
#include "Krystal.Web/Dom/Events/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Forms/DOMFormData.hpp"

namespace krys::web::html
{
  bool BaseCheckableInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.Checked())
    {
      return false;
    }
    formData.Append(element.Name(), element.Value());
    return true;
  }

  InputType::ShouldCallBaseEventHandler
    BaseCheckableInputType::HandleKeydownEvent(KeyboardEvent &event) noexcept
  {
    if (event.Key() == u8" ")
    {
      krys_debug_assert(Element());
      Element()->SetActive(true);
      // No SetDefaultHandled(): a keypress follows only if the keydown isn't handled.
      return ShouldCallBaseEventHandler::No;
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void BaseCheckableInputType::HandleKeypressEvent(KeyboardEvent &event) noexcept
  {
    if (event.CharCode() == ' ')
    {
      // Prevent scrolling down the page.
      event.SetDefaultHandled();
    }
  }

  bool BaseCheckableInputType::CanSetStringValue() const noexcept
  {
    return false;
  }

  Maybe<DOMString> BaseCheckableInputType::FallbackValue() const noexcept
  {
    return DOMString(u8"on");
  }

  bool BaseCheckableInputType::StoresValueSeparateFromAttribute() noexcept
  {
    return false;
  }

  void BaseCheckableInputType::SetValue(DOMStringView sanitizedValue, bool, TextFieldEventBehavior,
                                        TextControlSetValueSelection) noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetAttribute(u8"value", DOMString(sanitizedValue));
  }

  void BaseCheckableInputType::FireInputAndChangeEvents() noexcept
  {
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (!element->IsConnected())
    {
      return;
    }

    if (!ShouldSendChangeEventAfterCheckedChanged())
    {
      return;
    }

    Ref<InputType> protectedThis = ShareRef(*this);
    element->SetTextAsOfLastFormControlChangeEvent({});
    auto event = Event::Create(event_names::Input, true, false, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, element.get());
    if (auto *currentElement = Element())
    {
      currentElement->DispatchFormControlChangeEvent();
    }
  }
}
