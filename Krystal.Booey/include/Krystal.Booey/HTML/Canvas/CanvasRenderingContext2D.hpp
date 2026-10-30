#pragma once

#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo
{
  class Path;
  class RenderStyle;
  enum class TextDirection : bool;
}

namespace krys::boo::html
{
  class HTMLCanvasElement;
  class Path2D;

  /// @brief A `<canvas>`'s 2D context, `GetContext(u8"2d")`.
  /// @see https://html.spec.whatwg.org/#canvasrenderingcontext2d
  class CanvasRenderingContext2D final : public CanvasRenderingContext2DBase
  {
  public:
    KRYS_NODISCARD static UniquePtr<CanvasRenderingContext2D>
      Create(CanvasBase &, CanvasRenderingContext2DSettings &&, bool usesCSSCompatibilityParseMode) noexcept;

    CanvasRenderingContext2D(CanvasBase &, CanvasRenderingContext2DSettings &&,
                             bool usesCSSCompatibilityParseMode) noexcept;
    ~CanvasRenderingContext2D() noexcept override;

    KRYS_NODISCARD HTMLCanvasElement &Canvas() const noexcept;

    /// @brief Sets the font, a CSS `font` value relative to the canvas's font; ignored if it isn't one.
    /// @see https://html.spec.whatwg.org/#dom-context-2d-font
    void SetFont(dom::DOMStringView) noexcept;

    /// @brief Draws a focus ring along the current path if @p element, which must be the canvas's fallback
    /// content, has the focus.
    /// @see https://html.spec.whatwg.org/#dom-context-2d-drawfocusifneeded
    void DrawFocusIfNeeded(dom::Element &element) noexcept;
    /// @brief Draws a focus ring along @p path if @p element, which must be the canvas's fallback content,
    /// has the focus.
    void DrawFocusIfNeeded(Path2D &path, dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-direction
    KRYS_NODISCARD CanvasDirection GetDirection() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-filltext
    void FillText(dom::DOMStringView text, double x, double y, Maybe<double> maxWidth = null) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-stroketext
    void StrokeText(dom::DOMStringView text, double x, double y, Maybe<double> maxWidth = null) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-measuretext
    KRYS_NODISCARD Ref<TextMetrics> MeasureText(dom::DOMStringView text) noexcept;

  private:
    KRYS_NODISCARD const FontProxy *GetFontProxy() noexcept final;

    KRYS_NODISCARD Maybe<style::Filter>
      SetFilterStringWithoutUpdatingStyle(dom::DOMStringView) noexcept override;
    KRYS_NODISCARD RefPtr<boo::Filter> CreateFilter(const FloatRect &bounds) const noexcept override;
    KRYS_NODISCARD IntOutsets CalculateFilterOutsets(const FloatRect &bounds) const noexcept override;
    KRYS_NODISCARD StringAtom InheritedLanguage() const noexcept final;

    void SetFontWithoutUpdatingStyle(dom::DOMStringView) noexcept;

    void DrawFocusIfNeededInternal(const Path &, dom::Element &) noexcept;

    void DrawTextInternal(dom::DOMStringView text, double x, double y, bool fill,
                          Maybe<double> maxWidth = null) noexcept;

    KRYS_NODISCARD TextDirection ToTextDirection(Direction,
                                                 const RenderStyle **computedStyle = nullptr) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(krys::boo::html::CanvasRenderingContext2D, Is2d())
