#pragma once

#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo
{
  class RenderStyle;
  enum class TextDirection : bool;
}

namespace krys::boo::html
{
  class HTMLCanvasElement;

  /// @brief A `<canvas>`'s 2D context, `GetContext(u8"2d")`.
  /// @see https://html.spec.whatwg.org/#canvasrenderingcontext2d
  // FIXME: `drawFocusIfNeeded` (B14e), and the text methods and `font` (B14d).
  class CanvasRenderingContext2D final : public CanvasRenderingContext2DBase
  {
  public:
    KRYS_NODISCARD static UniquePtr<CanvasRenderingContext2D>
      Create(CanvasBase &, CanvasRenderingContext2DSettings &&, bool usesCSSCompatibilityParseMode) noexcept;

    CanvasRenderingContext2D(CanvasBase &, CanvasRenderingContext2DSettings &&,
                             bool usesCSSCompatibilityParseMode) noexcept;
    ~CanvasRenderingContext2D() noexcept override;

    KRYS_NODISCARD HTMLCanvasElement &Canvas() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-direction
    KRYS_NODISCARD CanvasDirection GetDirection() const noexcept;

  private:
    KRYS_NODISCARD TextDirection ToTextDirection(Direction,
                                                 const RenderStyle **computedStyle = nullptr) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(krys::boo::html::CanvasRenderingContext2D, Is2d())
