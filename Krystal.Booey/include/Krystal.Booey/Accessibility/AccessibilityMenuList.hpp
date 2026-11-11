/*
 * Copyright (C) 2010-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"

namespace krys::boo
{
  class RenderMenuList;
}

namespace krys::boo::ax
{
  class AccessibilityMenuListPopup;

  /// @brief A drop-down `select`: a pop-up button whose one child is its pop-up, whose children are the
  /// options.
  /// @note WebKit's pop-up buttons have no children on its Apple ports; Booey's, like its other ports' and
  /// Chrome's, have the pop-up and options.
  class AccessibilityMenuList final : public AccessibilityRenderObject
  {
  private:
    Ref<AccessibilityMenuListPopup> _popup;

  public:
    KRYS_NODISCARD static Ref<AccessibilityMenuList> Create(AXID id, RenderMenuList &renderer,
                                                            AXObjectCache &cache) noexcept;
    ~AccessibilityMenuList() noexcept override;

    KRYS_NODISCARD bool IsMenuList() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD AccessibilityMenuListPopup &Popup() const noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return true;
    }
    void AddChildren() noexcept override
    {
    }
    void UpdateChildrenIfNecessary() noexcept override;
    // The pop-up is the only child, for the menu list's life: changes are the pop-up's.
    void ClearChildren() noexcept override
    {
    }
    void SetNeedsToUpdateChildren() noexcept override
    {
    }

    KRYS_NODISCARD bool IsCollapsed() const noexcept override;
    KRYS_NODISCARD bool CanSetFocusAttribute() const noexcept override;

  private:
    AccessibilityMenuList(AXID id, RenderMenuList &renderer, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::PopUpButton;
    }
    void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityMenuList)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMenuList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
