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

#include "Krystal.Booey/Accessibility/AXObject.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class ScrollableArea;
  class Scrollbar;

  /// @brief A frame's view: the root of its document's tree, above the web area and the view's scroll bars.
  class AXScrollView final : public AXObject
  {
  private:
    // The view (a FrameView, which has two weak pointer factories, so the ScrollableArea one is used).
    WeakPtr<ScrollableArea> _scrollView;
    RefPtr<AXObject> _horizontalScrollbar;
    RefPtr<AXObject> _verticalScrollbar;
    bool _childrenDirty {false};

  public:
    KRYS_NODISCARD static Ref<AXScrollView> Create(AXId id, FrameView &view, AXObjectCache &cache) noexcept;

    ~AXScrollView() noexcept override;

    KRYS_NODISCARD bool IsDetached() const noexcept override;

    KRYS_NODISCARD bool IsScrollView() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD FrameView *ScrollView() const noexcept;

    /// @brief The view's document's web area.
    KRYS_NODISCARD AXObject *WebAreaObject() const noexcept;

    /// @brief The view's scroll bar running in @p orientation (vertical if undefined), if it has one.
    KRYS_NODISCARD AXObject *ScrollBar(AXOrientation orientation) noexcept;

    KRYS_NODISCARD Document *GetDocument() const noexcept override;

    KRYS_NODISCARD AXObject *ParentObject() const noexcept override;

    KRYS_NODISCARD bool IsEnabled() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool IsFocused() const noexcept override;

    KRYS_NODISCARD bool CanSetFocusAttribute() const noexcept override;

    KRYS_NODISCARD LayoutRect ElementRect() const noexcept override;

    KRYS_NODISCARD AXObject *AccessibilityHitTest(const PointI &point) const noexcept override;

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
    AXScrollView(AXId id, FrameView &view, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AXRole DetermineAccessibilityRole() noexcept override
    {
      return AXRole::ScrollArea;
    }

    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;

    void DetachRemoteParts(AXDetachmentType detachmentType) noexcept override;

    /// @brief Adds the view's scroll bars as children, making or removing their objects as the view gains or
    /// loses them.
    void UpdateScrollbars() noexcept;

    void UpdateScrollbar(RefPtr<AXObject> &object, Scrollbar *scrollbar) noexcept;

    void RemoveScrollbar(RefPtr<AXObject> &object) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::AXScrollView)
  KRYS_NODISCARD static bool IsType(const krys::boo::AXObject &object) noexcept
  {
    return object.IsScrollView();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
