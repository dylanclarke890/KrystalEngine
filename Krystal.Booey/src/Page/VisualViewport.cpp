/*
 * Copyright (C) 2017 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"

namespace krys::boo
{
  VisualViewport::VisualViewport(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  Ref<VisualViewport> VisualViewport::Create(LocalDOMWindow &window) noexcept
  {
    return AdoptRef(*new VisualViewport(window));
  }

  LocalFrame *VisualViewport::Frame() const noexcept
  {
    auto *window = _window.get();
    return window ? window->Frame() : nullptr;
  }

  void VisualViewport::UpdateFrameLayout() const noexcept
  {
    krys_debug_assert(Frame());
    (void)Frame()->Document()->UpdateLayoutIgnorePendingStylesheets();
  }

  double VisualViewport::OffsetLeft() const noexcept
  {
    if (!Frame())
    {
      return 0;
    }
    UpdateFrameLayout();
    return _offsetLeft;
  }

  double VisualViewport::OffsetTop() const noexcept
  {
    if (!Frame())
    {
      return 0;
    }
    UpdateFrameLayout();
    return _offsetTop;
  }

  double VisualViewport::PageLeft() const noexcept
  {
    if (!Frame())
    {
      return 0;
    }
    UpdateFrameLayout();
    return _pageLeft;
  }

  double VisualViewport::PageTop() const noexcept
  {
    if (!Frame())
    {
      return 0;
    }
    UpdateFrameLayout();
    return _pageTop;
  }

  double VisualViewport::Width() const noexcept
  {
    if (!Frame())
    {
      return 0;
    }
    UpdateFrameLayout();
    return _width;
  }

  double VisualViewport::Height() const noexcept
  {
    if (!Frame())
    {
      return 0;
    }
    UpdateFrameLayout();
    return _height;
  }

  double VisualViewport::Scale() const noexcept
  {
    if (!Frame())
    {
      return 1;
    }
    UpdateFrameLayout();
    return _scale;
  }

  void VisualViewport::Update() noexcept
  {
    double offsetLeft = 0;
    double offsetTop = 0;
    _pageLeft = 0;
    _pageTop = 0;
    double width = 0;
    double height = 0;
    double scale = 1;

    auto *frame = Frame();
    if (frame)
    {
      if (auto *view = frame->View())
      {
        // The visual viewport is the visible content, less the scrollbars, unscaled.
        // FIXME: The header and footer (visibleDocumentRect), once there are those.
        auto visualViewportRect = FloatRect(view->VisibleContentRect());
        visualViewportRect.Scale(1 / view->FrameScaleFactor());
        auto layoutViewportRect = FloatRect(view->LayoutViewportRect());
        auto pageZoomFactor = frame->PageZoomFactor();
        krys_debug_assert(pageZoomFactor);
        offsetLeft = (visualViewportRect.X() - layoutViewportRect.X()) / pageZoomFactor;
        offsetTop = (visualViewportRect.Y() - layoutViewportRect.Y()) / pageZoomFactor;
        _pageLeft = visualViewportRect.X() / pageZoomFactor;
        _pageTop = visualViewportRect.Y() / pageZoomFactor;
        width = visualViewportRect.Width() / pageZoomFactor;
        height = visualViewportRect.Height() / pageZoomFactor;
      }
      if (auto *page = frame->GetPage())
      {
        // FIXME: Divided by the chrome client's base viewport layout size scale factor, once it has one.
        scale = page->PageScaleFactor();
      }
    }

    auto *document = frame ? frame->Document() : nullptr;
    if (_offsetLeft != offsetLeft || _offsetTop != offsetTop)
    {
      if (document)
      {
        document->SetNeedsVisualViewportScrollEvent();
      }
      _offsetLeft = offsetLeft;
      _offsetTop = offsetTop;
    }

    if (_width != width || _height != height)
    {
      if (document)
      {
        document->SetNeedsVisualViewportResize();
      }
      _width = width;
      _height = height;
      _scale = scale;
    }
  }
}
