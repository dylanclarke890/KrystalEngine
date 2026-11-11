/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class ScrollableArea;
}

namespace krys::boo::ax
{
  /// @brief A frame's view: the root of its document's tree, above the web area and the view's scroll bars.
  class AccessibilityScrollView final : public AccessibilityObject
  {
  private:
    // The view (a LocalFrameView, which has two weak pointer factories, so the ScrollableArea one is used).
    WeakPtr<ScrollableArea> _scrollView;
    RefPtr<AccessibilityObject> _horizontalScrollbar;
    RefPtr<AccessibilityObject> _verticalScrollbar;
    bool _childrenDirty {false};

  public:
    KRYS_NODISCARD static Ref<AccessibilityScrollView> Create(AXID id, LocalFrameView &view,
                                                              AXObjectCache &cache) noexcept;
    ~AccessibilityScrollView() noexcept override;

    KRYS_NODISCARD bool IsDetached() const noexcept override;
    KRYS_NODISCARD bool IsScrollView() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD LocalFrameView *ScrollView() const noexcept;
    /// @brief The view's document's web area.
    KRYS_NODISCARD AccessibilityObject *WebAreaObject() const noexcept;
    /// @brief The view's scroll bar running in @p orientation (vertical if undefined), if it has one.
    KRYS_NODISCARD AccessibilityObject *ScrollBar(AccessibilityOrientation orientation) noexcept;
    /// @brief Whether this is the view of the cache's document (rather than of a frame in it).
    KRYS_NODISCARD bool IsRoot() const noexcept;

    KRYS_NODISCARD dom::Document *GetDocument() const noexcept override;
    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override;
    KRYS_NODISCARD bool IsEnabled() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool IsFocused() const noexcept override;
    KRYS_NODISCARD bool CanSetFocusAttribute() const noexcept override;
    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

    void AddChildren() noexcept override;
    void ClearChildren() noexcept override;
    void UpdateChildrenIfNecessary() noexcept override;
    void SetNeedsToUpdateChildren() noexcept override
    {
      _childrenDirty = true;
    }
    KRYS_NODISCARD bool NeedsToUpdateChildren() const noexcept override
    {
      return _childrenDirty;
    }

  private:
    AccessibilityScrollView(AXID id, LocalFrameView &view, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override
    {
      return AccessibilityRole::ScrollArea;
    }
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;
    void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept override;

    /// @brief Adds the view's scroll bars as children, making or removing their objects as the view gains or
    /// loses them.
    void UpdateScrollbars() noexcept;
    void UpdateScrollbar(RefPtr<AccessibilityObject> &object, Scrollbar *scrollbar) noexcept;
    void RemoveScrollbar(RefPtr<AccessibilityObject> &object) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityScrollView)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsScrollView();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
