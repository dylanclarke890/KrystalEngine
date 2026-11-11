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

namespace krys::boo::ax
{
  /// @brief An accessibility object for a renderer: what the renderer adds to its node's object (whether
  /// it's in the tree given how it's rendered, its role from the kind of renderer, and its bounds). Its
  /// children still come from the DOM, apart from what only the render tree has (list markers).
  class AccessibilityRenderObject : public AccessibilityNodeObject
  {
  private:
    WeakPtr<RenderObject> _renderer;

  public:
    /// @brief An object for @p renderer, and its node if it has one.
    KRYS_NODISCARD static Ref<AccessibilityRenderObject> Create(AXID id, RenderObject &renderer,
                                                                AXObjectCache &cache) noexcept;
    /// @brief An object for @p node with no renderer yet (`display: contents` and so on).
    KRYS_NODISCARD static Ref<AccessibilityRenderObject> Create(AXID id, dom::Node &node,
                                                                AXObjectCache &cache) noexcept;
    ~AccessibilityRenderObject() noexcept override;

    KRYS_NODISCARD bool IsDetached() const noexcept override
    {
      return !_renderer && AccessibilityNodeObject::IsDetached();
    }
    KRYS_NODISCARD bool IsAccessibilityRenderObject() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD RenderObject *Renderer() const noexcept override
    {
      return _renderer.get();
    }
    /// @brief Sets the renderer, for a node whose renderer was created or destroyed; whether it changed.
    bool SetRendererIfNeeded(RenderObject *renderer) noexcept;
    KRYS_NODISCARD dom::Document *GetDocument() const noexcept override;

    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override;
    void AddChildren() noexcept override;

    KRYS_NODISCARD dom::DOMString StringValue() const noexcept override;
    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

  protected:
    AccessibilityRenderObject(AXID id, RenderObject *renderer, dom::Node *node,
                              AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override;
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;
    void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept override;

  private:
    /// @brief Updates the roles that depend on the children (a menu with no items, a list).
    void UpdateRoleAfterChildrenCreation() noexcept;
    /// @brief The list item's marker renderer, if this is an unignored list item with one.
    KRYS_NODISCARD RenderObject *MarkerRenderer() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityRenderObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsAccessibilityRenderObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
