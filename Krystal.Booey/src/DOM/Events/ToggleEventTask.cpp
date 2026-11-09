/*
 * Copyright (C) 2024 Keith Cirkel <webkit@keithcirkel.co.uk>. All rights reserved.
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

#include "Krystal.Booey/DOM/Events/ToggleEventTask.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/ToggleEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"

namespace krys::boo::dom
{
  ToggleEventTask::ToggleEventTask(Element &element) noexcept : _element(CreateWeakPtr(&element))
  {
  }

  void ToggleEventTask::Queue(ToggleState oldState, ToggleState newState, Element *source) noexcept
  {
    if (_data)
    {
      oldState = _data->OldState;
    }

    RefPtr<Element> element = _element.lock();
    if (!element)
    {
      return;
    }

    _data = ToggleEventData {oldState, newState, ShareRefPtr(source)};
    html::EventLoop::Current().QueueTask(
      html::TaskSource::DOMManipulation, element->NodeDocument(),
      [task = ShareRef(*this), element, newState]
      {
        if (!task->_data || task->_data->NewState != newState)
        {
          return;
        }

        auto stringForState = [](ToggleState state)
        {
          return DOMString(state == ToggleState::Closed ? u8"closed" : u8"open");
        };

        auto data = *std::exchange(task->_data, null);
        ToggleEventInit init;
        init.OldState = stringForState(data.OldState);
        init.NewState = stringForState(data.NewState);
        init.Source = krys::move(data.Source);
        auto event = ToggleEvent::Create(DOMStringAtom(EventNames::Toggle), init, false);
        (void)EventDispatcher::DispatchToTarget(*event, element.get());
      });
  }
}
