/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/HTML/Types/TagName.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo::ax
{
  /// @brief An accessibility object for a DOM node: its role, states and name come from the node, and its
  /// children from the node's children in the composed tree. Nodes with no renderer that accessibility
  /// still needs (`display: contents`, canvas fallback content) get one of these; rendered ones get an
  /// `AccessibilityRenderObject`.
  class AccessibilityNodeObject : public AccessibilityObject
  {
  private:
    WeakPtr<dom::Node> _node;

  protected:
    AccessibilityRole _ariaRole {AccessibilityRole::Unknown};

  public:
    KRYS_NODISCARD static Ref<AccessibilityNodeObject> Create(AXID id, dom::Node &node,
                                                              AXObjectCache &cache) noexcept;
    ~AccessibilityNodeObject() noexcept override;

    void Init() noexcept override;
    KRYS_NODISCARD bool IsDetached() const noexcept override
    {
      return !_node;
    }
    KRYS_NODISCARD bool IsAccessibilityNodeObject() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD dom::Node *GetNode() const noexcept override
    {
      return _node.get();
    }

    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override;
    void AddChildren() noexcept override;
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override;
    void UpdateChildrenIfNecessary() noexcept override;
    void ClearChildren() noexcept override;
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

    KRYS_NODISCARD AccessibilityRole AriaRoleAttribute() const noexcept override
    {
      return _ariaRole;
    }

    KRYS_NODISCARD bool IsFocused() const noexcept override;
    KRYS_NODISCARD bool IsEnabled() const noexcept override;
    KRYS_NODISCARD bool IsChecked() const noexcept override;
    KRYS_NODISCARD bool IsIndeterminate() const noexcept override;
    KRYS_NODISCARD bool IsPressed() const noexcept override;
    KRYS_NODISCARD bool IsSelected() const noexcept override;
    KRYS_NODISCARD bool IsRequired() const noexcept override;
    KRYS_NODISCARD bool IsMultiSelectable() const noexcept override;
    KRYS_NODISCARD bool SupportsExpanded() const noexcept override;
    KRYS_NODISCARD bool IsExpanded() const noexcept override;
    KRYS_NODISCARD bool CanSetFocusAttribute() const noexcept override;
    KRYS_NODISCARD uint32 HeadingLevel() const noexcept override;

    KRYS_NODISCARD dom::DOMString AccessibleName() const noexcept override;
    KRYS_NODISCARD dom::DOMString AccessibleDescription() const noexcept override;
    KRYS_NODISCARD dom::DOMString StringValue() const noexcept override;

    /// @brief Whether the node is an `input` or `textarea` that edits text.
    KRYS_NODISCARD bool IsNativeTextControl() const noexcept;
    /// @brief Whether the element is the top of a `contenteditable` region.
    KRYS_NODISCARD bool HasContentEditableAttributeSet() const noexcept;
    /// @brief Whether the element has `aria-label`, `aria-labelledby` or `title`, which give it a name.
    KRYS_NODISCARD bool HasAccNameAttribute() const noexcept;

  protected:
    AccessibilityNodeObject(AXID id, dom::Node *node, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override;
    void RecomputeAriaRole() noexcept override
    {
      _ariaRole = DetermineAriaRoleAttribute();
    }
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;
    void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept override;

    /// @brief Whether `pre`, `kbd` and the like map to `Inline` (inside an inline renderer) or `TextGroup`.
    enum class TreatStyleFormatGroupAsInline : bool
    {
      No,
      Yes,
    };
    /// @brief The role the node's host language gives it (HTML-AAM's mappings), ignoring `role`.
    KRYS_NODISCARD AccessibilityRole
      DetermineAccessibilityRoleFromNode(TreatStyleFormatGroupAsInline treatStyleFormatGroupAsInline =
                                           TreatStyleFormatGroupAsInline::No) const noexcept;
    KRYS_NODISCARD AccessibilityRole RoleFromInputElement(const html::HTMLInputElement &input) const noexcept;
    /// @brief The role from the `role` attribute, if it names one that applies here.
    KRYS_NODISCARD AccessibilityRole DetermineAriaRoleAttribute() const noexcept;
    KRYS_NODISCARD AccessibilityRole RemapAriaRoleDueToParent(AccessibilityRole role) const noexcept;

    KRYS_NODISCARD bool IsDescendantOfElementType(Span<const html::TagName> tagNames) const noexcept;
    /// @brief Whether the element has something that makes it worth exposing even if its kind wouldn't be
    /// (an accessible name from an attribute, a `title` and so on).
    KRYS_NODISCARD bool HasAttributesRequiredForInclusion() const noexcept;
    /// @brief Whether the element is focusable but has no role of its own (`<div tabindex=0>`).
    KRYS_NODISCARD bool IsGenericFocusableElement() const noexcept;
    KRYS_NODISCARD bool IsStyleFormatGroup() const noexcept;
    /// @brief Whether the element is hidden by `display: none` or `visibility: hidden`.
    KRYS_NODISCARD bool IsRenderHidden() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityNodeObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsAccessibilityNodeObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
