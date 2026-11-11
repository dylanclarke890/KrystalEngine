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

#include "Krystal.Booey/Accessibility/AccessibilityScrollView.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityScrollbar.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"

namespace krys::boo::ax
{
  AccessibilityScrollView::AccessibilityScrollView(AXID id, LocalFrameView &view,
                                                   AXObjectCache &cache) noexcept
      : AccessibilityObject(id, cache), _scrollView(CreateWeakPtr<ScrollableArea>(&view))
  {
  }

  AccessibilityScrollView::~AccessibilityScrollView() noexcept
  {
    krys_debug_assert(IsDetached());
  }

  Ref<AccessibilityScrollView> AccessibilityScrollView::Create(AXID id, LocalFrameView &view,
                                                               AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityScrollView(id, view, cache));
  }

  bool AccessibilityScrollView::IsDetached() const noexcept
  {
    return !_scrollView;
  }

  LocalFrameView *AccessibilityScrollView::ScrollView() const noexcept
  {
    return static_cast<LocalFrameView *>(_scrollView.get());
  }

  bool AccessibilityScrollView::IsRoot() const noexcept
  {
    // The root of this cache's frame (each document has its own cache).
    auto *cache = AxObjectCache();
    return cache && GetDocument() == &cache->GetDocument();
  }

  dom::Document *AccessibilityScrollView::GetDocument() const noexcept
  {
    auto *view = ScrollView();
    return view ? view->Frame().Document() : nullptr;
  }

  AccessibilityObject *AccessibilityScrollView::ParentObject() const noexcept
  {
    // FIXME: A frame's view's parent is its owner element, in the parent document's tree
    // (crossFrameParentObject), with frames.
    return nullptr;
  }

  AccessibilityObject *AccessibilityScrollView::WebAreaObject() const noexcept
  {
    auto *document = GetDocument();
    auto *cache = AxObjectCache();
    if (!document || !document->GetRenderView() || !cache)
    {
      return nullptr;
    }
    return cache->GetOrCreate(*document);
  }

  AccessibilityObject *AccessibilityScrollView::ScrollBar(AccessibilityOrientation orientation) noexcept
  {
    UpdateScrollbars();
    switch (orientation)
    {
      // "Elements with the role scrollbar have an implicit aria-orientation value of vertical"
      // (https://w3c.github.io/aria/#scrollbar).
      case AccessibilityOrientation::Undefined:
      case AccessibilityOrientation::Vertical:   return _verticalScrollbar.get();
      case AccessibilityOrientation::Horizontal: return _horizontalScrollbar.get();
    }
    return nullptr;
  }

  bool AccessibilityScrollView::IsFocused() const noexcept
  {
    auto *webArea = WebAreaObject();
    return webArea && webArea->IsFocused();
  }

  bool AccessibilityScrollView::CanSetFocusAttribute() const noexcept
  {
    auto *webArea = WebAreaObject();
    return webArea && webArea->CanSetFocusAttribute();
  }

  LayoutRect AccessibilityScrollView::ElementRect() const noexcept
  {
    auto *view = ScrollView();
    return view ? LayoutRect(view->FrameRect()) : LayoutRect();
  }

  void AccessibilityScrollView::UpdateChildrenIfNecessary() noexcept
  {
    // Always worked out again: a new web area can have been made for the view (when going through the
    // history), and there are only ever a few children.
    ClearChildren();
    AddChildren();
  }

  void AccessibilityScrollView::ClearChildren() noexcept
  {
    // The scroll bars' objects are kept, to be added again.
    AccessibilityObject::ClearChildren();
    _childrenDirty = false;
  }

  void AccessibilityScrollView::AddChildren() noexcept
  {
    krys_debug_assert(!_childrenInitialized);
    _childrenInitialized = true;
    // FIXME: A frame's view (not the root) has its frame's root as its child (addLocalFrameChild), with
    // frames.
    if (IsRoot())
    {
      AddChild(WebAreaObject());
    }
    UpdateScrollbars();
  }

  void AccessibilityScrollView::UpdateScrollbars() noexcept
  {
    auto *view = ScrollView();
    if (!view)
    {
      return;
    }
    // FIXME: A view in a hidden web area (isWithinHiddenWebArea) has no scroll bars, with frames.
    if (!IsRoot())
    {
      RemoveScrollbar(_horizontalScrollbar);
      RemoveScrollbar(_verticalScrollbar);
      return;
    }
    UpdateScrollbar(_horizontalScrollbar, view->HorizontalScrollbar());
    UpdateScrollbar(_verticalScrollbar, view->VerticalScrollbar());
  }

  void AccessibilityScrollView::UpdateScrollbar(RefPtr<AccessibilityObject> &object,
                                                Scrollbar *scrollbar) noexcept
  {
    auto *existing = DynamicDowncast<AccessibilityScrollbar>(object.get());
    if (existing && &existing->GetScrollbar() != scrollbar)
    {
      RemoveScrollbar(object);
      existing = nullptr;
    }
    auto *cache = AxObjectCache();
    if (!scrollbar || !cache)
    {
      return;
    }
    if (!existing)
    {
      existing = DynamicDowncast<AccessibilityScrollbar>(cache->Create(*scrollbar));
      object = ShareRefPtr(existing);
    }
    if (!existing)
    {
      return;
    }
    existing->SetParent(this);
    if (std::ranges::none_of(_children, [&](const auto &child) { return child.get() == existing; }))
    {
      AddChild(existing);
    }
  }

  void AccessibilityScrollView::RemoveScrollbar(RefPtr<AccessibilityObject> &object) noexcept
  {
    auto scrollbar = std::exchange(object, nullptr);
    if (!scrollbar)
    {
      return;
    }
    auto position =
      std::ranges::find_if(_children, [&](const auto &child) { return child.get() == scrollbar.get(); });
    if (position != _children.end())
    {
      _children.erase(position);
      ResetChildrenIndexInParent();
    }
    scrollbar->DetachFromParent();
    if (auto *cache = AxObjectCache())
    {
      cache->Remove(scrollbar->ObjectID());
    }
  }

  bool AccessibilityScrollView::ComputeIsIgnored() const noexcept
  {
    // FIXME: A frame's view (not the root) is ignored in favour of its frame's root, with frames.
    auto *webArea = WebAreaObject();
    return !webArea || webArea->IsIgnored();
  }

  void AccessibilityScrollView::DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept
  {
    AccessibilityObject::DetachRemoteParts(detachmentType);
    RemoveScrollbar(_horizontalScrollbar);
    RemoveScrollbar(_verticalScrollbar);
    _scrollView = nullptr;
  }
}
