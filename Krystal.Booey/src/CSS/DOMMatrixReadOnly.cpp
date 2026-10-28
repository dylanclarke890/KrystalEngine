#include "Krystal.Booey/CSS/DOMMatrixReadOnly.hpp"
#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Transform.hpp"
#include "Krystal.Booey/DOM/DOMPoint.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <cmath>

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
    DOMMatrixReadOnly::Create(Maybe<Variant<CSSOMString, List<double>>> &&init) noexcept
  {
    if (!init)
    {
      return AdoptRef(*new DOMMatrixReadOnly);
    }

    return SwitchOn(
      *init,
      [](const CSSOMString &init) -> dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
      {
        auto parseResult = ParseStringIntoAbstractMatrix(init);
        if (parseResult.HasException())
        {
          return parseResult.ReleaseException();
        }
        return AdoptRef(*new DOMMatrixReadOnly(parseResult.Value().Matrix,
                                               parseResult.Value().Is2D ? Is2D::Yes : Is2D::No));
      },
      [](const List<double> &init) -> dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
      { return FromArrayHelper<DOMMatrixReadOnly, double>(Span<const double>(init)); });
  }

  DOMMatrixReadOnly::DOMMatrixReadOnly(const TransformationMatrix &matrix, Is2D is2D) noexcept
      : _matrix(matrix), _is2D(is2D == Is2D::Yes)
  {
    krys_debug_assert(!_is2D || _matrix.IsAffine());
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::CloneAsDOMMatrix() const noexcept
  {
    return DOMMatrix::Create(_matrix, _is2D ? Is2D::Yes : Is2D::No);
  }

  // https://tc39.github.io/ecma262/#sec-samevaluezero
  KRYS_NODISCARD static bool SameValueZero(double a, double b) noexcept
  {
    if (std::isnan(a) && std::isnan(b))
    {
      return true;
    }
    return a == b;
  }

  dom::ExceptionOr<void> DOMMatrixReadOnly::ValidateAndFixup(DOMMatrix2DInit &init) noexcept
  {
    using dom::Exception;
    using dom::ExceptionCode;

    if (init.A && init.M11 && !SameValueZero(init.A.value(), init.M11.value()))
    {
      return Exception {ExceptionCode::TypeError, u8"init.a and init.m11 do not match"};
    }
    if (init.B && init.M12 && !SameValueZero(init.B.value(), init.M12.value()))
    {
      return Exception {ExceptionCode::TypeError, u8"init.b and init.m12 do not match"};
    }
    if (init.C && init.M21 && !SameValueZero(init.C.value(), init.M21.value()))
    {
      return Exception {ExceptionCode::TypeError, u8"init.c and init.m21 do not match"};
    }
    if (init.D && init.M22 && !SameValueZero(init.D.value(), init.M22.value()))
    {
      return Exception {ExceptionCode::TypeError, u8"init.d and init.m22 do not match"};
    }
    if (init.E && init.M41 && !SameValueZero(init.E.value(), init.M41.value()))
    {
      return Exception {ExceptionCode::TypeError, u8"init.e and init.m41 do not match"};
    }
    if (init.F && init.M42 && !SameValueZero(init.F.value(), init.M42.value()))
    {
      return Exception {ExceptionCode::TypeError, u8"init.f and init.m42 do not match"};
    }

    if (!init.M11)
    {
      init.M11 = init.A.value_or(1);
    }
    if (!init.M12)
    {
      init.M12 = init.B.value_or(0);
    }
    if (!init.M21)
    {
      init.M21 = init.C.value_or(0);
    }
    if (!init.M22)
    {
      init.M22 = init.D.value_or(1);
    }
    if (!init.M41)
    {
      init.M41 = init.E.value_or(0);
    }
    if (!init.M42)
    {
      init.M42 = init.F.value_or(0);
    }

    return {};
  }

  dom::ExceptionOr<void> DOMMatrixReadOnly::ValidateAndFixup(DOMMatrixInit &init) noexcept
  {
    using dom::Exception;
    using dom::ExceptionCode;

    auto validate2D = ValidateAndFixup(static_cast<DOMMatrix2DInit &>(init));
    if (validate2D.HasException())
    {
      return validate2D.ReleaseException();
    }

    if (init.Is2D && init.Is2D.value())
    {
      if (init.M13)
      {
        return Exception {ExceptionCode::TypeError, u8"m13 should be 0 for a 2D matrix"};
      }
      if (init.M14)
      {
        return Exception {ExceptionCode::TypeError, u8"m14 should be 0 for a 2D matrix"};
      }
      if (init.M23)
      {
        return Exception {ExceptionCode::TypeError, u8"m23 should be 0 for a 2D matrix"};
      }
      if (init.M24)
      {
        return Exception {ExceptionCode::TypeError, u8"m24 should be 0 for a 2D matrix"};
      }
      if (init.M31)
      {
        return Exception {ExceptionCode::TypeError, u8"m31 should be 0 for a 2D matrix"};
      }
      if (init.M32)
      {
        return Exception {ExceptionCode::TypeError, u8"m32 should be 0 for a 2D matrix"};
      }
      if (init.M34)
      {
        return Exception {ExceptionCode::TypeError, u8"m34 should be 0 for a 2D matrix"};
      }
      if (init.M43)
      {
        return Exception {ExceptionCode::TypeError, u8"m43 should be 0 for a 2D matrix"};
      }
      if (init.M33 != 1)
      {
        return Exception {ExceptionCode::TypeError, u8"m33 should be 1 for a 2D matrix"};
      }
      if (init.M44 != 1)
      {
        return Exception {ExceptionCode::TypeError, u8"m44 should be 1 for a 2D matrix"};
      }
    }

    if (!init.Is2D)
    {
      if (init.M13 || init.M14 || init.M23 || init.M24 || init.M31 || init.M32 || init.M34 || init.M43
          || init.M33 != 1 || init.M44 != 1)
      {
        init.Is2D = false;
      }
      else
      {
        init.Is2D = true;
      }
    }
    return {};
  }

  dom::ExceptionOr<Ref<DOMMatrixReadOnly>> DOMMatrixReadOnly::FromMatrix(DOMMatrixInit &&init) noexcept
  {
    return FromMatrixHelper<DOMMatrixReadOnly>(krys::move(init));
  }

  dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
    DOMMatrixReadOnly::FromFloat32Array(Span<const float> array32) noexcept
  {
    return FromArrayHelper<DOMMatrixReadOnly>(array32);
  }

  dom::ExceptionOr<Ref<DOMMatrixReadOnly>>
    DOMMatrixReadOnly::FromFloat64Array(Span<const double> array64) noexcept
  {
    return FromArrayHelper<DOMMatrixReadOnly>(array64);
  }

  bool DOMMatrixReadOnly::IsIdentity() const noexcept
  {
    return _matrix.IsIdentity();
  }

  dom::ExceptionOr<DOMMatrixReadOnly::AbstractMatrix>
    DOMMatrixReadOnly::ParseStringIntoAbstractMatrix(const CSSOMString &string) noexcept
  {
    if (string.empty())
    {
      return AbstractMatrix {};
    }

    auto transform =
      PropertyParserHelpers::ParseTransformRaw(string, ParserContext(ParserMode::HTMLStandard));
    if (!transform)
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    // Check for an empty transform operations list, in which case we can use the default identity matrix.
    if (transform->IsNone())
    {
      return AbstractMatrix {};
    }

    auto [isWidthDependent, isHeightDependent] = transform->ComputeSizeDependencies();
    if (isWidthDependent || isHeightDependent)
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }

    AbstractMatrix matrix;
    for (const auto &function : *transform)
    {
      function->Apply(matrix.Matrix, {0, 0});
      if (function->Is3DOperation())
      {
        matrix.Is2D = false;
      }
    }
    return matrix;
  }

  dom::ExceptionOr<void> DOMMatrixReadOnly::SetMatrixValue(const CSSOMString &string) noexcept
  {
    auto parseResult = ParseStringIntoAbstractMatrix(string);
    if (parseResult.HasException())
    {
      return parseResult.ReleaseException();
    }

    _is2D = parseResult.Value().Is2D;
    _matrix = parseResult.Value().Matrix;
    return {};
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Translate(double tx, double ty, double tz) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->TranslateSelf(tx, ty, tz);
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrixreadonly-flipx
  Ref<DOMMatrix> DOMMatrixReadOnly::FlipX() const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    matrix->_matrix.FlipX();
    return matrix;
  }

  // https://drafts.fxtf.org/geometry/#dom-dommatrixreadonly-flipy
  Ref<DOMMatrix> DOMMatrixReadOnly::FlipY() const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    matrix->_matrix.FlipY();
    return matrix;
  }

  dom::ExceptionOr<Ref<DOMMatrix>> DOMMatrixReadOnly::Multiply(DOMMatrixInit &&other) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->MultiplySelf(krys::move(other));
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Scale(double scaleX, Maybe<double> scaleY, double scaleZ, double originX,
                                          double originY, double originZ) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->ScaleSelf(scaleX, scaleY, scaleZ, originX, originY, originZ);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Scale3d(double scale, double originX, double originY,
                                            double originZ) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->Scale3dSelf(scale, originX, originY, originZ);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::ScaleNonUniform(double scaleX, double scaleY) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->ScaleSelf(scaleX, scaleY, 1, 0, 0, 0);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Rotate(double rotX, Maybe<double> rotY, Maybe<double> rotZ) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->RotateSelf(rotX, rotY, rotZ);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::RotateFromVector(double x, double y) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->RotateFromVectorSelf(x, y);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::RotateAxisAngle(double x, double y, double z, double angle) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->RotateAxisAngleSelf(x, y, z, angle);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::SkewX(double sx) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->SkewXSelf(sx);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::SkewY(double sy) const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->SkewYSelf(sy);
  }

  Ref<DOMMatrix> DOMMatrixReadOnly::Inverse() const noexcept
  {
    auto matrix = CloneAsDOMMatrix();
    return matrix->InvertSelf();
  }

  Ref<dom::DOMPoint> DOMMatrixReadOnly::TransformPoint(dom::DOMPointInit &&pointInit) const noexcept
  {
    _matrix.Map4ComponentPoint(pointInit.X, pointInit.Y, pointInit.Z, pointInit.W);
    return dom::DOMPoint::Create(pointInit.X, pointInit.Y, pointInit.Z, pointInit.W);
  }

  Array<float, 16> DOMMatrixReadOnly::ToFloat32Array() const noexcept
  {
    const auto array64 = ToFloat64Array();
    Array<float, 16> array32;
    for (size_t i = 0; i < array64.size(); ++i)
    {
      array32[i] = static_cast<float>(array64[i]);
    }
    return array32;
  }

  Array<double, 16> DOMMatrixReadOnly::ToFloat64Array() const noexcept
  {
    return {_matrix.M11(), _matrix.M12(), _matrix.M13(), _matrix.M14(), _matrix.M21(), _matrix.M22(),
            _matrix.M23(), _matrix.M24(), _matrix.M31(), _matrix.M32(), _matrix.M33(), _matrix.M34(),
            _matrix.M41(), _matrix.M42(), _matrix.M43(), _matrix.M44()};
  }

  dom::ExceptionOr<CSSOMString> DOMMatrixReadOnly::ToString() const noexcept
  {
    using namespace std::string_view_literals;

    if (!_matrix.ContainsOnlyFiniteValues())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Matrix contains non-finite values"};
    }

    StringBuilder builder;
    if (GetIs2D())
    {
      builder.Append(u8"matrix("sv, _matrix.A(), u8", "sv, _matrix.B(), u8", "sv, _matrix.C(), u8", "sv,
                     _matrix.D(), u8", "sv, _matrix.E(), u8", "sv, _matrix.F(), u8')');
      return krys::move(builder).ToString();
    }

    builder.Append(u8"matrix3d("sv, _matrix.M11(), u8", "sv, _matrix.M12(), u8", "sv, _matrix.M13(), u8", "sv,
                   _matrix.M14(), u8", "sv, _matrix.M21(), u8", "sv, _matrix.M22(), u8", "sv, _matrix.M23(),
                   u8", "sv, _matrix.M24(), u8", "sv, _matrix.M31(), u8", "sv, _matrix.M32(), u8", "sv,
                   _matrix.M33(), u8", "sv, _matrix.M34(), u8", "sv, _matrix.M41(), u8", "sv, _matrix.M42(),
                   u8", "sv, _matrix.M43(), u8", "sv, _matrix.M44(), u8')');
    return krys::move(builder).ToString();
  }
}
