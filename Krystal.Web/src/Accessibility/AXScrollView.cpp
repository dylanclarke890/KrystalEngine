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

#include "Krystal.Web/Accessibility/AXScrollView.hpp"
#include "Krystal.Web/Accessibility/AXObjectCache.hpp"
#include "Krystal.Web/Accessibility/AXScrollbar.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Platform/Scrolling/Scrollbar.hpp"

namespace krys::web
{
  AXScrollView::AXScrollView(AXId id, FrameView &view, AXObjectCache &cache) noexcept
      : AXObject(id, cache), _scrollView(CreateWeakPtr<ScrollableArea>(&view))
  {
  }

  AXScrollView::~AXScrollView() noexcept
  {
    krys_debug_assert(IsDetached());
  }

  Ref<AXScrollView> AXScrollView::Create(AXId id, FrameView &view, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AXScrollView(id, view, cache));
  }

  bool AXScrollView::IsDetached() const noexcept
  {
    return !_scrollView;
  }

  FrameView *AXScrollView::ScrollView() const noexcept
  {
    return static_cast<FrameView *>(_scrollView.get());
  }

  Document *AXScrollView::GetDocument() const noexcept
  {
    auto *view = ScrollView();
    return view ? view->Frame().Document() : nullptr;
  }

  AXObject *AXScrollView::ParentObject() const noexcept
  {
    // A view is the top of its document's tree: with no subframes, it's never in another document's.
    return nullptr;
  }

  AXObject *AXScrollView::WebAreaObject() const noexcept
  {
    auto *document = GetDocument();
    auto *cache = AxObjectCache();
    if (!document || !document->GetRenderView() || !cache)
    {
      return nullptr;
    }
    return cache->GetOrCreate(*document);
  }

  AXObject *AXScrollView::ScrollBar(AXOrientation orientation) noexcept
  {
    UpdateScrollbars();
    switch (orientation)
    {
      // "Elements with the role scrollbar have an implicit aria-orientation value of vertical"
      // (https://w3c.github.io/aria/#scrollbar).
      case AXOrientation::Undefined:
      case AXOrientation::Vertical:   return _verticalScrollbar.get();
      case AXOrientation::Horizontal: return _horizontalScrollbar.get();
    }
    return nullptr;
  }

  bool AXScrollView::IsFocused() const noexcept
  {
    auto *webArea = WebAreaObject();
    return webArea && webArea->IsFocused();
  }

  bool AXScrollView::CanSetFocusAttribute() const noexcept
  {
    auto *webArea = WebAreaObject();
    return webArea && webArea->CanSetFocusAttribute();
  }

  AXObject *AXScrollView::AccessibilityHitTest(const PointI &point) const noexcept
  {
    auto *webArea = WebAreaObject();
    if (!webArea)
    {
      return nullptr;
    }
    for (const auto &scrollbar : {_horizontalScrollbar, _verticalScrollbar})
    {
      if (scrollbar && scrollbar->ElementRect().Contains(LayoutPoint(point)))
      {
        return scrollbar.get();
      }
    }
    return webArea->AccessibilityHitTest(point);
  }

  LayoutRect AXScrollView::ElementRect() const noexcept
  {
    auto *view = ScrollView();
    return view ? LayoutRect(view->FrameRect()) : LayoutRect();
  }

  void AXScrollView::UpdateChildrenIfNecessary() noexcept
  {
    // Always worked out again: a new web area can have been made for the view (when going through the
    // history), and there are only ever a few children.
    ClearChildren();
    AddChildren();
  }

  void AXScrollView::ClearChildren() noexcept
  {
    // The scroll bars' objects are kept, to be added again.
    AXObject::ClearChildren();
    _childrenDirty = false;
  }

  void AXScrollView::AddChildren() noexcept
  {
    krys_debug_assert(!_childrenInitialized);
    _childrenInitialized = true;
    AddChild(WebAreaObject());
    UpdateScrollbars();
  }

  void AXScrollView::UpdateScrollbars() noexcept
  {
    auto *view = ScrollView();
    if (!view)
    {
      return;
    }
    UpdateScrollbar(_horizontalScrollbar, view->HorizontalScrollbar());
    UpdateScrollbar(_verticalScrollbar, view->VerticalScrollbar());
  }

  void AXScrollView::UpdateScrollbar(RefPtr<AXObject> &object, Scrollbar *scrollbar) noexcept
  {
    auto *existing = DynamicDowncast<AXScrollbar>(object.get());
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
      existing = DynamicDowncast<AXScrollbar>(cache->Create(*scrollbar));
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

  void AXScrollView::RemoveScrollbar(RefPtr<AXObject> &object) noexcept
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

  bool AXScrollView::ComputeIsIgnored() const noexcept
  {
    auto *webArea = WebAreaObject();
    return !webArea || webArea->IsIgnored();
  }

  void AXScrollView::DetachRemoteParts(AXDetachmentType detachmentType) noexcept
  {
    AXObject::DetachRemoteParts(detachmentType);
    RemoveScrollbar(_horizontalScrollbar);
    RemoveScrollbar(_verticalScrollbar);
    _scrollView = nullptr;
  }
}
