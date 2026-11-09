/*
 * Copyright (C) 2006-2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo
{
  NavigationAction::UIEventWithKeyStateData::UIEventWithKeyStateData(
    const dom::UIEventWithKeyState &uiEvent) noexcept
      : IsTrusted(uiEvent.IsTrusted()), ShiftKey(uiEvent.ShiftKey()), CtrlKey(uiEvent.CtrlKey()),
        AltKey(uiEvent.AltKey()), MetaKey(uiEvent.MetaKey())
  {
  }

  NavigationAction::MouseEventData::MouseEventData(const dom::MouseEvent &mouseEvent) noexcept
      : UIEventWithKeyStateData(mouseEvent), AbsoluteLocation(mouseEvent.AbsoluteLocation()),
        Button(mouseEvent.ButtonType()), ButtonDown(mouseEvent.ButtonDown())
  {
  }

  namespace
  {
    KRYS_NODISCARD Maybe<NavigationAction::UIEventWithKeyStateData>
      KeyStateDataForFirstEventWithKeyState(dom::Event *event) noexcept
    {
      if (auto *uiEvent = dom::FindEventWithKeyState(event))
      {
        return NavigationAction::UIEventWithKeyStateData(*uiEvent);
      }
      return std::nullopt;
    }

    KRYS_NODISCARD Maybe<NavigationAction::MouseEventData>
      MouseEventDataForFirstMouseEvent(dom::Event *event) noexcept
    {
      for (auto *current = event; current; current = current->UnderlyingEvent())
      {
        if (auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(*current))
        {
          return NavigationAction::MouseEventData(*mouseEvent);
        }
      }
      return std::nullopt;
    }

    KRYS_NODISCARD NavigationType GetNavigationType(bool isFormSubmission, bool haveEvent) noexcept
    {
      if (isFormSubmission)
      {
        return NavigationType::FormSubmitted;
      }
      return haveEvent ? NavigationType::LinkClicked : NavigationType::Other;
    }
  }

  NavigationAction::NavigationAction() noexcept = default;

  NavigationAction::NavigationAction(const ResourceRequest &originalRequest, bool isFormSubmission,
                                     dom::Event *event, dom::DOMStringAtom downloadAttribute,
                                     dom::Element *sourceElement) noexcept
      : _originalRequest(originalRequest), _keyStateEventData(KeyStateDataForFirstEventWithKeyState(event)),
        _mouseEventData(MouseEventDataForFirstMouseEvent(event)),
        _type(GetNavigationType(isFormSubmission, event != nullptr))
  {
    SetDownloadAttribute(downloadAttribute);
    SetSourceElement(sourceElement);
  }

  NavigationAction::~NavigationAction() noexcept = default;
  NavigationAction::NavigationAction(const NavigationAction &) noexcept = default;
  NavigationAction &NavigationAction::operator=(const NavigationAction &) noexcept = default;
  NavigationAction::NavigationAction(NavigationAction &&) noexcept = default;
  NavigationAction &NavigationAction::operator=(NavigationAction &&) noexcept = default;
}
