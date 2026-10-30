#include "Krystal.Booey/HTML/Canvas/Path2D.hpp"
#include "Krystal.Booey/CSS/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"

namespace krys::boo::html
{
  Ref<Path2D> Path2D::Create(dom::DOMStringView pathData) noexcept
  {
    return Create(svg::BuildPathFromString(pathData));
  }

  dom::ExceptionOr<void> Path2D::AddPath(const Path2D &path, css::DOMMatrix2DInit &&matrixInit) noexcept
  {
    auto checkValid = css::DOMMatrixReadOnly::ValidateAndFixup(matrixInit);
    if (checkValid.HasException())
    {
      return checkValid.ReleaseException();
    }

    _path.AddPath(path.GetPath(), {*matrixInit.M11, *matrixInit.M12, *matrixInit.M21, *matrixInit.M22,
                                   *matrixInit.M41, *matrixInit.M42});
    return {};
  }
}
