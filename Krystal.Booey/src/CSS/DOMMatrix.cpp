#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <limits>

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<DOMMatrix>>
    DOMMatrix::Create(Maybe<Variant<CSSOMString, List<double>>> &&init) noexcept
  {
    if (!init)
    {
      return AdoptRef(*new DOMMatrix);
    }

    return SwitchOn(
      *init,
      [](const CSSOMString &init) -> dom::ExceptionOr<Ref<DOMMatrix>>
      {
        auto parseResult = ParseStringIntoAbstractMatrix(init);
        if (parseResult.HasException())
        {
          return parseResult.ReleaseException();
        }
        return AdoptRef(
          *new DOMMatrix(parseResult.Value().Matrix, parseResult.Value().Is2D ? Is2D::Yes : Is2D::No));
      },
      [](const List<double> &init) -> dom::ExceptionOr<Ref<DOMMatrix>>
      { return FromArrayHelper<DOMMatrix, double>(Span<const double>(init)); });
  }

  DOMMatrix::DOMMatrix(const TransformationMatrix &matrix, Is2D is2D) noexcept
      : DOMMatrixReadOnly(matrix, is2D)
  {
  }

  dom::ExceptionOr<Ref<DOMMatrix>> DOMMatrix::FromMatrix(DOMMatrixInit &&init) noexcept
  {
    return FromMatrixHelper<DOMMatrix>(krys::move(init));
  }

  dom::ExceptionOr<Ref<DOMMatrix>> DOMMatrix::FromFloat32Array(Span<const float> array32) noexcept
  {
    return FromArrayHelper<DOMMatrix>(array32);
  }

  dom::ExceptionOr<Ref<DOMMatrix>> DOMMatrix::FromFloat64Array(Span<const double> array64) noexcept
  {
    return FromArrayHelper<DOMMatrix>(array64);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-multiplyself
  dom::ExceptionOr<Ref<DOMMatrix>> DOMMatrix::MultiplySelf(DOMMatrixInit &&other) noexcept
  {
    auto fromMatrixResult = DOMMatrix::FromMatrix(krys::move(other));
    if (fromMatrixResult.HasException())
    {
      return fromMatrixResult.ReleaseException();
    }
    auto otherObject = fromMatrixResult.ReleaseValue();
    _matrix.Multiply(otherObject->_matrix);
    if (!otherObject->GetIs2D())
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-premultiplyself
  dom::ExceptionOr<Ref<DOMMatrix>> DOMMatrix::PreMultiplySelf(DOMMatrixInit &&other) noexcept
  {
    auto fromMatrixResult = DOMMatrix::FromMatrix(krys::move(other));
    if (fromMatrixResult.HasException())
    {
      return fromMatrixResult.ReleaseException();
    }
    auto otherObject = fromMatrixResult.ReleaseValue();
    _matrix = otherObject->_matrix * _matrix;
    if (!otherObject->GetIs2D())
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-translateself
  Ref<DOMMatrix> DOMMatrix::TranslateSelf(double tx, double ty, double tz) noexcept
  {
    _matrix.Translate3d(tx, ty, tz);
    if (tz)
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-scaleself
  Ref<DOMMatrix> DOMMatrix::ScaleSelf(double scaleX, Maybe<double> scaleY, double scaleZ, double originX,
                                      double originY, double originZ) noexcept
  {
    if (!scaleY)
    {
      scaleY = scaleX;
    }
    _matrix.Translate3d(originX, originY, originZ);
    // Post-multiply a non-uniform scale transformation on the current matrix.
    // The 3D scale matrix is described in CSS Transforms with sx = scaleX, sy = scaleY and sz = scaleZ.
    _matrix.Scale3d(scaleX, scaleY.value(), scaleZ);
    _matrix.Translate3d(-originX, -originY, -originZ);
    if (scaleZ != 1 || originZ)
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-scale3dself
  Ref<DOMMatrix> DOMMatrix::Scale3dSelf(double scale, double originX, double originY, double originZ) noexcept
  {
    _matrix.Translate3d(originX, originY, originZ);
    // Post-multiply a uniform 3D scale transformation (m11 = m22 = m33 = scale) on the current matrix.
    // The 3D scale matrix is described in CSS Transforms with sx = sy = sz = scale. [CSS3-TRANSFORMS]
    _matrix.Scale3d(scale, scale, scale);
    _matrix.Translate3d(-originX, -originY, -originZ);
    if (scale != 1 || originZ)
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-rotateself
  Ref<DOMMatrix> DOMMatrix::RotateSelf(double rotX, Maybe<double> rotY, Maybe<double> rotZ) noexcept
  {
    if (!rotY && !rotZ)
    {
      rotZ = rotX;
      rotX = 0;
      rotY = 0;
    }
    _matrix.Rotate3d(rotX, rotY.value_or(0), rotZ.value_or(0));
    if (rotX || rotY.value_or(0))
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-rotatefromvectorself
  Ref<DOMMatrix> DOMMatrix::RotateFromVectorSelf(double x, double y) noexcept
  {
    _matrix.RotateFromVector(x, y);
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-rotateaxisangleself
  Ref<DOMMatrix> DOMMatrix::RotateAxisAngleSelf(double x, double y, double z, double angle) noexcept
  {
    _matrix.Rotate3d(x, y, z, angle);
    if (x || y)
    {
      _is2D = false;
    }
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-skewxself
  Ref<DOMMatrix> DOMMatrix::SkewXSelf(double sx) noexcept
  {
    _matrix.SkewX(sx);
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-skewyself
  Ref<DOMMatrix> DOMMatrix::SkewYSelf(double sy) noexcept
  {
    _matrix.SkewY(sy);
    return ShareRef(*this);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrix-invertself
  Ref<DOMMatrix> DOMMatrix::InvertSelf() noexcept
  {
    if (auto inverse = _matrix.Inverse())
    {
      _matrix = *inverse;
    }
    else
    {
      constexpr auto nan = std::numeric_limits<double>::quiet_NaN();
      _matrix.SetMatrix(nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan);
      _is2D = false;
    }
    return ShareRef(*this);
  }
}
