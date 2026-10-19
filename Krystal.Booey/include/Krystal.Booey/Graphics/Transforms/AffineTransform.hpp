#pragma once

#include "Krystal.Booey/Core/FloatConversion.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatQuad;
  class FloatRect;
  class IntRect;
  class Region;
  class TransformationMatrix;

  /// @brief A 2D affine transform: a 2x2 matrix [a c; b d] plus a translation (e, f).
  class AffineTransform
  {
  private:
    Array<double, 6> _transform;

  public:
    /// @brief The identity transform.
    static const AffineTransform Identity;

    constexpr AffineTransform() noexcept : _transform {{1, 0, 0, 1, 0, 0}}
    {
    }

    constexpr AffineTransform(double a, double b, double c, double d, double e, double f) noexcept
        : _transform {{a, b, c, d, e, f}}
    {
    }

    constexpr explicit AffineTransform(Span<const double, 6> transform) noexcept
        : _transform {{transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]}}
    {
    }

    void SetMatrix(double a, double b, double c, double d, double e, double f) noexcept;

    void Map(double x, double y, double &x2, double &y2) const noexcept;

    // Rounds the mapped point to the nearest integer value.
    KRYS_NODISCARD IntPoint MapPoint(const IntPoint &point) const noexcept;
    KRYS_NODISCARD FloatPoint MapPoint(const FloatPoint &point) const noexcept;

    KRYS_NODISCARD IntSize MapSize(const IntSize &size) const noexcept;
    KRYS_NODISCARD FloatSize MapSize(const FloatSize &size) const noexcept;

    // Rounds the resulting mapped rectangle out. This is helpful for bounding
    // box computations but may not be what is wanted in other contexts.
    KRYS_NODISCARD IntRect MapRect(const IntRect &rect) const noexcept;

    KRYS_NODISCARD FloatRect MapRect(const FloatRect &rect) const noexcept;
    KRYS_NODISCARD FloatQuad MapQuad(const FloatQuad &quad) const noexcept;

    KRYS_NODISCARD Region MapRegion(const Region &region) const noexcept;

    KRYS_NODISCARD bool IsIdentity() const noexcept;

    KRYS_NODISCARD constexpr double A() const noexcept
    {
      return _transform[0];
    }

    constexpr void SetA(double a) noexcept
    {
      _transform[0] = a;
    }

    KRYS_NODISCARD constexpr double B() const noexcept
    {
      return _transform[1];
    }

    constexpr void SetB(double b) noexcept
    {
      _transform[1] = b;
    }

    KRYS_NODISCARD constexpr double C() const noexcept
    {
      return _transform[2];
    }

    constexpr void SetC(double c) noexcept
    {
      _transform[2] = c;
    }

    KRYS_NODISCARD constexpr double D() const noexcept
    {
      return _transform[3];
    }

    constexpr void SetD(double d) noexcept
    {
      _transform[3] = d;
    }

    KRYS_NODISCARD constexpr double E() const noexcept
    {
      return _transform[4];
    }

    constexpr void SetE(double e) noexcept
    {
      _transform[4] = e;
    }

    KRYS_NODISCARD constexpr double F() const noexcept
    {
      return _transform[5];
    }

    constexpr void SetF(double f) noexcept
    {
      _transform[5] = f;
    }

    void MakeIdentity() noexcept;

    AffineTransform &Multiply(const AffineTransform &other) noexcept;
    AffineTransform &Scale(double s) noexcept;
    AffineTransform &Scale(double sx, double sy) noexcept;
    // Same as Scale(sx, sy).
    AffineTransform &ScaleNonUniform(double sx, double sy) noexcept;
    AffineTransform &Scale(const FloatSize &s) noexcept;
    AffineTransform &Rotate(double angleInDegrees) noexcept;
    AffineTransform &RotateRadians(double angle) noexcept;
    AffineTransform &RotateFromVector(double x, double y) noexcept;
    AffineTransform &Translate(double tx, double ty) noexcept;
    AffineTransform &Translate(const FloatPoint &t) noexcept;
    AffineTransform &Translate(const FloatSize &t) noexcept;
    AffineTransform &Shear(double sx, double sy) noexcept;
    AffineTransform &FlipX() noexcept;
    AffineTransform &FlipY() noexcept;
    AffineTransform &Skew(double angleX, double angleY) noexcept;
    AffineTransform &SkewX(double angle) noexcept;
    AffineTransform &SkewY(double angle) noexcept;

    // These functions get the length of an axis-aligned unit vector
    // once it has been mapped through the transform
    KRYS_NODISCARD double XScale() const noexcept;
    KRYS_NODISCARD double YScale() const noexcept;

    // If you call this, you're probably doing it wrong.
    KRYS_NODISCARD bool IsInvertible() const noexcept;
    KRYS_NODISCARD Maybe<AffineTransform> Inverse() const noexcept;

    void Blend(const AffineTransform &from, double progress,
               CompositeOperation compositeOperation = CompositeOperation::Replace) noexcept;

    KRYS_NODISCARD TransformationMatrix ToTransformationMatrix() const noexcept;

    KRYS_NODISCARD constexpr bool IsIdentityOrTranslation() const noexcept
    {
      return _transform[0] == 1 && _transform[1] == 0 && _transform[2] == 0 && _transform[3] == 1;
    }

    KRYS_NODISCARD constexpr bool IsIdentityOrTranslationOrFlipped() const noexcept
    {
      return _transform[0] == 1 && _transform[1] == 0 && _transform[2] == 0
             && (_transform[3] == 1 || _transform[3] == -1);
    }

    KRYS_NODISCARD constexpr bool IsRotateOrShear() const noexcept
    {
      return _transform[1] || _transform[2];
    }

    KRYS_NODISCARD constexpr bool PreservesAxisAlignment() const noexcept
    {
      return (_transform[1] == 0 && _transform[2] == 0) || (_transform[0] == 0 && _transform[3] == 0);
    }

    KRYS_NODISCARD bool IsEssentiallyEqualToAsFloats(const AffineTransform &m2) const noexcept
    {
      for (size_t i = 0; i < 6; ++i)
      {
        if (!AreEssentiallyEqual(NarrowPrecisionToFloat(_transform[i]),
                                 NarrowPrecisionToFloat(m2._transform[i])))
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD friend constexpr bool operator==(const AffineTransform &,
                                                    const AffineTransform &) noexcept = default;

    // *this = *this * t (i.e., a multRight)
    AffineTransform &operator*=(const AffineTransform &t) noexcept
    {
      return Multiply(t);
    }

    // result = *this * t (i.e., a multRight)
    KRYS_NODISCARD AffineTransform operator*(const AffineTransform &t) const noexcept
    {
      AffineTransform result = *this;
      result *= t;
      return result;
    }

    KRYS_NODISCARD constexpr Span<const double, 6> AsSpan() const noexcept
    {
      return Span<const double, 6>(_transform);
    }

    KRYS_NODISCARD static AffineTransform MakeTranslation(FloatSize delta) noexcept
    {
      return AffineTransform(1, 0, 0, 1, delta.Width(), delta.Height());
    }

    KRYS_NODISCARD static AffineTransform MakeScale(FloatSize scale) noexcept
    {
      return AffineTransform(scale.Width(), 0, 0, scale.Height(), 0, 0);
    }

    KRYS_NODISCARD static AffineTransform MakeRotation(double angleInDegrees, FloatPoint center) noexcept;
    KRYS_NODISCARD static AffineTransform MakeRotation(double angleInDegrees) noexcept;

    // decompose the matrix into its component parts
    struct DecomposedType
    {
      double ScaleX;
      double ScaleY;
      double Angle;
      double RemainderA;
      double RemainderB;
      double RemainderC;
      double RemainderD;
      double TranslateX;
      double TranslateY;
    };

    bool Decompose(DecomposedType &decomp) const noexcept;
    void Recompose(const DecomposedType &decomp) noexcept;
  };

  inline constexpr AffineTransform AffineTransform::Identity {};

  KRYS_NODISCARD AffineTransform MakeMapBetweenRects(const FloatRect &source, const FloatRect &dest) noexcept;
}

namespace krys::boo
{
  TextStream &operator<<(TextStream &ts, const AffineTransform &transform) noexcept;
}
