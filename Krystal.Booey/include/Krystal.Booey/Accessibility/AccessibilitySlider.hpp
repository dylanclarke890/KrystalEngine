/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityMockObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo::ax
{
  /// @brief An `input type=range`: a slider whose only child is its thumb.
  class AccessibilitySlider final : public AccessibilityRenderObject
  {
  public:
    KRYS_NODISCARD static Ref<AccessibilitySlider> Create(AXID id, RenderObject &renderer,
                                                          AXObjectCache &cache) noexcept;

    void AddChildren() noexcept override;
    KRYS_NODISCARD float ValueForRange() const noexcept override;
    KRYS_NODISCARD float MaxValueForRange() const noexcept override;
    KRYS_NODISCARD float MinValueForRange() const noexcept override;
    KRYS_NODISCARD Maybe<AccessibilityOrientation> ExplicitOrientation() const noexcept override;
    using AccessibilityRenderObject::SetValue;
    bool SetValue(dom::DOMStringView value) noexcept override;

  private:
    AccessibilitySlider(AXID id, RenderObject &renderer, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::Slider;
    }
    KRYS_NODISCARD html::HTMLInputElement *InputElement() const noexcept;
  };

  /// @brief A slider's thumb, whose bounds are the thumb element's.
  class AccessibilitySliderThumb final : public AccessibilityMockObject
  {
  public:
    KRYS_NODISCARD static Ref<AccessibilitySliderThumb> Create(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

  private:
    AccessibilitySliderThumb(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::SliderThumb;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilitySliderThumb)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMockObject() && object.Role() == krys::boo::ax::AccessibilityRole::SliderThumb;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
