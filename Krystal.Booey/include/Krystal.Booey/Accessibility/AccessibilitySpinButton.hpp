/*
 * Copyright (C) 2011-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityMockObject.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Node;
  }

  namespace html
  {
    class SpinButtonElement;
  }
}

namespace krys::boo::ax
{
  class AccessibilitySpinButtonPart;

  /// @brief A number input's spin button (its inner `SpinButtonElement`, which isn't in the tree): an
  /// increment and a decrement button.
  class AccessibilitySpinButton final : public AccessibilityMockObject
  {
  private:
    WeakPtr<dom::Node> _spinButtonElement;

  public:
    KRYS_NODISCARD static Ref<AccessibilitySpinButton> Create(AXID id, AXObjectCache &cache) noexcept;
    ~AccessibilitySpinButton() noexcept override;

    void SetSpinButtonElement(html::SpinButtonElement *spinButton) noexcept;
    KRYS_NODISCARD AccessibilitySpinButtonPart *IncrementButton() const noexcept;
    KRYS_NODISCARD AccessibilitySpinButtonPart *DecrementButton() const noexcept;
    /// @brief Steps the input's value up (a positive @p amount) or down.
    void Step(int32 amount) noexcept;

    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;
    /// @brief The parts, made with the spin button, stay its children.
    void ClearChildren() noexcept override
    {
    }
    void AddChildren() noexcept override;

  private:
    AccessibilitySpinButton(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::SpinButton;
    }
    /// @brief Removes the parts from the cache with the spin button.
    void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilitySpinButton)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMockObject() && object.Role() == krys::boo::ax::AccessibilityRole::SpinButton;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
