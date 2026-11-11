/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityMockObject.hpp"

namespace krys::boo::ax
{
  /// @brief A drop-down `select`'s pop-up: the menu list's child, whose children are the select's options.
  class AccessibilityMenuListPopup final : public AccessibilityMockObject
  {
  public:
    KRYS_NODISCARD static Ref<AccessibilityMenuListPopup> Create(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD bool IsMenuListPopup() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool IsEnabled() const noexcept override;
    void AddChildren() noexcept override;
    void SetNeedsToUpdateChildren() noexcept override
    {
      _childrenDirty = true;
    }
    void SetNeedsToUpdateSubtree() noexcept override
    {
      _subtreeDirty = true;
    }
    KRYS_NODISCARD bool NeedsToUpdateChildren() const noexcept override
    {
      return _childrenDirty;
    }
    void UpdateChildrenIfNecessary() noexcept override;
    /// @brief The select's options changed: removes the objects of the options no longer in the document and
    /// adds the children again.
    void HandleChildrenChanged() noexcept;

  private:
    AccessibilityMenuListPopup(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::MenuListPopup;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityMenuListPopup)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMenuListPopup();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
