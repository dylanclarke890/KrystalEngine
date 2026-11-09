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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/DoublePoint.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Event;
  class MouseEvent;
  class UIEventWithKeyState;
}

namespace krys::boo
{
  /// @brief A navigation as the host is asked about it: the URL, what started it (a link, a form) and the
  /// input event that did.
  class NavigationAction : public FrameLoadRequestBase
  {
  public:
    /// @brief The modifier keys of the event that started a navigation.
    struct UIEventWithKeyStateData
    {
      bool IsTrusted {false};
      bool ShiftKey {false};
      bool CtrlKey {false};
      bool AltKey {false};
      bool MetaKey {false};

      explicit UIEventWithKeyStateData(const dom::UIEventWithKeyState &uiEvent) noexcept;
    };

    /// @brief The mouse event that started a navigation.
    struct MouseEventData : UIEventWithKeyStateData
    {
      /// @brief Where it happened, in the frame's absolute coordinates.
      DoublePoint AbsoluteLocation;
      MouseButton Button {MouseButton::None};
      bool ButtonDown {false};

      explicit MouseEventData(const dom::MouseEvent &mouseEvent) noexcept;
    };

  private:
    ResourceRequest _originalRequest;
    Maybe<UIEventWithKeyStateData> _keyStateEventData;
    Maybe<MouseEventData> _mouseEventData;
    NavigationType _type {NavigationType::Other};

  public:
    NavigationAction() noexcept;
    NavigationAction(const ResourceRequest &originalRequest, bool isFormSubmission,
                     dom::Event *event = nullptr,
                     dom::DOMStringAtom downloadAttribute = dom::DOMStringAtom::Null(),
                     dom::Element *sourceElement = nullptr) noexcept;
    ~NavigationAction() noexcept;
    NavigationAction(const NavigationAction &) noexcept;
    NavigationAction &operator=(const NavigationAction &) noexcept;
    NavigationAction(NavigationAction &&) noexcept;
    NavigationAction &operator=(NavigationAction &&) noexcept;

    /// @brief The modifier keys of the first event with them in the chain of events that started the
    /// navigation (a simulated click's underlying key press), if any.
    KRYS_NODISCARD const Maybe<UIEventWithKeyStateData> &KeyStateEventData() const noexcept
    {
      return _keyStateEventData;
    }

    /// @brief The first mouse event in the chain of events that started the navigation, if any.
    KRYS_NODISCARD const Maybe<MouseEventData> &GetMouseEventData() const noexcept
    {
      return _mouseEventData;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _originalRequest.URL().IsEmpty();
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _originalRequest.URL();
    }

    KRYS_NODISCARD const ResourceRequest &OriginalRequest() const noexcept
    {
      return _originalRequest;
    }

    KRYS_NODISCARD NavigationType Type() const noexcept
    {
      return _type;
    }
  };
}
