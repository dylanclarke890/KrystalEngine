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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityMockObject.hpp"

namespace krys::boo::ax
{
  /// @brief A spin button's increment or decrement button: the top or bottom half of the spin button.
  class AccessibilitySpinButtonPart final : public AccessibilityMockObject
  {
  private:
    bool _isIncrementor {true};

  public:
    KRYS_NODISCARD static Ref<AccessibilitySpinButtonPart> Create(AXID id, AXObjectCache &cache) noexcept;
    ~AccessibilitySpinButtonPart() noexcept override;

    KRYS_NODISCARD bool IsIncrementor() const noexcept
    {
      return _isIncrementor;
    }
    void SetIsIncrementor(bool value) noexcept
    {
      _isIncrementor = value;
    }
    /// @brief Steps the spin button's input up or down; false if the part has no spin button.
    bool Press() noexcept override;

    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

  private:
    AccessibilitySpinButtonPart(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::SpinButtonPart;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilitySpinButtonPart)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMockObject() && object.Role() == krys::boo::ax::AccessibilityRole::SpinButtonPart;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
