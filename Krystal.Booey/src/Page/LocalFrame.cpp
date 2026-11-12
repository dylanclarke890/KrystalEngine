/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2005 Alexey Proskuryakov <ap@nypop.com>
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008 Google Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  LocalFrame::LocalFrame(Page *page, UniquePtr<LocalFrameLoaderClient> &&loaderClient) noexcept
      : _page(CreateWeakPtr(page)), _eventHandler(CreateUnique<EventHandler>(*this)),
        _loader(CreateUnique<FrameLoader>(*this, krys::move(loaderClient)))
  {
  }

  LocalFrame::~LocalFrame() noexcept
  {
    if (_document)
    {
      _document->WillBeRemovedFromFrame();
    }
  }

  Ref<LocalFrame> LocalFrame::Create(Page *page, UniquePtr<LocalFrameLoaderClient> loaderClient) noexcept
  {
    return AdoptRef(*new LocalFrame(page, krys::move(loaderClient)));
  }

  void LocalFrame::SetView(RefPtr<LocalFrameView> &&view) noexcept
  {
    if (_view)
    {
      _view->LayoutContext().UnscheduleLayout();
    }

    // The document's render tree belongs to the view it's laid out in.
    if (_document && _document->HasLivingRenderTree())
    {
      _document->DestroyRenderTree();
    }

    _view = krys::move(view);

    if (_document && _view)
    {
      _document->CreateRenderTree();
    }
  }

  void LocalFrame::CreateView(const IntSize &viewportSize, const IntSize &fixedLayoutSize,
                              bool useFixedLayout) noexcept
  {
    SetView(nullptr);

    Ref frameView = LocalFrameView::Create(*this, viewportSize);
    frameView->SetFixedLayoutSize(fixedLayoutSize);
    frameView->SetUseFixedLayout(useFixedLayout);

    SetView(krys::move(frameView));
  }

  VisiblePosition LocalFrame::VisiblePositionForPoint(const IntPoint &framePoint) const noexcept
  {
    HitTestResult result = GetEventHandler().HitTestResultAtPoint(
      LayoutPoint(framePoint), {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                                HitTestRequest::Type::AllowVisibleChildFrameContentOnly});
    RefPtr<dom::Node> node = ShareRefPtr(result.InnerNonSharedNode());
    if (!node)
    {
      return {};
    }
    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return {};
    }
    auto visiblePos = renderer->VisiblePositionForPoint(result.LocalPoint(), HitTestSource::User);
    if (visiblePos.IsNull())
    {
      visiblePos = VisiblePosition(FirstPositionInOrBeforeNode(node.get()));
    }
    return visiblePos;
  }

  dom::Document *LocalFrame::DocumentAtPoint(const IntPoint &point) noexcept
  {
    if (!View())
    {
      return nullptr;
    }

    IntPoint pt = View()->WindowToContents(point);
    HitTestResult result = HitTestResult(LayoutPoint(pt));

    if (Document() && Document()->GetRenderView())
    {
      result = GetEventHandler().HitTestResultAtPoint(
        LayoutPoint(pt),
        {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
         HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent});
    }
    return result.InnerNode() ? &result.InnerNode()->NodeDocument() : nullptr;
  }

  void LocalFrame::SetDocument(RefPtr<dom::Document> &&newDocument) noexcept
  {
    krys_assert_msg(!newDocument || !newDocument->Frame(), "a document is in one frame at most");

    if (_documentIsBeingReplaced)
    {
      return;
    }

    SetForScope documentIsBeingReplaced(_documentIsBeingReplaced, true);

    if (auto *page = GetPage(); page && &page->MainFrame() == this)
    {
      page->DidChangeMainDocument(newDocument.get());
    }

    // The mouse's state refers to the old document's nodes.
    _eventHandler->Clear();

    if (RefPtr<dom::Document> previousDocument = _document)
    {
      previousDocument->WillBeRemovedFromFrame();
    }

    _document = newDocument;

    // Don't use _document because it can be overwritten and we want to guarantee
    // that the document is not destroyed during this function call.
    if (newDocument)
    {
      newDocument->DidBecomeCurrentDocumentInFrame(*this);
    }
  }

  float LocalFrame::FrameScaleFactor() const noexcept
  {
    auto *page = GetPage();
    if (!page)
    {
      return 1;
    }

    // Main frame is scaled with respect to the container.
    return page->PageScaleFactor();
  }

  void LocalFrame::DeviceOrPageScaleFactorChanged() noexcept
  {
    if (auto *root = _document ? _document->GetRenderView() : nullptr)
    {
      root->Compositor().DeviceOrPageScaleFactorChanged();
    }
  }

  void LocalFrame::SetPageZoomFactor(float factor) noexcept
  {
    SetPageAndTextZoomFactors(factor, _textZoomFactor);
  }

  void LocalFrame::SetTextZoomFactor(float factor) noexcept
  {
    SetPageAndTextZoomFactors(_pageZoomFactor, factor);
  }

  void LocalFrame::SetPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor) noexcept
  {
    if (_pageZoomFactor == pageZoomFactor && _textZoomFactor == textZoomFactor)
    {
      return;
    }

    _pageZoomFactor = pageZoomFactor;
    _textZoomFactor = textZoomFactor;

    RefPtr<dom::Document> document = _document;
    if (!document)
    {
      return;
    }

    document->ResolveStyle(dom::Document::ResolveStyleType::Rebuild);

    if (RefPtr<LocalFrameView> view = _view)
    {
      auto &layoutContext = view->LayoutContext();
      if (document->GetRenderView() && layoutContext.NeedsLayout() && layoutContext.DidFirstLayout())
      {
        layoutContext.Layout();
      }
    }
  }
}
