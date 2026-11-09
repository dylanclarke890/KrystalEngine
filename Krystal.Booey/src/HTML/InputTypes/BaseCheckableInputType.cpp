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

#include "Krystal.Booey/HTML/InputTypes/BaseCheckableInputType.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"

namespace krys::boo::html
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
    BaseCheckableInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() == u8"U+0020")
    {
      krys_debug_assert(Element());
      Element()->SetActive(true);
      // No SetDefaultHandled(): a keypress follows only if the keydown isn't handled.
      return ShouldCallBaseEventHandler::No;
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void BaseCheckableInputType::HandleKeypressEvent(dom::KeyboardEvent &event) noexcept
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

  Maybe<dom::DOMString> BaseCheckableInputType::FallbackValue() const noexcept
  {
    return dom::DOMString(u8"on");
  }

  bool BaseCheckableInputType::StoresValueSeparateFromAttribute() noexcept
  {
    return false;
  }

  void BaseCheckableInputType::SetValue(dom::DOMStringView sanitizedValue, bool, TextFieldEventBehavior,
                                        TextControlSetValueSelection) noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetAttribute(u8"value", dom::DOMString(sanitizedValue));
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
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Input), true, false, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, element.get());
    if (auto *currentElement = Element())
    {
      currentElement->DispatchFormControlChangeEvent();
    }
  }
}
