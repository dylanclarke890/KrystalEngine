#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include <cmath>

namespace krys::boo::html
{
  // https://html.spec.whatwg.org/#dom-path2d
  Ref<Path2D> Path2D::Create(dom::DOMStringView pathData) noexcept
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

  // https://html.spec.whatwg.org/#dom-path2d-addpath
  dom::ExceptionOr<void> Path2D::AddPath(const Path2D &path, css::DOMMatrix2DInit &&matrixInit) noexcept
  {
    // 2. Let matrix be the result of creating a DOMMatrix from the 2D dictionary transform.
    auto checkValid = css::DOMMatrixReadOnly::ValidateAndFixup(matrixInit);
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
    for (auto value : {*matrixInit.M11, *matrixInit.M12, *matrixInit.M21, *matrixInit.M22, *matrixInit.M41,
                       *matrixInit.M42})
    {
      if (!std::isfinite(value))
      {
        return {};
      }
    }
    AffineTransform matrix {*matrixInit.M11, *matrixInit.M12, *matrixInit.M21,
                            *matrixInit.M22, *matrixInit.M41, *matrixInit.M42};

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
