#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD double Det(const Array<double, 6> &transform) noexcept
    {
      return transform[0] * transform[3] - transform[1] * transform[2];
    }
  }

  AffineTransform AffineTransform::MakeRotation(double angleInDegrees, FloatPoint center) noexcept
  {
    if (center.IsZero())
    {
      return MakeRotation(angleInDegrees);
    }

    auto centerSize = ToFloatSize(center);
    auto matrix = MakeTranslation(centerSize);
    matrix.Rotate(angleInDegrees);
    matrix.Translate(-centerSize);
    return matrix;
  }

  AffineTransform AffineTransform::MakeRotation(double angleInDegrees) noexcept
  {
    auto angleInRadians = deg2rad(angleInDegrees);
    double cosAngle = std::cos(angleInRadians);
    double sinAngle = std::sin(angleInRadians);
    return AffineTransform {cosAngle, sinAngle, -sinAngle, cosAngle, 0, 0};
  }

  void AffineTransform::MakeIdentity() noexcept
  {
    SetMatrix(1, 0, 0, 1, 0, 0);
  }

  void AffineTransform::SetMatrix(double a, double b, double c, double d, double e, double f) noexcept
  {
    _transform[0] = a;
    _transform[1] = b;
    _transform[2] = c;
    _transform[3] = d;
    _transform[4] = e;
    _transform[5] = f;
  }

  bool AffineTransform::IsIdentity() const noexcept
  {
    return (_transform[0] == 1 && _transform[1] == 0 && _transform[2] == 0 && _transform[3] == 1
            && _transform[4] == 0 && _transform[5] == 0);
  }

  double AffineTransform::XScale() const noexcept
  {
    return std::hypot(_transform[0], _transform[1]);
  }

  double AffineTransform::YScale() const noexcept
  {
    return std::hypot(_transform[2], _transform[3]);
  }

  bool AffineTransform::IsInvertible() const noexcept
  {
    double determinant = Det(_transform);

    return std::isnormal(determinant);
  }

  Maybe<AffineTransform> AffineTransform::Inverse() const noexcept
  {
    AffineTransform result;
    if (IsIdentityOrTranslation())
    {
      result._transform[4] = -_transform[4];
      result._transform[5] = -_transform[5];
      return result;
    }

    double determinant = Det(_transform);
    if (!std::isnormal(determinant))
    {
      return {};
    }

    result._transform[0] = _transform[3] / determinant;
    result._transform[1] = -_transform[1] / determinant;
    result._transform[2] = -_transform[2] / determinant;
    result._transform[3] = _transform[0] / determinant;
    result._transform[4] = (_transform[2] * _transform[5] - _transform[3] * _transform[4]) / determinant;
    result._transform[5] = (_transform[1] * _transform[4] - _transform[0] * _transform[5]) / determinant;

    return result;
  }

  // Multiplies this AffineTransform by the provided AffineTransform - i.e.
  // this = this * other;
  AffineTransform &AffineTransform::Multiply(const AffineTransform &other) noexcept
  {
    AffineTransform trans;

    trans._transform[0] = other._transform[0] * _transform[0] + other._transform[1] * _transform[2];
    trans._transform[1] = other._transform[0] * _transform[1] + other._transform[1] * _transform[3];
    trans._transform[2] = other._transform[2] * _transform[0] + other._transform[3] * _transform[2];
    trans._transform[3] = other._transform[2] * _transform[1] + other._transform[3] * _transform[3];
    trans._transform[4] =
      other._transform[4] * _transform[0] + other._transform[5] * _transform[2] + _transform[4];
    trans._transform[5] =
      other._transform[4] * _transform[1] + other._transform[5] * _transform[3] + _transform[5];

    *this = trans;
    return *this;
  }

  AffineTransform &AffineTransform::Rotate(double a) noexcept
  {
    // angle is in degree. Switch to radian
    return RotateRadians(deg2rad(a));
  }

  AffineTransform &AffineTransform::RotateRadians(double a) noexcept
  {
    double cosAngle = std::cos(a);
    double sinAngle = std::sin(a);
    AffineTransform rot(cosAngle, sinAngle, -sinAngle, cosAngle, 0, 0);

    Multiply(rot);
    return *this;
  }

  AffineTransform &AffineTransform::Scale(double s) noexcept
  {
    return Scale(s, s);
  }

  AffineTransform &AffineTransform::Scale(double sx, double sy) noexcept
  {
    _transform[0] *= sx;
    _transform[1] *= sx;
    _transform[2] *= sy;
    _transform[3] *= sy;
    return *this;
  }

  AffineTransform &AffineTransform::ScaleNonUniform(double sx, double sy) noexcept
  {
    return Scale(sx, sy);
  }

  AffineTransform &AffineTransform::Scale(const FloatSize &s) noexcept
  {
    return Scale(s.Width(), s.Height());
  }

  // *this = *this * translation
  AffineTransform &AffineTransform::Translate(double tx, double ty) noexcept
  {
    if (IsIdentityOrTranslation())
    {
      _transform[4] += tx;
      _transform[5] += ty;
      return *this;
    }

    _transform[4] += tx * _transform[0] + ty * _transform[2];
    _transform[5] += tx * _transform[1] + ty * _transform[3];
    return *this;
  }

  AffineTransform &AffineTransform::Translate(const FloatPoint &t) noexcept
  {
    return Translate(t.X(), t.Y());
  }

  AffineTransform &AffineTransform::Translate(const FloatSize &t) noexcept
  {
    return Translate(t.Width(), t.Height());
  }

  AffineTransform &AffineTransform::RotateFromVector(double x, double y) noexcept
  {
    return RotateRadians(std::atan2(y, x));
  }

  AffineTransform &AffineTransform::FlipX() noexcept
  {
    return Scale(-1, 1);
  }

  AffineTransform &AffineTransform::FlipY() noexcept
  {
    return Scale(1, -1);
  }

  AffineTransform &AffineTransform::Shear(double sx, double sy) noexcept
  {
    double a = _transform[0];
    double b = _transform[1];

    _transform[0] += sy * _transform[2];
    _transform[1] += sy * _transform[3];
    _transform[2] += sx * a;
    _transform[3] += sx * b;

    return *this;
  }

  AffineTransform &AffineTransform::Skew(double angleX, double angleY) noexcept
  {
    return Shear(std::tan(deg2rad(angleX)), std::tan(deg2rad(angleY)));
  }

  AffineTransform &AffineTransform::SkewX(double angle) noexcept
  {
    return Shear(std::tan(deg2rad(angle)), 0);
  }

  AffineTransform &AffineTransform::SkewY(double angle) noexcept
  {
    return Shear(0, std::tan(deg2rad(angle)));
  }

  AffineTransform MakeMapBetweenRects(const FloatRect &source, const FloatRect &dest) noexcept
  {
    AffineTransform transform;
    transform.Translate(dest.X() - source.X(), dest.Y() - source.Y());
    transform.Scale(dest.Size() / source.Size());
    return transform;
  }

  void AffineTransform::Map(double x, double y, double &x2, double &y2) const noexcept
  {
    x2 = (_transform[0] * x + _transform[2] * y + _transform[4]);
    y2 = (_transform[1] * x + _transform[3] * y + _transform[5]);
  }

  IntPoint AffineTransform::MapPoint(const IntPoint &point) const noexcept
  {
    double x2, y2;
    Map(point.X(), point.Y(), x2, y2);

    // Round the point.
    return IntPoint(static_cast<int32>(std::lround(x2)), static_cast<int32>(std::lround(y2)));
  }

  FloatPoint AffineTransform::MapPoint(const FloatPoint &point) const noexcept
  {
    double x2, y2;
    Map(point.X(), point.Y(), x2, y2);

    return FloatPoint(NarrowPrecisionToFloat(x2), NarrowPrecisionToFloat(y2));
  }

  IntSize AffineTransform::MapSize(const IntSize &size) const noexcept
  {
    double width2 = size.Width() * XScale();
    double height2 = size.Height() * YScale();

    return IntSize(static_cast<int32>(std::lround(width2)), static_cast<int32>(std::lround(height2)));
  }

  FloatSize AffineTransform::MapSize(const FloatSize &size) const noexcept
  {
    double width2 = size.Width() * XScale();
    double height2 = size.Height() * YScale();

    return FloatSize(NarrowPrecisionToFloat(width2), NarrowPrecisionToFloat(height2));
  }

  IntRect AffineTransform::MapRect(const IntRect &rect) const noexcept
  {
    return EnclosingIntRect(MapRect(FloatRect(rect)));
  }

  FloatRect AffineTransform::MapRect(const FloatRect &rect) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      FloatRect mappedRect(rect);
      mappedRect.Move(NarrowPrecisionToFloat(_transform[4]), NarrowPrecisionToFloat(_transform[5]));
      return mappedRect;
    }

    // This is equivalent to mapPoint() on each corner, then finding the bounds of the resulting quad.
    // Map point is:
    // x2 = a * x + c * y + tx;
    // y2 = b * x + d * y + ty;
    // and since x and y are the same for points sharing a side, we can save some computation.

    auto a = A();
    auto b = B();
    auto c = C();
    auto d = D();

    auto tx = E();
    auto ty = F();

    double left = rect.X();
    double top = rect.Y();

    double right = rect.MaxX();
    double bottom = rect.MaxY();

    double aLeft = a * left;
    double aRight = a * right;

    double bLeft = b * left;
    double bRight = b * right;

    double cTop = c * top;
    double cBottom = c * bottom;

    double dTop = d * top;
    double dBottom = d * bottom;

    auto x1 = NarrowPrecisionToFloat(aLeft + cTop + tx);
    auto y1 = NarrowPrecisionToFloat(bLeft + dTop + ty);
    auto x2 = NarrowPrecisionToFloat(aRight + cTop + tx);
    auto y2 = NarrowPrecisionToFloat(bRight + dTop + ty);
    auto x3 = NarrowPrecisionToFloat(aRight + cBottom + tx);
    auto y3 = NarrowPrecisionToFloat(bRight + dBottom + ty);
    auto x4 = NarrowPrecisionToFloat(aLeft + cBottom + tx);
    auto y4 = NarrowPrecisionToFloat(bLeft + dBottom + ty);

    auto minX = Min4(x1, x2, x3, x4);
    auto minY = Min4(y1, y2, y3, y4);
    auto maxX = Max4(x1, x2, x3, x4);
    auto maxY = Max4(y1, y2, y3, y4);

    return FloatRect {minX, minY, maxX - minX, maxY - minY};
  }

  FloatQuad AffineTransform::MapQuad(const FloatQuad &q) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      FloatQuad mappedQuad(q);
      mappedQuad.Move(NarrowPrecisionToFloat(_transform[4]), NarrowPrecisionToFloat(_transform[5]));
      return mappedQuad;
    }

    FloatQuad result;
    result.SetP1(MapPoint(q.P1()));
    result.SetP2(MapPoint(q.P2()));
    result.SetP3(MapPoint(q.P3()));
    result.SetP4(MapPoint(q.P4()));
    return result;
  }

  Region AffineTransform::MapRegion(const Region &region) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      Region mappedRegion(region);
      mappedRegion.Translate(RoundedIntSize(
        FloatSize(NarrowPrecisionToFloat(_transform[4]), NarrowPrecisionToFloat(_transform[5]))));
      return mappedRegion;
    }

    Region mappedRegion;
    for (auto &rect : region.Rects())
    {
      mappedRegion.Unite(MapRect(rect));
    }

    return mappedRegion;
  }

  void AffineTransform::Blend(const AffineTransform &from, double progress,
                              CompositeOperation compositeOperation) noexcept
  {
    DecomposedType srA, srB;

    from.Decompose(srA);
    this->Decompose(srB);

    // If x-axis of one is flipped, and y-axis of the other, convert to an unflipped rotation.
    if ((srA.ScaleX < 0 && srB.ScaleY < 0) || (srA.ScaleY < 0 && srB.ScaleX < 0))
    {
      srA.ScaleX = -srA.ScaleX;
      srA.ScaleY = -srA.ScaleY;
      srA.Angle += srA.Angle < 0 ? std::numbers::pi : -std::numbers::pi;
    }

    // Don't rotate the long way around.
    srA.Angle = std::fmod(srA.Angle, 2 * std::numbers::pi);
    srB.Angle = std::fmod(srB.Angle, 2 * std::numbers::pi);

    if (std::abs(srA.Angle - srB.Angle) > std::numbers::pi)
    {
      if (srA.Angle > srB.Angle)
      {
        srA.Angle -= std::numbers::pi * 2;
      }
      else
      {
        srB.Angle -= std::numbers::pi * 2;
      }
    }

    srA.ScaleX += progress * (srB.ScaleX - srA.ScaleX);
    srA.ScaleY += progress * (srB.ScaleY - srA.ScaleY);
    srA.Angle += progress * (srB.Angle - srA.Angle);
    srA.RemainderA += progress * (srB.RemainderA - srA.RemainderA);
    srA.RemainderB += progress * (srB.RemainderB - srA.RemainderB);
    srA.RemainderC += progress * (srB.RemainderC - srA.RemainderC);
    srA.RemainderD += progress * (srB.RemainderD - srA.RemainderD);
    srA.TranslateX += progress * (srB.TranslateX - srA.TranslateX);
    srA.TranslateY += progress * (srB.TranslateY - srA.TranslateY);

    if (compositeOperation != CompositeOperation::Replace)
    {
      srA.ScaleX += srA.ScaleX;
      srA.ScaleY += srA.ScaleY;
      srA.Angle += srA.Angle;
      srA.RemainderA += srA.RemainderA;
      srA.RemainderB += srA.RemainderB;
      srA.RemainderC += srA.RemainderC;
      srA.RemainderD += srA.RemainderD;
      srA.TranslateX += srA.TranslateX;
      srA.TranslateY += srA.TranslateY;
    }

    this->Recompose(srA);
  }

  TransformationMatrix AffineTransform::ToTransformationMatrix() const noexcept
  {
    return TransformationMatrix(_transform[0], _transform[1], _transform[2], _transform[3], _transform[4],
                                _transform[5]);
  }

  bool AffineTransform::Decompose(DecomposedType &decomp) const noexcept
  {
    AffineTransform m(*this);

    // Compute scaling factors
    double sx = XScale();
    double sy = YScale();

    // Compute cross product of transformed unit vectors. If negative,
    // one axis was flipped.
    if (m.A() * m.D() - m.C() * m.B() < 0)
    {
      // Flip axis with minimum unit vector dot product
      if (m.A() < m.D())
      {
        sx = -sx;
      }
      else
      {
        sy = -sy;
      }
    }

    // Remove scale from matrix
    m.Scale(1 / sx, 1 / sy);

    // Compute rotation
    double angle = std::atan2(m.B(), m.A());

    // Remove rotation from matrix
    m.RotateRadians(-angle);

    // Return results
    decomp.ScaleX = sx;
    decomp.ScaleY = sy;
    decomp.Angle = angle;
    decomp.RemainderA = m.A();
    decomp.RemainderB = m.B();
    decomp.RemainderC = m.C();
    decomp.RemainderD = m.D();
    decomp.TranslateX = m.E();
    decomp.TranslateY = m.F();

    return true;
  }

  void AffineTransform::Recompose(const DecomposedType &decomp) noexcept
  {
    this->SetA(decomp.RemainderA);
    this->SetB(decomp.RemainderB);
    this->SetC(decomp.RemainderC);
    this->SetD(decomp.RemainderD);
    this->SetE(decomp.TranslateX);
    this->SetF(decomp.TranslateY);
    this->RotateRadians(decomp.Angle);
    this->Scale(decomp.ScaleX, decomp.ScaleY);
  }

  TextStream &operator<<(TextStream &ts, const AffineTransform &transform) noexcept
  {
    if (transform.IsIdentity())
    {
      ts << "identity";
    }
    else
    {
      ts << "{m=((" << transform.A() << "," << transform.B() << ")(" << transform.C() << "," << transform.D()
         << ")) t=(" << transform.E() << "," << transform.F() << ")}";
    }
    return ts;
  }
}
