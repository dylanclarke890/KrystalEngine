/*
 * Copyright (C) 2010, 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/InputTypes/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"

namespace krys::boo::html
{
  InputType::ShouldCallBaseEventHandler
    BaseClickableWithKeyInputType::HandleKeydownEvent(HTMLInputElement &element,
                                                      dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() == u8"U+0020")
    {
      element.SetActive(true);
      // No SetDefaultHandled(): a keypress follows only if the keydown isn't handled.
      return ShouldCallBaseEventHandler::No;
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void BaseClickableWithKeyInputType::HandleKeypressEvent(HTMLInputElement &element,
                                                          dom::KeyboardEvent &event) noexcept
  {
    int32 charCode = event.CharCode();
    if (charCode == '\r')
    {
      (void)element.DispatchSimulatedClick(&event);
      event.SetDefaultHandled();
      return;
    }
    if (charCode == ' ')
    {
      // Prevent scrolling down the page.
      event.SetDefaultHandled();
    }
  }

  void BaseClickableWithKeyInputType::HandleKeyupEvent(InputType &inputType,
                                                       dom::KeyboardEvent &event) noexcept
  {
    if (event.KeyIdentifier() != u8"U+0020")
    {
      return;
    }
    // Simulate mouse click for spacebar for button types.
    inputType.DispatchSimulatedClickIfActive(event);
  }

  InputType::ShouldCallBaseEventHandler
    BaseClickableWithKeyInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    return HandleKeydownEvent(*Element(), event);
  }

  void BaseClickableWithKeyInputType::HandleKeypressEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    HandleKeypressEvent(*Element(), event);
  }

  void BaseClickableWithKeyInputType::HandleKeyupEvent(dom::KeyboardEvent &event) noexcept
  {
    HandleKeyupEvent(*this, event);
  }
}
