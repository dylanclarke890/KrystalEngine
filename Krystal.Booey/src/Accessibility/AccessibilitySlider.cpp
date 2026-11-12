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

#include "Krystal.Booey/Accessibility/AccessibilitySlider.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo::ax
{
  Ref<AccessibilitySlider> AccessibilitySlider::Create(AXID id, RenderObject &renderer,
                                                       AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilitySlider(id, renderer, cache));
  }

  AccessibilitySlider::AccessibilitySlider(AXID id, RenderObject &renderer, AXObjectCache &cache) noexcept
      : AccessibilityRenderObject(id, &renderer, renderer.GetNode(), cache)
  {
  }

  Maybe<AccessibilityOrientation> AccessibilitySlider::ExplicitOrientation() const noexcept
  {
    if (auto orientation = OrientationFromARIA())
    {
      return orientation;
    }
    const auto *style = Style();
    if (!style)
    {
      return AccessibilityOrientation::Horizontal;
    }
    switch (style->UsedAppearance())
    {
      case StyleAppearance::SliderThumbVertical:
      case StyleAppearance::SliderVertical:      return AccessibilityOrientation::Vertical;
      default:                                   return AccessibilityOrientation::Horizontal;
    }
  }

  void AccessibilitySlider::AddChildren() noexcept
  {
    krys_debug_assert(!_childrenInitialized);
    _childrenInitialized = true;
    _subtreeDirty = false;

    auto *cache = AxObjectCache();
    if (!cache)
    {
      return;
    }
    // The slider's user agent shadow tree isn't exposed: only a thumb, standing for it.
    auto thumb = ShareRef(Downcast<AccessibilitySliderThumb>(*cache->Create(AccessibilityRole::SliderThumb)));
    thumb->SetParent(this);
    if (thumb->IsIgnored())
    {
      cache->Remove(thumb->ObjectID());
    }
    else
    {
      AddChild(thumb.get());
    }
  }

  float AccessibilitySlider::ValueForRange() const noexcept
  {
    auto *input = InputElement();
    return input ? static_cast<float>(input->ValueAsNumber()) : 0;
  }

  float AccessibilitySlider::MaxValueForRange() const noexcept
  {
    auto *input = InputElement();
    return input ? static_cast<float>(input->Maximum()) : 0;
  }

  float AccessibilitySlider::MinValueForRange() const noexcept
  {
    auto *input = InputElement();
    return input ? static_cast<float>(input->Minimum()) : 0;
  }

  html::HTMLInputElement *AccessibilitySlider::InputElement() const noexcept
  {
    return DynamicDowncast<html::HTMLInputElement>(GetNode());
  }

  Ref<AccessibilitySliderThumb> AccessibilitySliderThumb::Create(AXID id, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilitySliderThumb(id, cache));
  }

  AccessibilitySliderThumb::AccessibilitySliderThumb(AXID id, AXObjectCache &cache) noexcept
      : AccessibilityMockObject(id, cache)
  {
  }

  bool AccessibilitySlider::SetValue(dom::DOMStringView value) noexcept
  {
    RefPtr<html::HTMLInputElement> input = ShareRefPtr(InputElement());
    if (!input)
    {
      return false;
    }
    if (input->Value() != value)
    {
      (void)input->SetValue(value, html::TextFieldEventBehavior::DispatchInputAndChangeEvent);
    }
    return true;
  }

  LayoutRect AccessibilitySliderThumb::ElementRect() const noexcept
  {
    auto *parent = _parent.get();
    auto *input = parent ? DynamicDowncast<html::HTMLInputElement>(parent->GetNode()) : nullptr;
    auto *thumb = input ? input->SliderThumbElement() : nullptr;
    auto *renderer = thumb ? thumb->Renderer() : nullptr;
    return renderer ? LayoutRect(renderer->AbsoluteBoundingBoxRect()) : LayoutRect();
  }
}
