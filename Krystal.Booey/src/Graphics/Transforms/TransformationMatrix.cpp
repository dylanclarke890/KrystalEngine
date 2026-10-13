#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>

namespace krys::boo
{
  //
  // Adapted from Matrix Inversion by Richard Carling, Graphics Gems
  // <http://tog.acm.org/GraphicsGems/index.html>.

  // EULA: The Graphics Gems code is copyright-protected. In other words, you cannot claim the text of the
  // code as your own and resell it. Using the code is permitted in any program, product, or library,
  // non-commercial or commercial. Giving credit is not required, though is a nice gesture. The code comes
  // as-is, and if there are any flaws or problems with any Gems code, nobody involved with Gems - authors,
  // editors, publishers, or webmasters - are to be held responsible. Basically, don't be a jerk, and remember
  // that anything free comes with no guarantee.

  // A clarification about the storage of matrix elements
  //
  // This class uses a 2 dimensional array internally to store the elements of the matrix. The first index
  // into the array refers to the column that the element lies in; the second index refers to the row.
  //
  // In other words, this is the layout of the matrix:
  //
  // | _matrix[0][0] _matrix[1][0] _matrix[2][0] _matrix[3][0] |
  // | _matrix[0][1] _matrix[1][1] _matrix[2][1] _matrix[3][1] |
  // | _matrix[0][2] _matrix[1][2] _matrix[2][2] _matrix[3][2] |
  // | _matrix[0][3] _matrix[1][3] _matrix[2][3] _matrix[3][3] |

  namespace
  {
    using Matrix4 = TransformationMatrix::Matrix4;
    using Vector4 = Array<double, 4>;
    using Vector3 = Array<double, 3>;

    // inverse(original_matrix, inverse_matrix)
    //
    // calculate the inverse of a 4x4 matrix
    //
    // -1
    // A  = ___1__ adjoint A
    //       det A

    // double = determinant2x2(double a, double b, double c, double d)
    //
    // calculate the determinant of a 2x2 matrix.
    KRYS_NODISCARD double Determinant2x2(double a, double b, double c, double d) noexcept
    {
      return a * d - b * c;
    }

    // double = determinant3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3)
    //
    // Calculate the determinant of a 3x3 matrix
    // in the form
    //
    //     | a1,  b1,  c1 |
    //     | a2,  b2,  c2 |
    //     | a3,  b3,  c3 |
    KRYS_NODISCARD double Determinant3x3(double a1, double a2, double a3, double b1, double b2, double b3,
                                         double c1, double c2, double c3) noexcept
    {
      return a1 * Determinant2x2(b2, b3, c2, c3) - b1 * Determinant2x2(a2, a3, c2, c3)
             + c1 * Determinant2x2(a2, a3, b2, b3);
    }

    // double = determinant4x4(matrix)
    //
    // calculate the determinant of a 4x4 matrix.
    KRYS_NODISCARD double Determinant4x4(const Matrix4 &m) noexcept
    {
      // Assign to individual variable names to aid selecting
      // correct elements

      double a1 = m[0][0];
      double b1 = m[0][1];
      double c1 = m[0][2];
      double d1 = m[0][3];

      double a2 = m[1][0];
      double b2 = m[1][1];
      double c2 = m[1][2];
      double d2 = m[1][3];

      double a3 = m[2][0];
      double b3 = m[2][1];
      double c3 = m[2][2];
      double d3 = m[2][3];

      double a4 = m[3][0];
      double b4 = m[3][1];
      double c4 = m[3][2];
      double d4 = m[3][3];

      return a1 * Determinant3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4)
             - b1 * Determinant3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4)
             + c1 * Determinant3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4)
             - d1 * Determinant3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);
    }

    // adjoint( original_matrix, inverse_matrix )
    //
    //   calculate the adjoint of a 4x4 matrix
    //
    //    Let  a   denote the minor determinant of matrix A obtained by
    //         ij
    //
    //    deleting the ith row and jth column from A.
    //
    //                  i+j
    //   Let  b   = (-1)    a
    //        ij            ji
    //
    //  The matrix B = (b  ) is the adjoint of A
    //                   ij
    void Adjoint(const Matrix4 &matrix, Matrix4 &result) noexcept
    {
      // Assign to individual variable names to aid
      // selecting correct values
      double a1 = matrix[0][0];
      double b1 = matrix[0][1];
      double c1 = matrix[0][2];
      double d1 = matrix[0][3];

      double a2 = matrix[1][0];
      double b2 = matrix[1][1];
      double c2 = matrix[1][2];
      double d2 = matrix[1][3];

      double a3 = matrix[2][0];
      double b3 = matrix[2][1];
      double c3 = matrix[2][2];
      double d3 = matrix[2][3];

      double a4 = matrix[3][0];
      double b4 = matrix[3][1];
      double c4 = matrix[3][2];
      double d4 = matrix[3][3];

      // Row column labeling reversed since we transpose rows & columns
      result[0][0] = Determinant3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4);
      result[1][0] = -Determinant3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4);
      result[2][0] = Determinant3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4);
      result[3][0] = -Determinant3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);

      result[0][1] = -Determinant3x3(b1, b3, b4, c1, c3, c4, d1, d3, d4);
      result[1][1] = Determinant3x3(a1, a3, a4, c1, c3, c4, d1, d3, d4);
      result[2][1] = -Determinant3x3(a1, a3, a4, b1, b3, b4, d1, d3, d4);
      result[3][1] = Determinant3x3(a1, a3, a4, b1, b3, b4, c1, c3, c4);

      result[0][2] = Determinant3x3(b1, b2, b4, c1, c2, c4, d1, d2, d4);
      result[1][2] = -Determinant3x3(a1, a2, a4, c1, c2, c4, d1, d2, d4);
      result[2][2] = Determinant3x3(a1, a2, a4, b1, b2, b4, d1, d2, d4);
      result[3][2] = -Determinant3x3(a1, a2, a4, b1, b2, b4, c1, c2, c4);

      result[0][3] = -Determinant3x3(b1, b2, b3, c1, c2, c3, d1, d2, d3);
      result[1][3] = Determinant3x3(a1, a2, a3, c1, c2, c3, d1, d2, d3);
      result[2][3] = -Determinant3x3(a1, a2, a3, b1, b2, b3, d1, d2, d3);
      result[3][3] = Determinant3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
    }

    // Returns false if the matrix is not invertible
    KRYS_NODISCARD bool Inverse(const Matrix4 &matrix, Matrix4 &result) noexcept
    {
      // Calculate the 4x4 determinant
      // If the determinant is zero,
      // then the inverse matrix is not unique.
      double determinant = Determinant4x4(matrix);
      if (!std::isnormal(determinant))
      {
        return false;
      }

      // Scale the adjoint matrix to get the inverse
      double inverseDeterminant = 1 / determinant;

      // Calculate the adjoint matrix
      Adjoint(matrix, result);

      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          result[i][j] = result[i][j] * inverseDeterminant;
        }
      }

      return true;
    }

    // End of code adapted from Matrix Inversion by Richard Carling

    // Perform a decomposition on the passed matrix, return false if unsuccessful
    // From Graphics Gems: unmatrix.c

    // Transpose rotation portion of matrix a, return b
    void TransposeMatrix4(const Matrix4 &a, Matrix4 &b) noexcept
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          b[i][j] = a[j][i];
        }
      }
    }

    // Multiply a homogeneous point by a matrix and return the transformed point
    void V4MulPointByMatrix(const Vector4 &p, const Matrix4 &m, Vector4 &result) noexcept
    {
      result[0] = (p[0] * m[0][0]) + (p[1] * m[1][0]) + (p[2] * m[2][0]) + (p[3] * m[3][0]);
      result[1] = (p[0] * m[0][1]) + (p[1] * m[1][1]) + (p[2] * m[2][1]) + (p[3] * m[3][1]);
      result[2] = (p[0] * m[0][2]) + (p[1] * m[1][2]) + (p[2] * m[2][2]) + (p[3] * m[3][2]);
      result[3] = (p[0] * m[0][3]) + (p[1] * m[1][3]) + (p[2] * m[2][3]) + (p[3] * m[3][3]);
    }

    KRYS_NODISCARD double V3Length(const Vector3 &a) noexcept
    {
      return std::hypot(a[0], a[1], a[2]);
    }

    void V3Scale(Vector3 &v, double desiredLength) noexcept
    {
      double len = V3Length(v);
      if (len != 0)
      {
        double l = desiredLength / len;
        v[0] *= l;
        v[1] *= l;
        v[2] *= l;
      }
    }

    KRYS_NODISCARD double V3Dot(const Vector3 &a, const Vector3 &b) noexcept
    {
      return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
    }

    // Make a linear combination of two vectors and return the result.
    // result = (a * ascl) + (b * bscl)
    void V3Combine(const Vector3 &a, const Vector3 &b, Vector3 &result, double ascl, double bscl) noexcept
    {
      result[0] = (ascl * a[0]) + (bscl * b[0]);
      result[1] = (ascl * a[1]) + (bscl * b[1]);
      result[2] = (ascl * a[2]) + (bscl * b[2]);
    }

    // Return the cross product result = a cross b */
    void V3Cross(const Vector3 &a, const Vector3 &b, Vector3 &result) noexcept
    {
      result[0] = (a[1] * b[2]) - (a[2] * b[1]);
      result[1] = (a[2] * b[0]) - (a[0] * b[2]);
      result[2] = (a[0] * b[1]) - (a[1] * b[0]);
    }

    KRYS_NODISCARD bool Decompose2(const Matrix4 &matrix,
                                   TransformationMatrix::Decomposed2Type &result) noexcept
    {
      double row0x = matrix[0][0];
      double row0y = matrix[0][1];
      double row1x = matrix[1][0];
      double row1y = matrix[1][1];
      result.TranslateX = matrix[3][0];
      result.TranslateY = matrix[3][1];

      // Compute scaling factors.
      result.ScaleX = std::hypot(row0x, row0y);
      result.ScaleY = std::hypot(row1x, row1y);

      // If determinant is negative, one axis was flipped.
      double determinant = row0x * row1y - row0y * row1x;
      if (determinant < 0)
      {
        // Flip axis with minimum unit vector dot product.
        if (row0x < row1y)
        {
          result.ScaleX = -result.ScaleX;
        }
        else
        {
          result.ScaleY = -result.ScaleY;
        }
      }

      // Renormalize matrix to remove scale.
      if (result.ScaleX)
      {
        row0x *= 1 / result.ScaleX;
        row0y *= 1 / result.ScaleX;
      }
      if (result.ScaleY)
      {
        row1x *= 1 / result.ScaleY;
        row1y *= 1 / result.ScaleY;
      }

      // Compute rotation and renormalize matrix.
      result.Angle = std::atan2(row0y, row0x);

      if (result.Angle)
      {
        // Rotate(-angle) = [cos(angle), sin(angle), -sin(angle), cos(angle)]
        //                = [row0x, -row0y, row0y, row0x]
        // Thanks to the normalization above.
        double sn = -row0y;
        double cs = row0x;
        double m11 = row0x, m12 = row0y;
        double m21 = row1x, m22 = row1y;

        row0x = cs * m11 + sn * m21;
        row0y = cs * m12 + sn * m22;
        row1x = -sn * m11 + cs * m21;
        row1y = -sn * m12 + cs * m22;
      }

      result.M11 = row0x;
      result.M12 = row0y;
      result.M21 = row1x;
      result.M22 = row1y;

      // Convert into degrees because our rotation functions expect it.
      result.Angle = rad2deg(result.Angle);

      return true;
    }

    KRYS_NODISCARD bool Decompose4(const Matrix4 &mat, TransformationMatrix::Decomposed4Type &result) noexcept
    {
      Matrix4 localMatrix = mat;

      // Normalize the matrix.
      if (localMatrix[3][3] == 0)
      {
        return false;
      }

      int i, j;
      for (i = 0; i < 4; i++)
      {
        for (j = 0; j < 4; j++)
        {
          localMatrix[i][j] /= localMatrix[3][3];
        }
      }

      // perspectiveMatrix is used to solve for perspective, but it also provides
      // an easy way to test for singularity of the upper 3x3 component.
      Matrix4 perspectiveMatrix = localMatrix;
      for (i = 0; i < 3; i++)
      {
        perspectiveMatrix[i][3] = 0;
      }
      perspectiveMatrix[3][3] = 1;

      if (!std::isnormal(Determinant4x4(perspectiveMatrix)))
      {
        return false;
      }

      // First, isolate perspective. This is the messiest.
      if (localMatrix[0][3] != 0 || localMatrix[1][3] != 0 || localMatrix[2][3] != 0)
      {
        // rightHandSide is the right hand side of the equation.
        Vector4 rightHandSide;
        rightHandSide[0] = localMatrix[0][3];
        rightHandSide[1] = localMatrix[1][3];
        rightHandSide[2] = localMatrix[2][3];
        rightHandSide[3] = localMatrix[3][3];

        // Solve the equation by inverting perspectiveMatrix and multiplying
        // rightHandSide by the inverse. (This is the easiest way, not
        // necessarily the best.)
        Matrix4 inversePerspectiveMatrix, transposedInversePerspectiveMatrix;
        if (!Inverse(perspectiveMatrix, inversePerspectiveMatrix))
        {
          return false;
        }
        TransposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);

        Vector4 perspectivePoint;
        V4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);

        result.PerspectiveX = perspectivePoint[0];
        result.PerspectiveY = perspectivePoint[1];
        result.PerspectiveZ = perspectivePoint[2];
        result.PerspectiveW = perspectivePoint[3];

        // Clear the perspective partition
        localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = 0;
        localMatrix[3][3] = 1;
      }
      else
      {
        // No perspective.
        result.PerspectiveX = result.PerspectiveY = result.PerspectiveZ = 0;
        result.PerspectiveW = 1;
      }

      // Next take care of translation (easy).
      result.TranslateX = localMatrix[3][0];
      localMatrix[3][0] = 0;
      result.TranslateY = localMatrix[3][1];
      localMatrix[3][1] = 0;
      result.TranslateZ = localMatrix[3][2];
      localMatrix[3][2] = 0;

      // Note: Deviating from the spec in terms of variable naming. The matrix is
      // stored on column major order and not row major. Using the variable 'row'
      // instead of 'column' in the spec pseudocode has been the source of
      // confusion, specifically in sorting out rotations.
      Array<Vector3, 3> column;
      Vector3 pdum3;

      // Now get scale and shear.
      for (i = 0; i < 3; i++)
      {
        column[i][0] = localMatrix[i][0];
        column[i][1] = localMatrix[i][1];
        column[i][2] = localMatrix[i][2];
      }

      // Compute X scale factor and normalize the first column.
      result.ScaleX = V3Length(column[0]);
      V3Scale(column[0], 1.0);

      // Compute XY shear factor and make 2nd column orthogonal to 1st.
      result.SkewXY = V3Dot(column[0], column[1]);
      V3Combine(column[1], column[0], column[1], 1.0, -result.SkewXY);

      // Now, compute Y scale and normalize 2nd column.
      result.ScaleY = V3Length(column[1]);
      V3Scale(column[1], 1.0);
      result.SkewXY /= result.ScaleY;

      // Compute XZ and YZ shears, orthogonalize 3rd column.
      result.SkewXZ = V3Dot(column[0], column[2]);
      V3Combine(column[2], column[0], column[2], 1.0, -result.SkewXZ);
      result.SkewYZ = V3Dot(column[1], column[2]);
      V3Combine(column[2], column[1], column[2], 1.0, -result.SkewYZ);

      // Next, get Z scale and normalize 3rd column.
      result.ScaleZ = V3Length(column[2]);
      V3Scale(column[2], 1.0);
      result.SkewXZ /= result.ScaleZ;
      result.SkewYZ /= result.ScaleZ;

      // At this point, the matrix (in column[]) is orthonormal.
      // Check for a coordinate system flip. If the determinant
      // is -1, then negate the matrix and the scaling factors.
      V3Cross(column[1], column[2], pdum3);
      if (V3Dot(column[0], pdum3) < 0)
      {
        result.ScaleX *= -1;
        result.ScaleY *= -1;
        result.ScaleZ *= -1;

        for (i = 0; i < 3; i++)
        {
          column[i][0] *= -1;
          column[i][1] *= -1;
          column[i][2] *= -1;
        }
      }

      // Lastly, compute the quaternions.
      // See https://en.wikipedia.org/wiki/Rotation_matrix#Quaternion.
      // Note: deviating from spec (http://www.w3.org/TR/css3-transforms/)
      // which has a degenerate case when the trace (t) of the orthonormal matrix
      // (Q) approaches -1. In the Wikipedia article, Q_ij is indexing on row then
      // column. Thus, Q_ij = column[j][i].

      // The following are equivalent represnetations of the rotation matrix:
      //
      // Axis-angle form:
      //
      //      [ c+(1-c)x^2  (1-c)xy-sz  (1-c)xz+sy ]    c = cos theta
      // R =  [ (1-c)xy+sz  c+(1-c)y^2  (1-c)yz-sx ]    s = sin theta
      //      [ (1-c)xz-sy  (1-c)yz+sx  c+(1-c)z^2 ]    [x,y,z] = axis or rotation
      //
      // The sum of the diagonal elements (trace) is a simple function of the cosine
      // of the angle. The w component of the quaternion is cos(theta/2), and we
      // make use of the double angle formula to directly compute w from the
      // trace. Differences between pairs of skew symmetric elements in this matrix
      // isolate the remaining components. Since w can be zero (also numerically
      // unstable if near zero), we cannot rely solely on this approach to compute
      // the quaternion components.
      //
      // Quaternion form:
      //
      //       [ 1-2(y^2+z^2)    2(xy-zw)      2(xz+yw)   ]
      //  r =  [   2(xy+zw)    1-2(x^2+z^2)    2(yz-xw)   ]    q = (x,y,y,w)
      //       [   2(xz-yw)      2(yz+xw)    1-2(x^2+y^2) ]
      //
      // Different linear combinations of the diagonal elements isolates x, y or z.
      // Sums or differences between skew symmetric elements isolate the remainder.

      double r, s, t, x, y, z, w;

      t = column[0][0] + column[1][1] + column[2][2];

      // https://en.wikipedia.org/wiki/Rotation_matrix#Quaternion
      if (1 + t > 0.001)
      {
        // Numerically stable as long as 1+t is not close to zero. Otherwise use the
        // diagonal element with the greatest value to compute the quaternions.
        r = std::sqrt(1.0 + t);
        s = 0.5 / r;
        w = 0.5 * r;
        x = (column[1][2] - column[2][1]) * s;
        y = (column[2][0] - column[0][2]) * s;
        z = (column[0][1] - column[1][0]) * s;
      }
      else if (column[0][0] > column[1][1] && column[0][0] > column[2][2])
      {
        // Q_xx is largest.
        r = std::sqrt(1.0 + column[0][0] - column[1][1] - column[2][2]);
        s = 0.5 / r;
        x = 0.5 * r;
        y = (column[1][0] - column[0][1]) * s;
        z = (column[2][0] + column[0][2]) * s;
        w = (column[1][2] - column[2][1]) * s;
      }
      else if (column[1][1] > column[2][2])
      {
        // Q_yy is largest.
        r = std::sqrt(1.0 - column[0][0] + column[1][1] - column[2][2]);
        s = 0.5 / r;
        x = (column[1][0] + column[0][1]) * s;
        y = 0.5 * r;
        z = (column[2][1] + column[1][2]) * s;
        w = (column[2][0] - column[0][2]) * s;
      }
      else
      {
        // Q_zz is largest.
        r = std::sqrt(1.0 - column[0][0] - column[1][1] + column[2][2]);
        s = 0.5 / r;
        x = (column[2][0] + column[0][2]) * s;
        y = (column[2][1] + column[1][2]) * s;
        z = 0.5 * r;
        w = (column[0][1] - column[1][0]) * s;
      }

      result.Quaternion.X = x;
      result.Quaternion.Y = y;
      result.Quaternion.Z = z;
      result.Quaternion.W = w;

      return true;
    }

    // End of Supporting Math Functions

    KRYS_NODISCARD float ClampEdgeValue(float f) noexcept
    {
      krys_debug_assert(!std::isnan(f));
      return std::min<float>(std::max<float>(f, -LayoutUnit::Max() / 2), LayoutUnit::Max() / 2);
    }

    KRYS_NODISCARD double RoundEpsilonToZero(double val) noexcept
    {
      if (-Limits<double>::epsilon() < val && val < Limits<double>::epsilon())
      {
        return 0.0f;
      }
      return val;
    }

    void BlendFloat(double &from, double to, double progress, CompositeOperation compositeOperation) noexcept
    {
      switch (compositeOperation)
      {
        case CompositeOperation::Replace: from = from + (to - from) * progress; break;
        case CompositeOperation::Accumulate:
          krys_debug_assert(progress == 1.0);
          from += to - 1;
          break;
        case CompositeOperation::Add:
          krys_debug_assert(progress == 1.0);
          from += to;
          break;
      }
    }
  }

  TransformationMatrix::TransformationMatrix(const AffineTransform &t) noexcept
  {
    SetMatrix(t.A(), t.B(), t.C(), t.D(), t.E(), t.F());
  }

  TransformationMatrix TransformationMatrix::FromQuaternion(const boo::Quaternion &q) noexcept
  {
    double xx = q.X * q.X;
    double yy = q.Y * q.Y;
    double zz = q.Z * q.Z;
    double xz = q.X * q.Z;
    double xy = q.X * q.Y;
    double yz = q.Y * q.Z;
    double xw = q.W * q.X;
    double yw = q.W * q.Y;
    double zw = q.W * q.Z;

    return TransformationMatrix(1 - 2 * (yy + zz), 2 * (xy + zw), 2 * (xz - yw), 0, 2 * (xy - zw),
                                1 - 2 * (xx + zz), 2 * (yz + xw), 0, 2 * (xz + yw), 2 * (yz - xw),
                                1 - 2 * (xx + yy), 0, 0, 0, 0, 1);
  }

  TransformationMatrix TransformationMatrix::FromProjection(double fovUp, double fovDown, double fovLeft,
                                                            double fovRight, double depthNear,
                                                            double depthFar) noexcept
  {
    double upTan = std::tan(fovUp);
    double downTan = std::tan(fovDown);
    double leftTan = std::tan(fovLeft);
    double rightTan = std::tan(fovRight);
    double xScale = 2.0 / (leftTan + rightTan);
    double yScale = 2.0 / (upTan + downTan);
    double invDepth = 1.0 / (depthNear - depthFar);

    return TransformationMatrix(xScale, 0.0f, 0.0f, 0.0f, 0.0f, yScale, 0.0f, 0.0f,
                                (leftTan - rightTan) * xScale * -0.5, (upTan - downTan) * yScale * 0.5,
                                (depthNear + depthFar) * invDepth, -1.0f, 0.0f, 0.0f,
                                (2.0f * depthFar * depthNear) * invDepth, 0.0f);
  }

  TransformationMatrix TransformationMatrix::FromProjection(double fovy, double aspect, double depthNear,
                                                            double depthFar) noexcept
  {
    double f = 1.0f / std::tan(static_cast<float>(fovy / 2));
    double invDepth = 1.0f / (depthNear - depthFar);

    return TransformationMatrix(f / aspect, 0.0f, 0.0f, 0.0f, 0.0f, f, 0.0f, 0.0f, 0.0f, 0.0f,
                                (depthFar + depthNear) * invDepth, -1.0f, 0.0f, 0.0f,
                                (2.0f * depthFar * depthNear) * invDepth, 0.0f);
  }

  TransformationMatrix &TransformationMatrix::Scale(double s) noexcept
  {
    return ScaleNonUniform(s, s);
  }

  TransformationMatrix &TransformationMatrix::RotateFromVector(double x, double y) noexcept
  {
    return RotateRadians(std::atan2(y, x));
  }

  TransformationMatrix &TransformationMatrix::FlipX() noexcept
  {
    return ScaleNonUniform(-1.0, 1.0);
  }

  TransformationMatrix &TransformationMatrix::FlipY() noexcept
  {
    return ScaleNonUniform(1.0, -1.0);
  }

  FloatPoint TransformationMatrix::ProjectPoint(const FloatPoint &p, bool *clamped) const noexcept
  {
    // This is basically raytracing. We have a point in the destination
    // plane with z=0, and we cast a ray parallel to the z-axis from that
    // point to find the z-position at which it intersects the z=0 plane
    // with the transform applied. Once we have that point we apply the
    // inverse transform to find the corresponding point in the source
    // space.
    //
    // Given a plane with normal Pn, and a ray starting at point R0 and
    // with direction defined by the vector Rd, we can find the
    // intersection point as a distance d from R0 in units of Rd by:
    //
    // d = -dot (Pn', R0) / dot (Pn', Rd)
    if (clamped)
    {
      *clamped = false;
    }

    if (M33() == 0)
    {
      // In this case, the projection plane is parallel to the ray we are trying to
      // trace, and there is no well-defined value for the projection.
      return FloatPoint();
    }

    double x = p.X();
    double y = p.Y();
    double z = -(M13() * x + M23() * y + M43()) / M33();

    // FIXME: use multVecMatrix()
    double outX = x * M11() + y * M21() + z * M31() + M41();
    double outY = x * M12() + y * M22() + z * M32() + M42();

    double w = x * M14() + y * M24() + z * M34() + M44();
    if (w <= 0)
    {
      // Using int max causes overflow when other code uses the projected point. To
      // represent infinity yet reduce the risk of overflow, we use a large but
      // not-too-large number here when clamping.
      const int largeNumber = 100'000'000 / FixedPointDenominator;
      outX = std::copysign(largeNumber, outX);
      outY = std::copysign(largeNumber, outY);
      if (clamped)
      {
        *clamped = true;
      }
    }
    else if (w != 1)
    {
      outX /= w;
      outY /= w;
    }

    return FloatPoint(static_cast<float>(outX), static_cast<float>(outY));
  }

  FloatQuad TransformationMatrix::ProjectQuad(const FloatQuad &q, bool *clamped) const noexcept
  {
    FloatQuad projectedQuad;

    bool clamped1 = false;
    bool clamped2 = false;
    bool clamped3 = false;
    bool clamped4 = false;

    projectedQuad.SetP1(ProjectPoint(q.P1(), &clamped1));
    projectedQuad.SetP2(ProjectPoint(q.P2(), &clamped2));
    projectedQuad.SetP3(ProjectPoint(q.P3(), &clamped3));
    projectedQuad.SetP4(ProjectPoint(q.P4(), &clamped4));

    if (clamped)
    {
      *clamped = clamped1 || clamped2 || clamped3 || clamped4;
    }

    // If all points on the quad had w < 0, then the entire quad would not be visible to the projected
    // surface.
    bool everythingWasClipped = clamped1 && clamped2 && clamped3 && clamped4;
    if (everythingWasClipped)
    {
      return FloatQuad();
    }

    return projectedQuad;
  }

  LayoutRect TransformationMatrix::ClampedBoundsOfProjectedQuad(const FloatQuad &q) const noexcept
  {
    FloatRect mappedQuadBounds = ProjectQuad(q).BoundingBox();

    float left = ClampEdgeValue(std::floor(mappedQuadBounds.X()));
    float top = ClampEdgeValue(std::floor(mappedQuadBounds.Y()));

    float right;
    if (std::isinf(mappedQuadBounds.X()) && std::isinf(mappedQuadBounds.Width()))
    {
      right = LayoutUnit::Max() / 2;
    }
    else
    {
      right = ClampEdgeValue(std::ceil(mappedQuadBounds.MaxX()));
    }

    float bottom;
    if (std::isinf(mappedQuadBounds.Y()) && std::isinf(mappedQuadBounds.Height()))
    {
      bottom = LayoutUnit::Max() / 2;
    }
    else
    {
      bottom = ClampEdgeValue(std::ceil(mappedQuadBounds.MaxY()));
    }

    return LayoutRect(LayoutUnit::Clamp(left), LayoutUnit::Clamp(top), LayoutUnit::Clamp(right - left),
                      LayoutUnit::Clamp(bottom - top));
  }

  void TransformationMatrix::Map4ComponentPoint(double &x, double &y, double &z, double &w) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      x += _matrix[3][0];
      y += _matrix[3][1];
      z += _matrix[3][2];
      return;
    }

    Vector4 input = {x, y, z, w};
    Vector4 result;
    V4MulPointByMatrix(input, _matrix, result);

    x = result[0];
    y = result[1];
    z = result[2];
    w = result[3];
  }

  FloatPoint TransformationMatrix::MapPoint(const FloatPoint &p) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      return FloatPoint(p.X() + static_cast<float>(_matrix[3][0]), p.Y() + static_cast<float>(_matrix[3][1]));
    }

    return InternalMapPoint(p);
  }

  FloatPoint3D TransformationMatrix::MapPoint(const FloatPoint3D &p) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      return FloatPoint3D(p.X() + static_cast<float>(_matrix[3][0]),
                          p.Y() + static_cast<float>(_matrix[3][1]),
                          p.Z() + static_cast<float>(_matrix[3][2]));
    }

    return InternalMapPoint(p);
  }

  IntRect TransformationMatrix::MapRect(const IntRect &rect) const noexcept
  {
    return EnclosingIntRect(MapRect(FloatRect(rect)));
  }

  LayoutRect TransformationMatrix::MapRect(const LayoutRect &r) const noexcept
  {
    return EnclosingLayoutRect(MapRect(FloatRect(r)));
  }

  FloatRect TransformationMatrix::MapRect(const FloatRect &r) const noexcept
  {
    auto type = GetType();
    if (type == Type::IdentityOrTranslation)
    {
      FloatRect mappedRect(r);
      mappedRect.Move(static_cast<float>(_matrix[3][0]), static_cast<float>(_matrix[3][1]));
      return mappedRect;
    }

    float minX = r.X();
    float minY = r.Y();
    float maxX = r.MaxX();
    float maxY = r.MaxY();

    if (type == Type::Affine)
    {
      double a = M11();
      double b = M12();
      double c = M21();
      double d = M22();

      double minResultX;
      double minResultY;
      double maxResultX;
      double maxResultY;

      if (a > 0)
      {
        maxResultX = a * maxX;
        minResultX = a * minX;
      }
      else
      {
        maxResultX = a * minX;
        minResultX = a * maxX;
      }

      if (b > 0)
      {
        maxResultY = b * maxX;
        minResultY = b * minX;
      }
      else
      {
        maxResultY = b * minX;
        minResultY = b * maxX;
      }

      if (c > 0)
      {
        maxResultX += c * maxY;
        minResultX += c * minY;
      }
      else
      {
        maxResultX += c * minY;
        minResultX += c * maxY;
      }

      if (d > 0)
      {
        maxResultY += d * maxY;
        minResultY += d * minY;
      }
      else
      {
        maxResultY += d * minY;
        minResultY += d * maxY;
      }

      return FloatRect(static_cast<float>(minResultX + M41()), static_cast<float>(minResultY + M42()),
                       static_cast<float>(maxResultX - minResultX),
                       static_cast<float>(maxResultY - minResultY));
    }

    FloatQuad result;

    result.SetP1(InternalMapPoint(FloatPoint(minX, minY)));
    result.SetP2(InternalMapPoint(FloatPoint(maxX, minY)));
    result.SetP3(InternalMapPoint(FloatPoint(maxX, maxY)));
    result.SetP4(InternalMapPoint(FloatPoint(minX, maxY)));

    return result.BoundingBox();
  }

  FloatQuad TransformationMatrix::MapQuad(const FloatQuad &q) const noexcept
  {
    if (IsIdentityOrTranslation())
    {
      FloatQuad mappedQuad(q);
      mappedQuad.Move(static_cast<float>(_matrix[3][0]), static_cast<float>(_matrix[3][1]));
      return mappedQuad;
    }

    FloatQuad result;
    result.SetP1(InternalMapPoint(q.P1()));
    result.SetP2(InternalMapPoint(q.P2()));
    result.SetP3(InternalMapPoint(q.P3()));
    result.SetP4(InternalMapPoint(q.P4()));
    return result;
  }

  TransformationMatrix &TransformationMatrix::ScaleNonUniform(double sx, double sy) noexcept
  {
    _matrix[0][0] *= sx;
    _matrix[0][1] *= sx;
    _matrix[0][2] *= sx;
    _matrix[0][3] *= sx;

    _matrix[1][0] *= sy;
    _matrix[1][1] *= sy;
    _matrix[1][2] *= sy;
    _matrix[1][3] *= sy;
    return *this;
  }

  TransformationMatrix &TransformationMatrix::Scale3d(double sx, double sy, double sz) noexcept
  {
    ScaleNonUniform(sx, sy);

    _matrix[2][0] *= sz;
    _matrix[2][1] *= sz;
    _matrix[2][2] *= sz;
    _matrix[2][3] *= sz;
    return *this;
  }

  TransformationMatrix &TransformationMatrix::Rotate3d(double x, double y, double z, double angle,
                                                       RotationSnapping snapping) noexcept
  {
    // Normalize the axis of rotation
    double length = std::hypot(x, y, z);
    if (length == 0)
    {
      // A direction vector that cannot be normalized, such as [0, 0, 0], will cause the rotation to not be
      // applied.
      return *this;
    }
    else if (length != 1)
    {
      x /= length;
      y /= length;
      z /= length;
    }

    // Angles are in degrees. Switch to radians.
    angle = deg2rad(angle);

    double sinTheta = snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::sin(angle))
                                                                       : std::sin(angle);
    double cosTheta = snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::cos(angle))
                                                                       : std::cos(angle);

    TransformationMatrix mat;

    // Optimize cases where the axis is along a major axis
    // Since we've already normalized the vector we don't need to check that the
    // other two dimensions are zero
    if (x == 1.0)
    {
      mat._matrix[0][0] = 1.0;
      mat._matrix[0][1] = 0.0;
      mat._matrix[0][2] = 0.0;
      mat._matrix[1][0] = 0.0;
      mat._matrix[1][1] = cosTheta;
      mat._matrix[1][2] = sinTheta;
      mat._matrix[2][0] = 0.0;
      mat._matrix[2][1] = -sinTheta;
      mat._matrix[2][2] = cosTheta;
      mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
      mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
      mat._matrix[3][3] = 1.0;
    }
    else if (y == 1.0)
    {
      mat._matrix[0][0] = cosTheta;
      mat._matrix[0][1] = 0.0;
      mat._matrix[0][2] = -sinTheta;
      mat._matrix[1][0] = 0.0;
      mat._matrix[1][1] = 1.0;
      mat._matrix[1][2] = 0.0;
      mat._matrix[2][0] = sinTheta;
      mat._matrix[2][1] = 0.0;
      mat._matrix[2][2] = cosTheta;
      mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
      mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
      mat._matrix[3][3] = 1.0;
    }
    else if (z == 1.0)
    {
      mat._matrix[0][0] = cosTheta;
      mat._matrix[0][1] = sinTheta;
      mat._matrix[0][2] = 0.0;
      mat._matrix[1][0] = -sinTheta;
      mat._matrix[1][1] = cosTheta;
      mat._matrix[1][2] = 0.0;
      mat._matrix[2][0] = 0.0;
      mat._matrix[2][1] = 0.0;
      mat._matrix[2][2] = 1.0;
      mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
      mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
      mat._matrix[3][3] = 1.0;
    }
    else
    {
      // This case is the rotation about an arbitrary unit vector.
      //
      // Formula is adapted from Wikipedia article on Rotation matrix,
      // http://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
      //
      // An alternate resource with the same matrix: http://www.fastgraph.com/makegames/3drotation/
      //
      double oneMinusCosTheta = 1 - cosTheta;
      mat._matrix[0][0] = cosTheta + x * x * oneMinusCosTheta;
      mat._matrix[0][1] = y * x * oneMinusCosTheta + z * sinTheta;
      mat._matrix[0][2] = z * x * oneMinusCosTheta - y * sinTheta;
      mat._matrix[1][0] = x * y * oneMinusCosTheta - z * sinTheta;
      mat._matrix[1][1] = cosTheta + y * y * oneMinusCosTheta;
      mat._matrix[1][2] = z * y * oneMinusCosTheta + x * sinTheta;
      mat._matrix[2][0] = x * z * oneMinusCosTheta + y * sinTheta;
      mat._matrix[2][1] = y * z * oneMinusCosTheta - x * sinTheta;
      mat._matrix[2][2] = cosTheta + z * z * oneMinusCosTheta;
      mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
      mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
      mat._matrix[3][3] = 1.0;
    }
    Multiply(mat);
    return *this;
  }

  TransformationMatrix &TransformationMatrix::Rotate(double angle, RotationSnapping snapping) noexcept
  {
    if (!std::fmod(angle, 360))
    {
      return *this;
    }

    return RotateRadians(deg2rad(angle), snapping);
  }

  TransformationMatrix &TransformationMatrix::RotateRadians(double angle, RotationSnapping snapping) noexcept
  {
    double sinZ = snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::sin(angle))
                                                                   : std::sin(angle);
    double cosZ = snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::cos(angle))
                                                                   : std::cos(angle);
    Multiply({cosZ, sinZ, -sinZ, cosZ, 0, 0});
    return *this;
  }

  TransformationMatrix &TransformationMatrix::Rotate3d(double rx, double ry, double rz,
                                                       RotationSnapping snapping) noexcept
  {
    // Angles are in degrees. Switch to radians.
    rx = deg2rad(rx);
    ry = deg2rad(ry);
    rz = deg2rad(rz);

    TransformationMatrix mat;

    double sinTheta =
      snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::sin(rz)) : std::sin(rz);
    double cosTheta =
      snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::cos(rz)) : std::cos(rz);

    mat._matrix[0][0] = cosTheta;
    mat._matrix[0][1] = sinTheta;
    mat._matrix[0][2] = 0.0;
    mat._matrix[1][0] = -sinTheta;
    mat._matrix[1][1] = cosTheta;
    mat._matrix[1][2] = 0.0;
    mat._matrix[2][0] = 0.0;
    mat._matrix[2][1] = 0.0;
    mat._matrix[2][2] = 1.0;
    mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
    mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
    mat._matrix[3][3] = 1.0;

    TransformationMatrix rmat(mat);

    sinTheta =
      snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::sin(ry)) : std::sin(ry);
    cosTheta =
      snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::cos(ry)) : std::cos(ry);

    mat._matrix[0][0] = cosTheta;
    mat._matrix[0][1] = 0.0;
    mat._matrix[0][2] = -sinTheta;
    mat._matrix[1][0] = 0.0;
    mat._matrix[1][1] = 1.0;
    mat._matrix[1][2] = 0.0;
    mat._matrix[2][0] = sinTheta;
    mat._matrix[2][1] = 0.0;
    mat._matrix[2][2] = cosTheta;
    mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
    mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
    mat._matrix[3][3] = 1.0;

    rmat.Multiply(mat);

    sinTheta =
      snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::sin(rx)) : std::sin(rx);
    cosTheta =
      snapping == RotationSnapping::Snap90degRotations ? RoundEpsilonToZero(std::cos(rx)) : std::cos(rx);

    mat._matrix[0][0] = 1.0;
    mat._matrix[0][1] = 0.0;
    mat._matrix[0][2] = 0.0;
    mat._matrix[1][0] = 0.0;
    mat._matrix[1][1] = cosTheta;
    mat._matrix[1][2] = sinTheta;
    mat._matrix[2][0] = 0.0;
    mat._matrix[2][1] = -sinTheta;
    mat._matrix[2][2] = cosTheta;
    mat._matrix[0][3] = mat._matrix[1][3] = mat._matrix[2][3] = 0.0;
    mat._matrix[3][0] = mat._matrix[3][1] = mat._matrix[3][2] = 0.0;
    mat._matrix[3][3] = 1.0;

    rmat.Multiply(mat);

    Multiply(rmat);
    return *this;
  }

  TransformationMatrix &TransformationMatrix::Translate(double tx, double ty) noexcept
  {
    _matrix[3][0] += tx * _matrix[0][0] + ty * _matrix[1][0];
    _matrix[3][1] += tx * _matrix[0][1] + ty * _matrix[1][1];
    _matrix[3][2] += tx * _matrix[0][2] + ty * _matrix[1][2];
    _matrix[3][3] += tx * _matrix[0][3] + ty * _matrix[1][3];
    return *this;
  }

  TransformationMatrix &TransformationMatrix::Translate3d(double tx, double ty, double tz) noexcept
  {
    _matrix[3][0] += tx * _matrix[0][0] + ty * _matrix[1][0] + tz * _matrix[2][0];
    _matrix[3][1] += tx * _matrix[0][1] + ty * _matrix[1][1] + tz * _matrix[2][1];
    _matrix[3][2] += tx * _matrix[0][2] + ty * _matrix[1][2] + tz * _matrix[2][2];
    _matrix[3][3] += tx * _matrix[0][3] + ty * _matrix[1][3] + tz * _matrix[2][3];
    return *this;
  }

  TransformationMatrix &TransformationMatrix::TranslateRight(double tx, double ty) noexcept
  {
    if (tx != 0)
    {
      _matrix[0][0] += _matrix[0][3] * tx;
      _matrix[1][0] += _matrix[1][3] * tx;
      _matrix[2][0] += _matrix[2][3] * tx;
      _matrix[3][0] += _matrix[3][3] * tx;
    }

    if (ty != 0)
    {
      _matrix[0][1] += _matrix[0][3] * ty;
      _matrix[1][1] += _matrix[1][3] * ty;
      _matrix[2][1] += _matrix[2][3] * ty;
      _matrix[3][1] += _matrix[3][3] * ty;
    }

    return *this;
  }

  TransformationMatrix &TransformationMatrix::TranslateRight3d(double tx, double ty, double tz) noexcept
  {
    TranslateRight(tx, ty);
    if (tz != 0)
    {
      _matrix[0][2] += _matrix[0][3] * tz;
      _matrix[1][2] += _matrix[1][3] * tz;
      _matrix[2][2] += _matrix[2][3] * tz;
      _matrix[3][2] += _matrix[3][3] * tz;
    }

    return *this;
  }

  TransformationMatrix &TransformationMatrix::Skew(double sx, double sy) noexcept
  {
    // angles are in degrees. Switch to radians
    sx = deg2rad(sx);
    sy = deg2rad(sy);

    TransformationMatrix mat;
    mat._matrix[0][1] = std::tan(sy); // note that the y shear goes in the first row
    mat._matrix[1][0] = std::tan(sx); // and the x shear in the second row

    Multiply(mat);
    return *this;
  }

  TransformationMatrix &TransformationMatrix::ApplyPerspective(double p) noexcept
  {
    TransformationMatrix mat;
    if (p != 0)
    {
      mat._matrix[2][3] = -1 / p;
    }

    Multiply(mat);
    return *this;
  }

  TransformationMatrix TransformationMatrix::RectToRect(const FloatRect &from, const FloatRect &to) noexcept
  {
    krys_debug_assert(!from.IsEmpty());
    return TransformationMatrix(to.Width() / from.Width(), 0, 0, to.Height() / from.Height(),
                                to.X() - from.X(), to.Y() - from.Y());
  }

  TransformationMatrix &TransformationMatrix::Zoom(double zoomFactor) noexcept
  {
    _matrix[0][3] /= zoomFactor;
    _matrix[1][3] /= zoomFactor;
    _matrix[2][3] /= zoomFactor;
    _matrix[3][0] *= zoomFactor;
    _matrix[3][1] *= zoomFactor;
    _matrix[3][2] *= zoomFactor;
    return *this;
  }

  // this = mat * this.
  TransformationMatrix &TransformationMatrix::Multiply(const TransformationMatrix &mat) noexcept
  {
    Matrix4 tmp;

    for (int i = 0; i < 4; ++i)
    {
      for (int j = 0; j < 4; ++j)
      {
        tmp[i][j] = (mat._matrix[i][0] * _matrix[0][j] + mat._matrix[i][1] * _matrix[1][j]
                     + mat._matrix[i][2] * _matrix[2][j] + mat._matrix[i][3] * _matrix[3][j]);
      }
    }

    _matrix = tmp;
    return *this;
  }

  TransformationMatrix &TransformationMatrix::MultiplyAffineTransform(const AffineTransform &matrix) noexcept
  {
    if (matrix.IsIdentity())
    {
      return *this;
    }

    if (matrix.IsIdentityOrTranslation())
    {
      return Translate(matrix.E(), matrix.F());
    }

    return Multiply(matrix.ToTransformationMatrix());
  }

  void TransformationMatrix::MultVecMatrix(double x, double y, double &resultX,
                                           double &resultY) const noexcept
  {
    resultX = _matrix[3][0] + x * _matrix[0][0] + y * _matrix[1][0];
    resultY = _matrix[3][1] + x * _matrix[0][1] + y * _matrix[1][1];
    double w = _matrix[3][3] + x * _matrix[0][3] + y * _matrix[1][3];
    if (w != 1 && w != 0)
    {
      resultX /= w;
      resultY /= w;
    }
  }

  void TransformationMatrix::MultVecMatrix(double x, double y, double z, double &resultX, double &resultY,
                                           double &resultZ) const noexcept
  {
    resultX = _matrix[3][0] + x * _matrix[0][0] + y * _matrix[1][0] + z * _matrix[2][0];
    resultY = _matrix[3][1] + x * _matrix[0][1] + y * _matrix[1][1] + z * _matrix[2][1];
    resultZ = _matrix[3][2] + x * _matrix[0][2] + y * _matrix[1][2] + z * _matrix[2][2];
    double w = _matrix[3][3] + x * _matrix[0][3] + y * _matrix[1][3] + z * _matrix[2][3];
    if (w != 1 && w != 0)
    {
      resultX /= w;
      resultY /= w;
      resultZ /= w;
    }
  }

  bool TransformationMatrix::IsInvertible() const noexcept
  {
    auto type = GetType();
    if (type == Type::IdentityOrTranslation)
    {
      return true;
    }

    return std::isnormal(type == Type::Affine ? (M11() * M22() - M12() * M21()) : Determinant4x4(_matrix));
  }

  Maybe<TransformationMatrix> TransformationMatrix::Inverse() const noexcept
  {
    auto type = GetType();
    if (type == Type::IdentityOrTranslation)
    {
      // identity matrix
      if (_matrix[3][0] == 0 && _matrix[3][1] == 0 && _matrix[3][2] == 0)
      {
        return TransformationMatrix();
      }

      // translation
      return TransformationMatrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -_matrix[3][0], -_matrix[3][1],
                                  -_matrix[3][2], 1);
    }

    if (type == Type::Affine)
    {
      double a = M11();
      double b = M12();
      double c = M21();
      double d = M22();
      double e = M41();
      double f = M42();
      double determinant = a * d - b * c;
      if (!std::isnormal(determinant))
      {
        return {};
      }

      double inverseDeterminant = 1 / determinant;
      return TransformationMatrix {d * inverseDeterminant,
                                   -b * inverseDeterminant,
                                   -c * inverseDeterminant,
                                   a * inverseDeterminant,
                                   (c * f - d * e) * inverseDeterminant,
                                   (b * e - a * f) * inverseDeterminant};
    }

    TransformationMatrix invMat;
    // FIXME: Use LU decomposition to apply the inverse instead of calculating the inverse explicitly.
    // Calculating the inverse of a 4x4 matrix using cofactors is numerically unstable and unnecessary to
    // apply the inverse transformation to a point.
    if (!boo::Inverse(_matrix, invMat._matrix))
    {
      return {};
    }

    return invMat;
  }

  void TransformationMatrix::MakeAffine() noexcept
  {
    _matrix[0][2] = 0;
    _matrix[0][3] = 0;

    _matrix[1][2] = 0;
    _matrix[1][3] = 0;

    _matrix[2][0] = 0;
    _matrix[2][1] = 0;
    _matrix[2][2] = 1;
    _matrix[2][3] = 0;

    _matrix[3][2] = 0;
    _matrix[3][3] = 1;
  }

  void TransformationMatrix::Flatten() noexcept
  {
    _matrix[0][2] = 0;

    _matrix[1][2] = 0;

    _matrix[2][0] = 0;
    _matrix[2][1] = 0;
    _matrix[2][2] = 1;
    _matrix[2][3] = 0;

    _matrix[3][2] = 0;
  }

  AffineTransform TransformationMatrix::ToAffineTransform() const noexcept
  {
    return AffineTransform(_matrix[0][0], _matrix[0][1], _matrix[1][0], _matrix[1][1], _matrix[3][0],
                           _matrix[3][1]);
  }

  void TransformationMatrix::Blend2(const TransformationMatrix &from, double progress,
                                    CompositeOperation compositeOperation) noexcept
  {
    Decomposed2Type fromDecomp;
    Decomposed2Type toDecomp;
    if (!from.Decompose2(fromDecomp) || !Decompose2(toDecomp))
    {
      if (progress < 0.5)
      {
        *this = from;
      }
      return;
    }

    // If x-axis of one is flipped, and y-axis of the other, convert to an unflipped rotation.
    if ((fromDecomp.ScaleX < 0 && toDecomp.ScaleY < 0) || (fromDecomp.ScaleY < 0 && toDecomp.ScaleX < 0))
    {
      fromDecomp.ScaleX = -fromDecomp.ScaleX;
      fromDecomp.ScaleY = -fromDecomp.ScaleY;
      fromDecomp.Angle += fromDecomp.Angle < 0 ? 180 : -180;
    }

    // Don't rotate the long way around.
    if (!fromDecomp.Angle)
    {
      fromDecomp.Angle = 360;
    }
    if (!toDecomp.Angle)
    {
      toDecomp.Angle = 360;
    }

    if (std::abs(fromDecomp.Angle - toDecomp.Angle) > 180)
    {
      if (fromDecomp.Angle > toDecomp.Angle)
      {
        fromDecomp.Angle -= 360;
      }
      else
      {
        toDecomp.Angle -= 360;
      }
    }

    // When compositeOperation is accumulate, if the decomposed function is a 1-based value (for affine matrix
    // these properties are m11, m22, scaleX and scaleY), use one based accumulation. Otherwise, use behavior
    // for add.
    auto operationForNonOneBasedValues =
      compositeOperation == CompositeOperation::Accumulate ? CompositeOperation::Add : compositeOperation;
    BlendFloat(fromDecomp.M11, toDecomp.M11, progress, compositeOperation);
    BlendFloat(fromDecomp.M12, toDecomp.M12, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.M21, toDecomp.M21, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.M22, toDecomp.M22, progress, compositeOperation);
    BlendFloat(fromDecomp.TranslateX, toDecomp.TranslateX, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.TranslateY, toDecomp.TranslateY, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.ScaleX, toDecomp.ScaleX, progress, compositeOperation);
    BlendFloat(fromDecomp.ScaleY, toDecomp.ScaleY, progress, compositeOperation);
    BlendFloat(fromDecomp.Angle, toDecomp.Angle, progress, operationForNonOneBasedValues);
    Recompose2(fromDecomp);
  }

  void TransformationMatrix::Blend4(const TransformationMatrix &from, double progress,
                                    CompositeOperation compositeOperation) noexcept
  {
    Decomposed4Type fromDecomp;
    Decomposed4Type toDecomp;
    if (!from.Decompose4(fromDecomp) || !Decompose4(toDecomp))
    {
      if (progress < 0.5)
      {
        *this = from;
      }
      return;
    }

    // When compositeOperation is accumulate, if the decomposed function is a 1-based value (for non-affine
    // matrix these properties are scaleX, scaleY, scaleZ, and perspectiveW), use one based accumulation.
    // Otherwise, use behavior for add.
    auto operationForNonOneBasedValues =
      compositeOperation == CompositeOperation::Accumulate ? CompositeOperation::Add : compositeOperation;

    BlendFloat(fromDecomp.ScaleX, toDecomp.ScaleX, progress, compositeOperation);
    BlendFloat(fromDecomp.ScaleY, toDecomp.ScaleY, progress, compositeOperation);
    BlendFloat(fromDecomp.ScaleZ, toDecomp.ScaleZ, progress, compositeOperation);
    BlendFloat(fromDecomp.SkewXY, toDecomp.SkewXY, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.SkewXZ, toDecomp.SkewXZ, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.SkewYZ, toDecomp.SkewYZ, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.TranslateX, toDecomp.TranslateX, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.TranslateY, toDecomp.TranslateY, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.TranslateZ, toDecomp.TranslateZ, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.PerspectiveX, toDecomp.PerspectiveX, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.PerspectiveY, toDecomp.PerspectiveY, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.PerspectiveZ, toDecomp.PerspectiveZ, progress, operationForNonOneBasedValues);
    BlendFloat(fromDecomp.PerspectiveW, toDecomp.PerspectiveW, progress, compositeOperation);
    fromDecomp.Quaternion =
      fromDecomp.Quaternion.Interpolate(toDecomp.Quaternion, progress, compositeOperation);

    Recompose4(fromDecomp);
  }

  void TransformationMatrix::Blend(const TransformationMatrix &from, double progress,
                                   CompositeOperation compositeOperation) noexcept
  {
    if (!progress && compositeOperation == CompositeOperation::Replace)
    {
      *this = from;
      return;
    }

    if (progress == 1 && compositeOperation == CompositeOperation::Replace)
    {
      return;
    }

    if (from.IsIdentity() && IsIdentity())
    {
      return;
    }

    if (from.IsAffine() && IsAffine())
    {
      Blend2(from, progress, compositeOperation);
    }
    else
    {
      Blend4(from, progress, compositeOperation);
    }
  }

  bool TransformationMatrix::Decompose2(Decomposed2Type &decomp) const noexcept
  {
    if (IsIdentity())
    {
      decomp = {};
      decomp.ScaleX = 1;
      decomp.ScaleY = 1;
      decomp.M11 = 1;
      decomp.M22 = 1;
      return true;
    }

    return boo::Decompose2(_matrix, decomp);
  }

  bool TransformationMatrix::Decompose4(Decomposed4Type &decomp) const noexcept
  {
    if (IsIdentity())
    {
      decomp = {};
      decomp.PerspectiveW = 1;
      decomp.ScaleX = 1;
      decomp.ScaleY = 1;
      decomp.ScaleZ = 1;
      return true;
    }

    return boo::Decompose4(_matrix, decomp);
  }

  void TransformationMatrix::Recompose2(const Decomposed2Type &decomp) noexcept
  {
    MakeIdentity();

    _matrix[0][0] = decomp.M11;
    _matrix[0][1] = decomp.M12;
    _matrix[1][0] = decomp.M21;
    _matrix[1][1] = decomp.M22;

    TranslateRight(decomp.TranslateX, decomp.TranslateY);
    Rotate(decomp.Angle);
    Scale3d(decomp.ScaleX, decomp.ScaleY, 1);
  }

  void TransformationMatrix::Recompose4(const Decomposed4Type &decomp) noexcept
  {
    MakeIdentity();

    // First apply perspective.
    _matrix[0][3] = decomp.PerspectiveX;
    _matrix[1][3] = decomp.PerspectiveY;
    _matrix[2][3] = decomp.PerspectiveZ;
    _matrix[3][3] = decomp.PerspectiveW;

    // Next, translate.
    Translate3d(decomp.TranslateX, decomp.TranslateY, decomp.TranslateZ);

    // Apply rotation.
    TransformationMatrix rotationMatrix = TransformationMatrix::FromQuaternion(decomp.Quaternion);

    Multiply(rotationMatrix);

    // Apply skew.
    if (decomp.SkewYZ)
    {
      TransformationMatrix tmp;
      tmp.SetM32(decomp.SkewYZ);
      Multiply(tmp);
    }

    if (decomp.SkewXZ)
    {
      TransformationMatrix tmp;
      tmp.SetM31(decomp.SkewXZ);
      Multiply(tmp);
    }

    if (decomp.SkewXY)
    {
      TransformationMatrix tmp;
      tmp.SetM21(decomp.SkewXY);
      Multiply(tmp);
    }

    // Finally, apply scale.
    Scale3d(decomp.ScaleX, decomp.ScaleY, decomp.ScaleZ);
  }

  bool TransformationMatrix::IsIntegerTranslation() const noexcept
  {
    if (!IsIdentityOrTranslation())
    {
      return false;
    }

    // Check for translate Z.
    if (_matrix[3][2])
    {
      return false;
    }

    // Check for non-integer translate X/Y.
    if (static_cast<int>(_matrix[3][0]) != _matrix[3][0] || static_cast<int>(_matrix[3][1]) != _matrix[3][1])
    {
      return false;
    }

    return true;
  }

  bool TransformationMatrix::ContainsOnlyFiniteValues() const noexcept
  {
    for (auto &column : _matrix)
    {
      for (double value : column)
      {
        if (!std::isfinite(value))
        {
          return false;
        }
      }
    }
    return true;
  }

  TransformationMatrix TransformationMatrix::To2dTransform() const noexcept
  {
    return TransformationMatrix(_matrix[0][0], _matrix[0][1], 0, _matrix[0][3], _matrix[1][0], _matrix[1][1],
                                0, _matrix[1][3], 0, 0, 1, 0, _matrix[3][0], _matrix[3][1], 0, _matrix[3][3]);
  }

  auto TransformationMatrix::ToColumnMajorFloatArray() const noexcept -> FloatMatrix4
  {
    return {{float(M11()), float(M12()), float(M13()), float(M14()), float(M21()), float(M22()), float(M23()),
             float(M24()), float(M31()), float(M32()), float(M33()), float(M34()), float(M41()), float(M42()),
             float(M43()), float(M44())}};
  }

  bool TransformationMatrix::IsBackFaceVisible() const noexcept
  {
    // Back-face visibility is determined by transforming the normal vector (0, 0, 1) and
    // checking the sign of the resulting z component. However, normals cannot be
    // transformed by the original matrix, they require being transformed by the
    // inverse-transpose.
    //
    // Since we know we will be using (0, 0, 1), and we only care about the z-component of
    // the transformed normal, then we only need the m33() element of the
    // inverse-transpose. Therefore we do not need the transpose.
    //
    // Additionally, if we only need the m33() element, we do not need to compute a full
    // inverse. Instead, knowing the inverse of a matrix is adjoint(matrix) / determinant,
    // we can simply compute the m33() of the adjoint (adjugate) matrix, without computing
    // the full adjoint.

    double determinant = Determinant4x4(_matrix);

    // If the matrix is not invertible, then we assume its backface is not visible.
    if (!std::isnormal(determinant))
    {
      return false;
    }

    double cofactor33 = Determinant3x3(M11(), M12(), M14(), M21(), M22(), M24(), M41(), M42(), M44());
    double zComponentOfTransformedNormal = cofactor33 / determinant;

    return zComponentOfTransformedNormal < 0;
  }

  TransformationMatrix TransformationMatrix::Transpose() const noexcept
  {
    TransformationMatrix transpose;
    TransposeMatrix4(_matrix, transpose._matrix);
    return transpose;
  }
}
