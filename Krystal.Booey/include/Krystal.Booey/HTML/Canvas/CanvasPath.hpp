#pragma once

#include "Krystal.Booey/DOM/Dicts/DOMPointInit.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  /// @brief The path-building methods a 2D context and a `Path2D` share.
  /// @see https://html.spec.whatwg.org/#canvaspath
  class CanvasPath
  {
  public:
    using RadiusVariant = Variant<double, dom::DOMPointInit>;

  protected:
    Path _path;
    bool _hasInvertibleTransform {true};

  public:
    virtual ~CanvasPath() noexcept = default;

    /// @see https://html.spec.whatwg.org/#dom-context-2d-closepath
    void ClosePath() noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-moveto
    void MoveTo(float x, float y) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-lineto
    void LineTo(float x, float y) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-quadraticcurveto
    void QuadraticCurveTo(float cpx, float cpy, float x, float y) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-beziercurveto
    void BezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-arcto
    dom::ExceptionOr<void> ArcTo(float x1, float y1, float x2, float y2, float radius) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-arc
    dom::ExceptionOr<void> Arc(float x, float y, float radius, float startAngle, float endAngle,
                               bool anticlockwise = false) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-ellipse
    dom::ExceptionOr<void> Ellipse(float x, float y, float radiusX, float radiusY, float rotation,
                                   float startAngle, float endAngle, bool anticlockwise = false) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-rect
    void Rect(float x, float y, float width, float height) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-roundrect
    dom::ExceptionOr<void> RoundRect(float x, float y, float width, float height,
                                     const RadiusVariant &radii) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-context-2d-roundrect
    dom::ExceptionOr<void> RoundRect(float x, float y, float width, float height,
                                     Span<const RadiusVariant> radii) noexcept;

  protected:
    CanvasPath() noexcept = default;
    explicit CanvasPath(const Path &path) noexcept : _path(path)
    {
    }

    KRYS_NODISCARD bool HasInvertibleTransform() const noexcept
    {
      return _hasInvertibleTransform;
    }

    void LineTo(FloatPoint) noexcept;
  };
}
