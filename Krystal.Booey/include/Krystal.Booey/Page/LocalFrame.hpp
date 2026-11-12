/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
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

#pragma once

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Booey/Loader/Cache/CachePolicy.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Page;
  class VisiblePosition;

  /// @brief Shows a document in a view: the host creates a frame, gives it a view (`CreateView`) and a
  /// document (`SetDocument`), and sets the zoom factors and the media environment.
  /// @note FIXME: Only what layout, style and navigation within a document need: no document loading, frame
  /// tree, scripting or printing. The media environment stands in for the page's chrome client and the
  /// screen.
  class LocalFrame final : public RefCounted<LocalFrame>, public CanMakeWeakPtr<LocalFrame>
  {
    KRYS_NON_COPY_MOVABLE(LocalFrame);

  private:
    WeakPtr<Page> _page;
    RefPtr<dom::Document> _document;
    RefPtr<LocalFrameView> _view;
    UniquePtr<EventHandler> _eventHandler;
    UniquePtr<FrameLoader> _loader;
    float _pageZoomFactor {1};
    float _textZoomFactor {1};
    CachePolicy _subresourceCachePolicy {CachePolicy::Use};
    bool _documentIsBeingReplaced {false};

    LocalFrame(Page *page, UniquePtr<LocalFrameLoaderClient> &&loaderClient) noexcept;

  public:
    ~LocalFrame() noexcept;

    /// @brief A frame, the main frame of @p page if given.
    /// @param loaderClient The host's side of the frame's loader; without one, the frame ignores navigations.
    KRYS_NODISCARD static Ref<LocalFrame>
      Create(Page *page = nullptr, UniquePtr<LocalFrameLoaderClient> loaderClient = nullptr) noexcept;

    /// @brief Loads documents into the frame; links and form submissions go through it to the host.
    KRYS_NODISCARD FrameLoader &Loader() const noexcept
    {
      return *_loader;
    }

    /// @brief How the document's subresources use the memory cache, unless the load is a reload: the host
    /// sets `Reload` when it loads a page again, so edited files load again.
    KRYS_NODISCARD CachePolicy SubresourceCachePolicy() const noexcept
    {
      return _subresourceCachePolicy;
    }
    void SetSubresourceCachePolicy(CachePolicy policy) noexcept
    {
      _subresourceCachePolicy = policy;
    }

    /// @brief The page the frame is in, if any.
    KRYS_NODISCARD Page *GetPage() const noexcept
    {
      return _page.get();
    }

    /// @brief The document the frame shows, if any.
    KRYS_NODISCARD dom::Document *Document() const noexcept
    {
      return _document.get();
    }

    /// @brief The window of the frame's document, if any.
    KRYS_NODISCARD LocalDOMWindow *Window() const noexcept
    {
      return _document ? _document->Window() : nullptr;
    }

    /// @brief The view the frame's document is laid out in, if any.
    KRYS_NODISCARD LocalFrameView *View() const noexcept
    {
      return _view.get();
    }

    /// @brief Replaces the frame's view. The document's render tree is destroyed with the old view and
    /// created with the new one.
    /// @brief Turns the host's mouse input into DOM events for this frame.
    KRYS_NODISCARD EventHandler &GetEventHandler() const noexcept
    {
      return *_eventHandler;
    }

    void SetView(RefPtr<LocalFrameView> &&view) noexcept;

    /// @brief Gives the frame a new view of @p viewportSize, laid out at @p fixedLayoutSize if
    /// @p useFixedLayout.
    void CreateView(const IntSize &viewportSize, const IntSize &fixedLayoutSize = {},
                    bool useFixedLayout = false) noexcept;

    /// @brief Shows @p newDocument in the frame, in place of its current document. @p newDocument must not be
    /// in a frame already.
    void SetDocument(RefPtr<dom::Document> &&newDocument) noexcept;

    /// @brief The caret position nearest @p framePoint (in the frame's contents).
    KRYS_NODISCARD VisiblePosition VisiblePositionForPoint(const IntPoint &framePoint) const noexcept;
    /// @brief The document under @p point (in the window).
    KRYS_NODISCARD dom::Document *DocumentAtPoint(const IntPoint &point) noexcept;

    /// @brief The zoom applied to the whole document (the root style's `zoom`).
    KRYS_NODISCARD float PageZoomFactor() const noexcept
    {
      return _pageZoomFactor;
    }

    /// @brief The zoom applied to font sizes, on top of the page zoom.
    KRYS_NODISCARD float TextZoomFactor() const noexcept
    {
      return _textZoomFactor;
    }

    void SetPageZoomFactor(float factor) noexcept;
    void SetTextZoomFactor(float factor) noexcept;
    void SetPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor) noexcept;

    /// @brief How much the frame's content is scaled: the page's scale.
    KRYS_NODISCARD float FrameScaleFactor() const noexcept;
    /// @brief Tells the frame's compositor the device or page scale changed.
    void DeviceOrPageScaleFactorChanged() noexcept;
  };
}
