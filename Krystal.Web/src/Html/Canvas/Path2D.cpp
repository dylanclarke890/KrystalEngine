/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Html/Canvas/Path2D.hpp"
#include "Krystal.Core/Geometry/AffineTransform.hpp"
#include "Krystal.Web/Css/Geometry/DOMMatrixReadOnly.hpp"
#include "Krystal.Web/Svg/Internals/SVGPathUtilities.hpp"
#include <cmath>

namespace krys::web::html
{
  // https://html.spec.whatwg.org/multipage/canvas.html#dom-path2d
  Ref<Path2D> Path2D::Create(DOMStringView pathData) noexcept
  {
    // 4. Let svgPath be the result of parsing and interpreting path according to SVG 2's rules for path data.
    auto svgPath = svg::BuildPathFromString(pathData);

    // 7. Return output.
    if (svgPath.IsEmpty())
    {
      return Create();
    }

    // 5. Let (x, y) be the last point in svgPath.
    auto lastPoint = svgPath.CurrentPoint();

    // 1. Let output be a new Path2D object.
    // 6. Add all the subpaths, if any, from svgPath to output.
    auto output = Create(svgPath);

    // 7. Create a new subpath in output with (x, y) as the only point in the subpath.
    output->_path.MoveTo(lastPoint);
    return output;
  }

  // https://html.spec.whatwg.org/multipage/canvas.html#dom-path2d-addpath
  ExceptionOr<void> Path2D::AddPath(const Path2D &path, DOMMatrix2DInit transform) noexcept
  {
    // 2. Let matrix be the result of creating a DOMMatrix from the 2D dictionary transform.
    //    https://drafts.csswg.org/geometry-1/#create-a-dommatrixreadonly-from-the-2d-dictionary
    auto checkValid = DOMMatrixReadOnly::ValidateAndFixup(transform);
    if (checkValid.HasException())
    {
      return checkValid.ReleaseException();
    }

    // 1. If the Path2D object path has no subpaths, then return.
    if (path.GetPath().IsEmpty())
    {
      return {};
    }

    // 3. If one or more of matrix's m11 element, m12 element, m21 element, m22 element, m41 element, or m42
    //    element are infinite or NaN, then return.
    for (auto value :
         {*transform.M11, *transform.M12, *transform.M21, *transform.M22, *transform.M41, *transform.M42})
    {
      if (!std::isfinite(value))
      {
        return {};
      }
    }
    AffineTransform matrix {*transform.M11, *transform.M12, *transform.M21,
                            *transform.M22, *transform.M41, *transform.M42};

    // 4. Create a copy of all the subpaths in path. Let c be this copy.
    // 5. Transform all the coordinates and lines in c by the transform matrix matrix.
    // 6. Let (x, y) be the last point in the last subpath of c.
    // 7. Add all the subpaths in c to a.
    auto lastPoint = matrix.MapPoint(path.GetPath().CurrentPoint());
    _path.AddPath(path.GetPath(), matrix);

    // 8. Create a new subpath in a with (x, y) as the only point in the subpath.
    _path.MoveTo(lastPoint);
    return {};
  }
}
