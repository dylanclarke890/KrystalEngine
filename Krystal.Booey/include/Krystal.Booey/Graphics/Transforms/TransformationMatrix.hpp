#pragma once

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Transforms/Quaternion.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class AffineTransform;
  class IntRect;
  class LayoutRect;
  class FloatRect;
  class FloatQuad;

  /// @brief A 4x4 transform matrix. The first index is the column, the second the row.
  class TransformationMatrix
  {
  public:
    using Matrix4 = Array<Array<double, 4>, 4>;
    using FloatMatrix4 = Array<float, 16>;

    enum class RotationSnapping : uint8
    {
      None,
      Snap90degRotations,
    };

    // Decompose the matrix into its component parts.
    struct Decomposed2Type
    {
      double ScaleX, ScaleY;
      double TranslateX, TranslateY;
      double Angle;
      double M11, M12, M21, M22;

      KRYS_NODISCARD friend bool operator==(const Decomposed2Type &,
                                            const Decomposed2Type &) noexcept = default;
    };

    struct Decomposed4Type
    {
      double ScaleX, ScaleY, ScaleZ;
      double SkewXY, SkewXZ, SkewYZ;
      boo::Quaternion Quaternion;
      double TranslateX, TranslateY, TranslateZ;
      double PerspectiveX, PerspectiveY, PerspectiveZ, PerspectiveW;

      KRYS_NODISCARD friend bool operator==(const Decomposed4Type &,
                                            const Decomposed4Type &) noexcept = default;
    };

  private:
    enum class Type : uint8
    {
      IdentityOrTranslation,
      Affine,
      Other
    };

    alignas(16) Matrix4 _matrix;

  public:
    /// @brief The identity matrix.
    static const TransformationMatrix Identity;

    constexpr TransformationMatrix() noexcept
        : _matrix {{
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1},
          }}
    {
    }

    constexpr TransformationMatrix(double a, double b, double c, double d, double e, double f) noexcept
        : _matrix {{
            {a, b, 0, 0},
            {c, d, 0, 0},
            {0, 0, 1, 0},
            {e, f, 0, 1},
          }}
    {
    }

    constexpr TransformationMatrix(double tx, double ty) noexcept
        : _matrix {{
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {tx, ty, 0, 1},
          }}
    {
    }

    constexpr TransformationMatrix(double m11, double m12, double m13, double m14, double m21, double m22,
                                   double m23, double m24, double m31, double m32, double m33, double m34,
                                   double m41, double m42, double m43, double m44) noexcept
        : _matrix {{
            {m11, m12, m13, m14},
            {m21, m22, m23, m24},
            {m31, m32, m33, m34},
            {m41, m42, m43, m44},
          }}
    {
    }

    TransformationMatrix(const AffineTransform &t) noexcept;

    KRYS_NODISCARD static TransformationMatrix FromQuaternion(const boo::Quaternion &q) noexcept;

    // Field of view in radians
    KRYS_NODISCARD static TransformationMatrix FromProjection(double fovUp, double fovDown, double fovLeft,
                                                              double fovRight, double depthNear,
                                                              double depthFar) noexcept;
    KRYS_NODISCARD static TransformationMatrix FromProjection(double fovy, double aspect, double depthNear,
                                                              double depthFar) noexcept;

    constexpr void SetMatrix(double a, double b, double c, double d, double e, double f) noexcept
    {
      SetMatrix(a, b, 0, 0, c, d, 0, 0, 0, 0, 1, 0, e, f, 0, 1);
    }

    constexpr void SetMatrix(double m11, double m12, double m13, double m14, double m21, double m22,
                             double m23, double m24, double m31, double m32, double m33, double m34,
                             double m41, double m42, double m43, double m44) noexcept
    {
      _matrix[0] = {m11, m12, m13, m14};
      _matrix[1] = {m21, m22, m23, m24};
      _matrix[2] = {m31, m32, m33, m34};
      _matrix[3] = {m41, m42, m43, m44};
    }

    constexpr TransformationMatrix &MakeIdentity() noexcept
    {
      SetMatrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
      return *this;
    }

    KRYS_NODISCARD constexpr bool IsIdentity() const noexcept
    {
      return *this == TransformationMatrix();
    }

    // This form preserves the double math from input to output.
    void Map(double x, double y, double &x2, double &y2) const noexcept
    {
      MultVecMatrix(x, y, x2, y2);
    }

    void Map4ComponentPoint(double &x, double &y, double &z, double &w) const noexcept;

    // Maps a 3D point through the transform, returning a 3D point.
    KRYS_NODISCARD FloatPoint3D MapPoint(const FloatPoint3D &p) const noexcept;

    // Maps a 2D point through the transform, returning a 2D point.
    // Note that this ignores the z component, effectively projecting the point into the z=0 plane.
    KRYS_NODISCARD FloatPoint MapPoint(const FloatPoint &p) const noexcept;

    // Like the version above, except that it rounds the mapped point to the nearest integer value.
    KRYS_NODISCARD IntPoint MapPoint(const IntPoint &p) const noexcept
    {
      return RoundedIntPoint(MapPoint(FloatPoint(p)));
    }

    // If the matrix has 3D components, the z component of the result is
    // dropped, effectively projecting the rect into the z=0 plane.
    KRYS_NODISCARD FloatRect MapRect(const FloatRect &r) const noexcept;

    // Rounds the resulting mapped rectangle out. This is helpful for bounding
    // box computations but may not be what is wanted in other contexts.
    KRYS_NODISCARD IntRect MapRect(const IntRect &r) const noexcept;
    KRYS_NODISCARD LayoutRect MapRect(const LayoutRect &r) const noexcept;

    // If the matrix has 3D components, the z component of the result is
    // dropped, effectively projecting the quad into the z=0 plane.
    KRYS_NODISCARD FloatQuad MapQuad(const FloatQuad &q) const noexcept;

    // Maps a point on the z=0 plane into a point on the plane with with the transform applied, by
    // extending a ray perpendicular to the source plane and computing the local x,y position of
    // the point where that ray intersects with the destination plane.
    KRYS_NODISCARD FloatPoint ProjectPoint(const FloatPoint &p, bool *clamped = nullptr) const noexcept;

    // Projects the four corners of the quad.
    KRYS_NODISCARD FloatQuad ProjectQuad(const FloatQuad &q, bool *clamped = nullptr) const noexcept;

    // Projects the four corners of the quad and takes a bounding box,
    // while sanitizing values created when the w component is negative.
    KRYS_NODISCARD LayoutRect ClampedBoundsOfProjectedQuad(const FloatQuad &q) const noexcept;

    KRYS_NODISCARD constexpr double M11() const noexcept
    {
      return _matrix[0][0];
    }

    constexpr void SetM11(double f) noexcept
    {
      _matrix[0][0] = f;
    }

    KRYS_NODISCARD constexpr double M12() const noexcept
    {
      return _matrix[0][1];
    }

    constexpr void SetM12(double f) noexcept
    {
      _matrix[0][1] = f;
    }

    KRYS_NODISCARD constexpr double M13() const noexcept
    {
      return _matrix[0][2];
    }

    constexpr void SetM13(double f) noexcept
    {
      _matrix[0][2] = f;
    }

    KRYS_NODISCARD constexpr double M14() const noexcept
    {
      return _matrix[0][3];
    }

    constexpr void SetM14(double f) noexcept
    {
      _matrix[0][3] = f;
    }

    KRYS_NODISCARD constexpr double M21() const noexcept
    {
      return _matrix[1][0];
    }

    constexpr void SetM21(double f) noexcept
    {
      _matrix[1][0] = f;
    }

    KRYS_NODISCARD constexpr double M22() const noexcept
    {
      return _matrix[1][1];
    }

    constexpr void SetM22(double f) noexcept
    {
      _matrix[1][1] = f;
    }

    KRYS_NODISCARD constexpr double M23() const noexcept
    {
      return _matrix[1][2];
    }

    constexpr void SetM23(double f) noexcept
    {
      _matrix[1][2] = f;
    }

    KRYS_NODISCARD constexpr double M24() const noexcept
    {
      return _matrix[1][3];
    }

    constexpr void SetM24(double f) noexcept
    {
      _matrix[1][3] = f;
    }

    KRYS_NODISCARD constexpr double M31() const noexcept
    {
      return _matrix[2][0];
    }

    constexpr void SetM31(double f) noexcept
    {
      _matrix[2][0] = f;
    }

    KRYS_NODISCARD constexpr double M32() const noexcept
    {
      return _matrix[2][1];
    }

    constexpr void SetM32(double f) noexcept
    {
      _matrix[2][1] = f;
    }

    KRYS_NODISCARD constexpr double M33() const noexcept
    {
      return _matrix[2][2];
    }

    constexpr void SetM33(double f) noexcept
    {
      _matrix[2][2] = f;
    }

    KRYS_NODISCARD constexpr double M34() const noexcept
    {
      return _matrix[2][3];
    }

    constexpr void SetM34(double f) noexcept
    {
      _matrix[2][3] = f;
    }

    KRYS_NODISCARD constexpr double M41() const noexcept
    {
      return _matrix[3][0];
    }

    constexpr void SetM41(double f) noexcept
    {
      _matrix[3][0] = f;
    }

    KRYS_NODISCARD constexpr double M42() const noexcept
    {
      return _matrix[3][1];
    }

    constexpr void SetM42(double f) noexcept
    {
      _matrix[3][1] = f;
    }

    KRYS_NODISCARD constexpr double M43() const noexcept
    {
      return _matrix[3][2];
    }

    constexpr void SetM43(double f) noexcept
    {
      _matrix[3][2] = f;
    }

    KRYS_NODISCARD constexpr double M44() const noexcept
    {
      return _matrix[3][3];
    }

    constexpr void SetM44(double f) noexcept
    {
      _matrix[3][3] = f;
    }

    KRYS_NODISCARD constexpr double A() const noexcept
    {
      return _matrix[0][0];
    }

    constexpr void SetA(double a) noexcept
    {
      _matrix[0][0] = a;
    }

    KRYS_NODISCARD constexpr double B() const noexcept
    {
      return _matrix[0][1];
    }

    constexpr void SetB(double b) noexcept
    {
      _matrix[0][1] = b;
    }

    KRYS_NODISCARD constexpr double C() const noexcept
    {
      return _matrix[1][0];
    }

    constexpr void SetC(double c) noexcept
    {
      _matrix[1][0] = c;
    }

    KRYS_NODISCARD constexpr double D() const noexcept
    {
      return _matrix[1][1];
    }

    constexpr void SetD(double d) noexcept
    {
      _matrix[1][1] = d;
    }

    KRYS_NODISCARD constexpr double E() const noexcept
    {
      return _matrix[3][0];
    }

    constexpr void SetE(double e) noexcept
    {
      _matrix[3][0] = e;
    }

    KRYS_NODISCARD constexpr double F() const noexcept
    {
      return _matrix[3][1];
    }

    constexpr void SetF(double f) noexcept
    {
      _matrix[3][1] = f;
    }

    // this = mat * this.
    TransformationMatrix &Multiply(const TransformationMatrix &mat) noexcept;

    // Identical to Multiply(TransformationMatrix&), but saving a AffineTransform -> TransformationMatrix
    // roundtrip for identity or translation matrices.
    TransformationMatrix &MultiplyAffineTransform(const AffineTransform &matrix) noexcept;

    TransformationMatrix &Scale(double s) noexcept;
    TransformationMatrix &ScaleNonUniform(double sx, double sy) noexcept;
    TransformationMatrix &Scale3d(double sx, double sy, double sz) noexcept;

    // Angle is in degrees.
    TransformationMatrix &Rotate(double angle,
                                 RotationSnapping snapping = RotationSnapping::Snap90degRotations) noexcept;
    TransformationMatrix &
      RotateRadians(double angle, RotationSnapping snapping = RotationSnapping::Snap90degRotations) noexcept;
    TransformationMatrix &RotateFromVector(double x, double y) noexcept;
    TransformationMatrix &Rotate3d(double rx, double ry, double rz,
                                   RotationSnapping snapping = RotationSnapping::Snap90degRotations) noexcept;

    // The vector (x,y,z) is normalized if it is not already. A vector of (0,0,0) uses a vector of (0,0,1).
    TransformationMatrix &Rotate3d(double x, double y, double z, double angle,
                                   RotationSnapping snapping = RotationSnapping::Snap90degRotations) noexcept;

    TransformationMatrix &Translate(double tx, double ty) noexcept;
    TransformationMatrix &Translate3d(double tx, double ty, double tz) noexcept;

    // translation added with a post-multiply
    TransformationMatrix &TranslateRight(double tx, double ty) noexcept;
    TransformationMatrix &TranslateRight3d(double tx, double ty, double tz) noexcept;

    TransformationMatrix &FlipX() noexcept;
    TransformationMatrix &FlipY() noexcept;
    TransformationMatrix &Skew(double angleX, double angleY) noexcept;

    TransformationMatrix &SkewX(double angle) noexcept
    {
      return Skew(angle, 0);
    }

    TransformationMatrix &SkewY(double angle) noexcept
    {
      return Skew(0, angle);
    }

    TransformationMatrix &ApplyPerspective(double p) noexcept;

    KRYS_NODISCARD constexpr bool HasPerspective() const noexcept
    {
      return _matrix[0][3] != 0.0f || _matrix[1][3] != 0.0f || _matrix[2][3] != 0.0f || _matrix[3][3] != 1.0f;
    }

    // Returns a transformation that maps a rect to a rect.
    KRYS_NODISCARD static TransformationMatrix RectToRect(const FloatRect &from,
                                                          const FloatRect &to) noexcept;

    // Changes the transform to:
    //
    //     scale3d(z, z, z) * mat * scale3d(1/z, 1/z, 1/z)
    //
    // Useful for mapping zoomed points to their zoomed transformed result:
    //
    //     new_mat * (scale3d(z, z, z) * x) == scale3d(z, z, z) * (mat * x)
    //
    TransformationMatrix &Zoom(double zoomFactor) noexcept;

    KRYS_NODISCARD bool IsInvertible() const noexcept;
    KRYS_NODISCARD Maybe<TransformationMatrix> Inverse() const noexcept;

    KRYS_NODISCARD bool Decompose2(Decomposed2Type &decomp) const noexcept;
    void Recompose2(const Decomposed2Type &decomp) noexcept;
    KRYS_NODISCARD bool Decompose4(Decomposed4Type &decomp) const noexcept;
    void Recompose4(const Decomposed4Type &decomp) noexcept;

    void Blend(const TransformationMatrix &from, double progress,
               CompositeOperation compositeOperation = CompositeOperation::Replace) noexcept;
    void Blend2(const TransformationMatrix &from, double progress,
                CompositeOperation compositeOperation = CompositeOperation::Replace) noexcept;
    void Blend4(const TransformationMatrix &from, double progress,
                CompositeOperation compositeOperation = CompositeOperation::Replace) noexcept;

    KRYS_NODISCARD constexpr bool IsAffine() const noexcept
    {
      return (M13() == 0 && M14() == 0 && M23() == 0 && M24() == 0 && M31() == 0 && M32() == 0 && M33() == 1
              && M34() == 0 && M43() == 0 && M44() == 1);
    }

    // Throw away the non-affine parts of the matrix (lossy!).
    void MakeAffine() noexcept;

    // Sets the 3rd row and column to (0, 0, 1, 0).
    // Should produce the same results as mapping points into 2d before
    // applying the next matrix.
    void Flatten() noexcept;

    KRYS_NODISCARD AffineTransform ToAffineTransform() const noexcept;

    KRYS_NODISCARD friend constexpr bool operator==(const TransformationMatrix &,
                                                    const TransformationMatrix &) noexcept = default;

    // *this = *this * t
    TransformationMatrix &operator*=(const TransformationMatrix &t) noexcept
    {
      return Multiply(t);
    }

    // result = *this * t
    KRYS_NODISCARD TransformationMatrix operator*(const TransformationMatrix &t) const noexcept
    {
      TransformationMatrix result = *this;
      result.Multiply(t);
      return result;
    }

    KRYS_NODISCARD constexpr bool IsIdentityOrTranslation() const noexcept
    {
      return _matrix[0][0] == 1 && _matrix[0][1] == 0 && _matrix[0][2] == 0 && _matrix[0][3] == 0
             && _matrix[1][0] == 0 && _matrix[1][1] == 1 && _matrix[1][2] == 0 && _matrix[1][3] == 0
             && _matrix[2][0] == 0 && _matrix[2][1] == 0 && _matrix[2][2] == 1 && _matrix[2][3] == 0
             && _matrix[3][3] == 1;
    }

    KRYS_NODISCARD bool IsIntegerTranslation() const noexcept;
    KRYS_NODISCARD bool ContainsOnlyFiniteValues() const noexcept;

    // Returns the matrix without 3D components.
    KRYS_NODISCARD TransformationMatrix To2dTransform() const noexcept;

    KRYS_NODISCARD FloatMatrix4 ToColumnMajorFloatArray() const noexcept;

    // A local-space layer is implicitly defined at the z = 0 plane, with its front side
    // facing the positive z-axis (i.e. a camera looking along the negative z-axis sees
    // the front side of the layer). This function checks if the transformed layer's back
    // face would be visible to a camera looking along the negative z-axis in the target space.
    KRYS_NODISCARD bool IsBackFaceVisible() const noexcept;

    KRYS_NODISCARD TransformationMatrix Transpose() const noexcept;

  private:
    // multiply passed 2D point by matrix (assume z=0)
    void MultVecMatrix(double x, double y, double &dstX, double &dstY) const noexcept;

    KRYS_NODISCARD FloatPoint InternalMapPoint(const FloatPoint &sourcePoint) const noexcept
    {
      double resultX;
      double resultY;
      MultVecMatrix(sourcePoint.X(), sourcePoint.Y(), resultX, resultY);
      return FloatPoint(static_cast<float>(resultX), static_cast<float>(resultY));
    }

    void MultVecMatrix(double x, double y, double z, double &dstX, double &dstY, double &dstZ) const noexcept;

    KRYS_NODISCARD FloatPoint3D InternalMapPoint(const FloatPoint3D &sourcePoint) const noexcept
    {
      double resultX;
      double resultY;
      double resultZ;
      MultVecMatrix(sourcePoint.X(), sourcePoint.Y(), sourcePoint.Z(), resultX, resultY, resultZ);
      return FloatPoint3D(static_cast<float>(resultX), static_cast<float>(resultY),
                          static_cast<float>(resultZ));
    }

    KRYS_NODISCARD constexpr Type GetType() const noexcept
    {
      if (!M13() && !M14() && !M23() && !M24() && !M34() && !M31() && !M32() && M33() == 1 && M44() == 1)
      {
        if (!M12() && !M21() && M11() == 1 && M22() == 1)
        {
          return Type::IdentityOrTranslation;
        }
        if (!M43())
        {
          return Type::Affine;
        }
      }
      return Type::Other;
    }
  };

  inline constexpr TransformationMatrix TransformationMatrix::Identity {};
}
