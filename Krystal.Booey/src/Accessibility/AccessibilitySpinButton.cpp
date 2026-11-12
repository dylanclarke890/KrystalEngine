/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilitySpinButton.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySpinButtonPart.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo::ax
{
  AccessibilitySpinButton::AccessibilitySpinButton(AXID id, AXObjectCache &cache) noexcept
      : AccessibilityMockObject(id, cache)
  {
  }

  AccessibilitySpinButton::~AccessibilitySpinButton() noexcept = default;

  Ref<AccessibilitySpinButton> AccessibilitySpinButton::Create(AXID id, AXObjectCache &cache) noexcept
  {
    auto spinButton = AdoptRef(*new AccessibilitySpinButton(id, cache));
    // The parts are made here rather than in the constructor: they need a reference to the spin button.
    for (bool isIncrementor : {true, false})
    {
      auto &part = Downcast<AccessibilitySpinButtonPart>(*cache.Create(AccessibilityRole::SpinButtonPart));
      part.SetIsIncrementor(isIncrementor);
      part.SetParent(spinButton.get());
      spinButton->AddChild(&part);
    }
    spinButton->_childrenInitialized = true;
    return spinButton;
  }

  void AccessibilitySpinButton::SetSpinButtonElement(html::SpinButtonElement *spinButton) noexcept
  {
    _spinButtonElement = CreateWeakPtr<dom::Node>(spinButton);
  }

  AccessibilitySpinButtonPart *AccessibilitySpinButton::IncrementButton() const noexcept
  {
    return _children.size() == 2 ? DynamicDowncast<AccessibilitySpinButtonPart>(_children[0].get()) : nullptr;
  }

  AccessibilitySpinButtonPart *AccessibilitySpinButton::DecrementButton() const noexcept
  {
    return _children.size() == 2 ? DynamicDowncast<AccessibilitySpinButtonPart>(_children[1].get()) : nullptr;
  }

  void AccessibilitySpinButton::Step(int32 amount) noexcept
  {
    if (auto element = ShareRefPtr(DynamicDowncast<html::SpinButtonElement>(_spinButtonElement.get())))
    {
      element->Step(amount);
    }
  }

  LayoutRect AccessibilitySpinButton::ElementRect() const noexcept
  {
    auto *element = DynamicDowncast<html::SpinButtonElement>(_spinButtonElement.get());
    auto *renderer = element ? element->Renderer() : nullptr;
    return renderer ? LayoutRect(renderer->AbsoluteBoundingBoxRect()) : LayoutRect();
  }

  void AccessibilitySpinButton::AddChildren() noexcept
  {
    // The children are set once, when the spin button is made, and are never out of date.
    krys_debug_assert(_childrenInitialized);
    krys_debug_assert(!_subtreeDirty);
    krys_debug_assert(!_childrenDirty);
  }

  void AccessibilitySpinButton::DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept
  {
    auto *cache = AxObjectCache();
    AccessibilityMockObject::DetachRemoteParts(detachmentType);
    auto parts = std::exchange(_children, {});
    if (cache)
    {
      for (const auto &part : parts)
      {
        cache->Remove(part->ObjectID());
      }
    }
  }
}
