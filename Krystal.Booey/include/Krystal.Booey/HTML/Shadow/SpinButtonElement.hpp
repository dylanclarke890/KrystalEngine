/*
 * Copyright (C) 2006, 2008, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/Page/PopupOpeningObserver.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Page;
}

namespace krys::boo::html
{
  /// @brief What a spin button steps: the input type of the field it's in.
  class SpinButtonOwner
  {
  public:
    virtual ~SpinButtonOwner() noexcept = default;
    virtual void FocusAndSelectSpinButtonOwner() noexcept = 0;
    KRYS_NODISCARD virtual bool ShouldSpinButtonRespondToMouseEvents() const noexcept = 0;
    virtual void SpinButtonStepDown() noexcept = 0;
    virtual void SpinButtonStepUp() noexcept = 0;
  };

  /// @brief A number field's spin button (the `::-webkit-inner-spin-button` part): its upper half steps up
  /// and its lower half down, repeating while held.
  class SpinButtonElement final : public HTMLDivElement, public PopupOpeningObserver
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SpinButtonElement);

  public:
    enum class UpDownState : uint8
    {
      Indeterminate, // Hovered, but the event is not handled.
      Down,
      Up,
    };

  private:
    // The owner clears this when it goes (RemoveSpinButtonOwner).
    SpinButtonOwner *_spinButtonOwner;
    bool _capturing {false};
    // The page told of popups while the mouse is captured.
    WeakPtr<boo::Page> _popupObservedPage;
    UpDownState _upDownState {UpDownState::Indeterminate};
    UpDownState _pressStartingState {UpDownState::Indeterminate};
    Timer _repeatingTimer;

  public:
    SpinButtonElement(dom::Document &, SpinButtonOwner &) noexcept;
    ~SpinButtonElement() noexcept override;

    // The owner of SpinButtonElement must call removeSpinButtonOwner
    // because SpinButtonElement can be outlive SpinButtonOwner
    // implementation, e.g. during event handling.
    KRYS_NODISCARD static Ref<SpinButtonElement> Create(dom::Document &, SpinButtonOwner &) noexcept;

    KRYS_NODISCARD UpDownState GetUpDownState() const noexcept
    {
      return _upDownState;
    }

    void ReleaseCapture() noexcept;
    void RemoveSpinButtonOwner() noexcept
    {
      _spinButtonOwner = nullptr;
    }

    void Step(int32 amount) noexcept;

    KRYS_NODISCARD bool IsSpinButtonElement() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept override;
    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept override;
    void DefaultEventHandler(dom::Event &) noexcept override;

    void SetHovered(bool, style::InvalidationScope = style::InvalidationScope::All,
                    HitTestRequest = {}) noexcept override;

  private:
    void WillDetachRenderers() noexcept final;

    // PopupOpeningObserver.
    void WillOpenPopup() noexcept final;
    void StopObservingPopups() noexcept;

    void DoStepAction(int32) noexcept;
    void StartRepeatingTimer() noexcept;
    void StopRepeatingTimer() noexcept;
    void RepeatingTimerFired() noexcept;

    KRYS_NODISCARD bool ShouldRespondToMouseEvents() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SpinButtonElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSpinButtonElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
