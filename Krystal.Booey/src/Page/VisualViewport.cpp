#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

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
        // There's no pinch zoom, so the visual viewport is the layout viewport: the visible content, less the
        // scrollbars.
        auto visualViewportRect = FloatRect(view->VisibleContentRect());
        auto layoutViewportRect = visualViewportRect;
        auto pageZoomFactor = frame->PageZoomFactor();
        krys_debug_assert(pageZoomFactor);
        offsetLeft = (visualViewportRect.X() - layoutViewportRect.X()) / pageZoomFactor;
        offsetTop = (visualViewportRect.Y() - layoutViewportRect.Y()) / pageZoomFactor;
        _pageLeft = visualViewportRect.X() / pageZoomFactor;
        _pageTop = visualViewportRect.Y() / pageZoomFactor;
        width = visualViewportRect.Width() / pageZoomFactor;
        height = visualViewportRect.Height() / pageZoomFactor;
      }
      // FIXME: The page scale factor, once pages can be pinch-zoomed.
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
