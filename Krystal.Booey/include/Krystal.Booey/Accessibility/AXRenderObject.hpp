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

#include "Krystal.Booey/Accessibility/AXNodeObject.hpp"

namespace krys::boo
{
  class AXSVGObject;

  /// @brief An accessibility object for a renderer: what the renderer adds to its node's object (whether
  /// it's in the tree given how it's rendered, its role from the kind of renderer, and its bounds). Its
  /// children still come from the DOM, apart from what only the render tree has (list markers).
  class AXRenderObject : public AXNodeObject
  {
  private:
    WeakPtr<RenderObject> _renderer;
    /// @brief Whether a `treeitem` is in a tree (one that isn't loses its role).
    bool _isValidTreeItem {false};

  public:
    /// @brief An object for @p renderer, and its node if it has one.
    KRYS_NODISCARD static Ref<AXRenderObject> Create(AXId id, RenderObject &renderer,
                                                     AXObjectCache &cache) noexcept;

    /// @brief An object for @p node with no renderer yet (`display: contents` and so on).
    KRYS_NODISCARD static Ref<AXRenderObject> Create(AXId id, dom::Node &node, AXObjectCache &cache) noexcept;

    ~AXRenderObject() noexcept override;

    KRYS_NODISCARD bool IsDetached() const noexcept override
    {
      return !_renderer && AXNodeObject::IsDetached();
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

    KRYS_NODISCARD AXObject *ParentObject() const noexcept override;

    void AddChildren() noexcept override;

    KRYS_NODISCARD dom::DOMString StringValue() const noexcept override;

    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

    KRYS_NODISCARD bool IsOffScreen() const noexcept override;

    KRYS_NODISCARD AXObject *AccessibilityHitTest(const IntPoint &point) const noexcept override;

    KRYS_NODISCARD AXObject *ElementAccessibilityHitTest(const IntPoint &point) const noexcept override;

    /// @brief The object of the root of the SVG document this image shows, in that document's cache (made if
    /// @p create and it doesn't exist yet); null if it isn't showing an SVG image.
    KRYS_NODISCARD AXSVGObject *RemoteSVGRoot(bool create = true) const noexcept;

    using AXNodeObject::SetValue;

    bool SetValue(dom::DOMStringView value) noexcept override;

    KRYS_NODISCARD bool IsIgnoredElementWithinMathTree() const noexcept override;

  protected:
    AXRenderObject(AXId id, RenderObject *renderer, dom::Node *node, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AXRole DetermineAccessibilityRole() noexcept override;

    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;

    void DetachRemoteParts(AXDetachmentType detachmentType) noexcept override;

  private:
    /// @brief The control of the label the object is in (not the control itself), if any.
    KRYS_NODISCARD AXObject *ControlForLabelElement() const noexcept;

    /// @brief The tree the object is in, if any.
    KRYS_NODISCARD AXObject *ContainingTree() const noexcept;

    /// @brief Whether the object can be in the tree it's in: tree items, their descendants and static text.
    KRYS_NODISCARD bool IsAllowedChildOfTree() const noexcept;

    /// @brief Whether an exposed row is above the object, with no cell or table between: what's between a
    /// row and its cells is ignored.
    KRYS_NODISCARD bool IgnoredByRowAncestor() const noexcept;

    /// @brief An image's map's areas that are links.
    void AddImageMapChildren() noexcept;

    /// @brief A number input's spin button.
    void AddTextFieldChildren() noexcept;

    /// @brief The root of the SVG document an image shows.
    void AddRemoteSVGChildren() noexcept;

    /// @brief Moves @p rect, in an SVG image's document's coordinates, to where the image showing it is.
    void OffsetForRemoteSVGElement(LayoutRect &rect) const noexcept;

    /// @brief Updates the roles that depend on the children (a menu with no items, a list).
    void UpdateRoleAfterChildrenCreation() noexcept;

    /// @brief The list item's marker renderer, if this is an unignored list item with one.
    KRYS_NODISCARD RenderObject *MarkerRenderer() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::AXRenderObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::AXObject &object) noexcept
  {
    return object.IsAccessibilityRenderObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
