#pragma once

#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LocalDOMWindow;
  class LocalFrame;

  /// @brief The part of the page the user sees: the layout viewport, as pages aren't pinch-zoomed. It fires
  /// `resize` as its size changes, and `scroll` as it moves within the layout viewport (never, at scale 1).
  /// @see https://drafts.csswg.org/cssom-view/#the-visualviewport-interface
  class VisualViewport final : public dom::EventTarget
  {
    KRYS_NON_COPY_MOVABLE(VisualViewport);

  private:
    WeakPtr<LocalDOMWindow> _window;
    double _offsetLeft {0};
    double _offsetTop {0};
    double _pageLeft {0};
    double _pageTop {0};
    double _width {0};
    double _height {0};
    double _scale {1};

    explicit VisualViewport(LocalDOMWindow &window) noexcept;

  public:
    KRYS_NODISCARD static Ref<VisualViewport> Create(LocalDOMWindow &window) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-offsetleft
    KRYS_NODISCARD double OffsetLeft() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-offsettop
    KRYS_NODISCARD double OffsetTop() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-pageleft
    KRYS_NODISCARD double PageLeft() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-pagetop
    KRYS_NODISCARD double PageTop() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-width
    KRYS_NODISCARD double Width() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-height
    KRYS_NODISCARD double Height() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-visualviewport-scale
    KRYS_NODISCARD double Scale() const noexcept;

    /// @brief Takes the viewport's geometry from the frame's view, and has the document fire `resize` or
    /// `scroll` at its next rendering update if it changed.
    void Update() noexcept;

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;

    void UpdateFrameLayout() const noexcept;
  };
}
