/*
 * Copyright (C) 2006, 2007, 2009, 2010, 2011, 2012 Apple Inc. All rights reserved.
 * Copyright (C) 2012, 2013 Adobe Systems Incorporated. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Geometry/DOMPointReadOnly.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/Paths/Path.hpp"
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
