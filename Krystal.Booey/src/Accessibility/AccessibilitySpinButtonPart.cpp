/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilitySpinButtonPart.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySpinButton.hpp"

namespace krys::boo::ax
{
  AccessibilitySpinButtonPart::AccessibilitySpinButtonPart(AXID id, AXObjectCache &cache) noexcept
      : AccessibilityMockObject(id, cache)
  {
  }

  AccessibilitySpinButtonPart::~AccessibilitySpinButtonPart() noexcept = default;

  Ref<AccessibilitySpinButtonPart> AccessibilitySpinButtonPart::Create(AXID id, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilitySpinButtonPart(id, cache));
  }

  LayoutRect AccessibilitySpinButtonPart::ElementRect() const noexcept
  {
    // FIXME: The render tree should say where the parts are; nothing relates them to it.
    auto *parent = ParentObject();
    if (!parent)
    {
      return {};
    }
    auto rect = parent->ElementRect();
    if (!_isIncrementor)
    {
      rect.SetY(rect.Y() + rect.Height() / 2);
    }
    rect.SetHeight(rect.Height() / 2);
    return rect;
  }

  bool AccessibilitySpinButtonPart::Press() noexcept
  {
    auto *spinButton = DynamicDowncast<AccessibilitySpinButton>(_parent.get());
    if (!spinButton)
    {
      return false;
    }
    spinButton->Step(_isIncrementor ? 1 : -1);
    return true;
  }
}
