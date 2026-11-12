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

#include "Krystal.Booey/Accessibility/AccessibilityNodeObject.hpp"

namespace krys::boo::html
{
  class HTMLOptionElement;
}

namespace krys::boo::ax
{
  /// @brief An `option` of a drop-down `select`: a child of the menu list's pop-up, with no children.
  class AccessibilityMenuListOption final : public AccessibilityNodeObject
  {
  private:
    WeakPtr<AccessibilityObject> _parent;

  public:
    KRYS_NODISCARD static Ref<AccessibilityMenuListOption> Create(AXID id, html::HTMLOptionElement &element,
                                                                  AXObjectCache &cache) noexcept;

    KRYS_NODISCARD bool IsMenuListOption() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override
    {
      return _parent.get();
    }
    void SetParent(AccessibilityObject *parent) noexcept
    {
      _parent = CreateWeakPtr(parent);
    }
    void DetachFromParent() noexcept override
    {
      _parent = nullptr;
    }
    void AddChildren() noexcept override
    {
      _childrenInitialized = true;
    }
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool IsEnabled() const noexcept override;
    KRYS_NODISCARD bool IsSelected() const noexcept override;
    KRYS_NODISCARD bool CanSetSelectedAttribute() const noexcept override
    {
      return IsEnabled();
    }
    KRYS_NODISCARD dom::DOMString StringValue() const noexcept override;
    void SetSelected(bool selected) noexcept override;
    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

  private:
    AccessibilityMenuListOption(AXID id, html::HTMLOptionElement &element, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::MenuListOption;
    }
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override
    {
      return IsIgnoredByDefault();
    }
    KRYS_NODISCARD html::HTMLOptionElement *OptionElement() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityMenuListOption)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsMenuListOption();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
