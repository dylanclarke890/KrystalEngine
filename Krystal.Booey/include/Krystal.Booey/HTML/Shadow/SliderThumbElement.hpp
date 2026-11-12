/*
 * Copyright (C) 2006-2018 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/DoublePoint.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"

namespace krys::boo::dom
{
  class TouchEvent;
}

namespace krys::boo::html
{
  class HTMLInputElement;

  /// @brief A range's thumb (the `::-webkit-slider-thumb` part), which the user drags along the track.
  class SliderThumbElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SliderThumbElement);

  private:
    bool _inDragMode {false};

  public:
    explicit SliderThumbElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<SliderThumbElement> Create(dom::Document &) noexcept;

    void SetPositionFromValue() noexcept;

    void DragFrom(const LayoutPoint &) noexcept;
    KRYS_NODISCARD HTMLInputElement *HostInput() const noexcept;
    void SetPositionFromPoint(const LayoutPoint &) noexcept;

    void HostDisabledStateChanged() noexcept;

    KRYS_NODISCARD bool IsSliderThumbElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final;
    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept final;
    void DefaultEventHandler(dom::Event &) noexcept final;

  private:
    void WillDetachRenderers() noexcept final;
    KRYS_NODISCARD Maybe<style::UnadjustedStyle> ResolveCustomStyle(const style::ResolutionContext &,
                                                                    const RenderStyle *) noexcept final;

    void StartDragging() noexcept;
    void StopDragging() noexcept;
  };

  /// @brief The container of a range's track (the `::-webkit-slider-container` part), which lays the track
  /// and thumb out, and drags the thumb by touch.
  class SliderContainerElement final : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SliderContainerElement);

    enum class SlidingDirection : uint8
    {
      NoMove,
      Horizontal,
      Vertical
    };

  private:
    DoublePoint _touchStartPoint;
    SlidingDirection _slidingDirection {SlidingDirection::NoMove};
    bool _touchStarted {false};

  public:
    explicit SliderContainerElement(dom::Document &) noexcept;

    KRYS_NODISCARD static Ref<SliderContainerElement> Create(dom::Document &) noexcept;

    KRYS_NODISCARD bool IsSliderContainerElement() const noexcept final
    {
      return true;
    }

    void DefaultEventHandler(dom::Event &) noexcept final;

  private:
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    void OnAdopt(dom::Document &oldDocument) noexcept final;

    /// @brief Moves the thumb to a single touch as it starts, and with the touch's moves along the slider,
    /// as Chrome does; a touch that first moves across the slider is left to pan.
    void HandleTouchEvent(dom::TouchEvent &) noexcept;
    /// @brief Whether the touch's moves so far are along the slider.
    KRYS_NODISCARD bool CanSlide() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SliderThumbElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSliderThumbElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::SliderContainerElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &element) noexcept
  {
    return element.IsSliderContainerElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsHTMLElement() && IsType(Downcast<krys::boo::html::HTMLElement>(node));
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
