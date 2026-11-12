/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityNodeObject.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLSelectElement;
}

namespace krys::boo::ax
{
  /// @brief An `option` or `optgroup` of a list box `select`: a child of the list box, wherever it is in the
  /// select, with no children of its own.
  class AccessibilityListBoxOption final : public AccessibilityNodeObject
  {
  public:
    KRYS_NODISCARD static Ref<AccessibilityListBoxOption> Create(AXID id, html::HTMLElement &element,
                                                                 AXObjectCache &cache) noexcept;
    ~AccessibilityListBoxOption() noexcept override;

    KRYS_NODISCARD bool IsListBoxOption() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override;
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
    KRYS_NODISCARD bool CanSetSelectedAttribute() const noexcept override;
    void SetSelected(bool selected) noexcept override;
    KRYS_NODISCARD dom::DOMString StringValue() const noexcept override;
    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

  private:
    AccessibilityListBoxOption(AXID id, html::HTMLElement &element, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::ListBoxOption;
    }
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;

    KRYS_NODISCARD html::HTMLSelectElement *ListBoxOptionParentNode() const noexcept;
    KRYS_NODISCARD int32 ListBoxOptionIndex() const noexcept;
  };
}
