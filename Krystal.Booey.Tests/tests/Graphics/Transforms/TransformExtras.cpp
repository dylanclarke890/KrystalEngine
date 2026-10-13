#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;

  namespace
  {
    void CheckNear(const TransformationMatrix &a, const TransformationMatrix &b, double epsilon = 1e-9)
    {
      auto x = a.ToColumnMajorFloatArray();
      auto y = b.ToColumnMajorFloatArray();
      for (size_t i = 0; i < x.size(); ++i)
      {
        CHECK_THAT(x[i], WithinAbs(y[i], epsilon));
      }
    }
  }

  TEST_CASE("FloatPoint: Transforms", "[Graphics][FloatPoint]")
  {
    FloatPoint test(100.0f, 200.0f);

    AffineTransform affine;
    CHECK(test.MatrixTransform(affine) == FloatPoint(100, 200));

    AffineTransform affine2(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);
    CHECK(test.MatrixTransform(affine2) == FloatPoint(200, 400));

    TransformationMatrix matrix;
    CHECK(test.MatrixTransform(matrix) == FloatPoint(100, 200));
    CHECK(test.MatrixTransform(affine2.ToTransformationMatrix()) == FloatPoint(200, 400));
  }

  TEST_CASE("AffineTransform: rest of the API", "[Graphics][AffineTransform]")
  {
    STATIC_CHECK(AffineTransform::Identity == AffineTransform());
    CHECK(AffineTransform::Identity.IsIdentity());

    // A quarter turn about (10, 10) maps (20, 10) to (10, 20).
    auto rotation = AffineTransform::MakeRotation(90, FloatPoint(10, 10));
    auto mapped = rotation.MapPoint(FloatPoint(20, 10));
    CHECK_THAT(mapped.X(), WithinAbs(10, 1e-5));
    CHECK_THAT(mapped.Y(), WithinAbs(20, 1e-5));
    CHECK(AffineTransform::MakeRotation(90, FloatPoint()) == AffineTransform::MakeRotation(90));
    CHECK(AffineTransform::MakeScale(FloatSize(2, 3)) == AffineTransform(2, 0, 0, 3, 0, 0));

    AffineTransform fromVector;
    fromVector.RotateFromVector(0, 1);
    CHECK(fromVector.IsEssentiallyEqualToAsFloats(AffineTransform::MakeRotation(90)));
    CHECK(fromVector.IsRotateOrShear());
    CHECK_FALSE(fromVector.IsIdentityOrTranslation());
    // cos(pi/2) is not exactly zero, so this is not axis-aligned; an exact quarter turn is.
    CHECK_FALSE(fromVector.PreservesAxisAlignment());
    CHECK(AffineTransform(0, 1, -1, 0, 0, 0).PreservesAxisAlignment());
    CHECK_FALSE(AffineTransform::MakeRotation(45).PreservesAxisAlignment());

    AffineTransform flipped;
    flipped.FlipY();
    CHECK(flipped.IsIdentityOrTranslationOrFlipped());
    CHECK_FALSE(flipped.IsIdentityOrTranslation());

    AffineTransform skewed;
    skewed.SkewX(45);
    CHECK(skewed.MapPoint(FloatPoint(0, 10)) == FloatPoint(10, 10));
    AffineTransform skewedY;
    skewedY.SkewY(45);
    CHECK(skewedY.MapPoint(FloatPoint(10, 0)) == FloatPoint(10, 10));

    CHECK(AffineTransform(1, 2, 3, 4, 5, 6).AsSpan()[4] == 5);
    CHECK(AffineTransform(Span<const double, 6>(AffineTransform(1, 2, 3, 4, 5, 6).AsSpan()))
          == AffineTransform(1, 2, 3, 4, 5, 6));

    AffineTransform set;
    set.SetMatrix(1, 2, 3, 4, 5, 6);
    CHECK(set == AffineTransform(1, 2, 3, 4, 5, 6));
    set *= AffineTransform::MakeTranslation(FloatSize(1, 1));
    CHECK(set == AffineTransform(1, 2, 3, 4, 9, 12));

    // A singular transform has no inverse.
    CHECK_FALSE(AffineTransform(1, 2, 2, 4, 0, 0).IsInvertible());
    CHECK_FALSE(AffineTransform(1, 2, 2, 4, 0, 0).Inverse());
    // A translation's inverse is the opposite translation.
    CHECK(*AffineTransform::MakeTranslation(FloatSize(3, -4)).Inverse()
          == AffineTransform::MakeTranslation(FloatSize(-3, 4)));
  }

  TEST_CASE("AffineTransform: decomposition", "[Graphics][AffineTransform]")
  {
    AffineTransform transform;
    transform.Translate(10, 20);
    transform.Rotate(30);
    transform.Scale(2, 3);

    AffineTransform::DecomposedType decomposed;
    REQUIRE(transform.Decompose(decomposed));
    CHECK_THAT(decomposed.ScaleX, WithinAbs(2, 1e-9));
    CHECK_THAT(decomposed.ScaleY, WithinAbs(3, 1e-9));
    CHECK_THAT(decomposed.Angle, WithinAbs(deg2rad(30.0), 1e-9));

    AffineTransform recomposed;
    recomposed.Recompose(decomposed);
    CHECK(recomposed.IsEssentiallyEqualToAsFloats(transform));

    // Halfway between the identity and a scale by 3 is a scale by 2.
    AffineTransform blended;
    blended.Scale(3);
    blended.Blend(AffineTransform(), 0.5);
    CHECK(blended.IsEssentiallyEqualToAsFloats(AffineTransform::MakeScale(FloatSize(2, 2))));
  }

  TEST_CASE("AffineTransform: mapping regions and sizes", "[Graphics][AffineTransform]")
  {
    Region region(IntRect(0, 0, 10, 10));
    region.Unite(IntRect(10, 10, 10, 10));

    auto translated = AffineTransform::MakeTranslation(FloatSize(5, 5)).MapRegion(region);
    CHECK(translated == Translate(region, IntSize(5, 5)));

    auto scaled = AffineTransform::MakeScale(FloatSize(2, 2)).MapRegion(region);
    CHECK(scaled.Rects() == SmallList<IntRect, 1> {IntRect(0, 0, 20, 20), IntRect(20, 20, 20, 20)});

    auto rotation = AffineTransform::MakeRotation(90);
    CHECK(rotation.MapSize(IntSize(3, 4)) == IntSize(3, 4));
    CHECK(AffineTransform::MakeScale(FloatSize(2, 0.5f)).MapSize(FloatSize(3, 4)) == FloatSize(6, 2));
    CHECK(rotation.MapRect(IntRect(0, 0, 10, 20)) == IntRect(-20, 0, 20, 10));
  }

  TEST_CASE("TransformationMatrix: 3D rotation and projection", "[Graphics][TransformationMatrix]")
  {
    // A rotation about z matches the 2D rotation.
    TransformationMatrix about;
    about.Rotate3d(0, 0, 1, 90);
    TransformationMatrix rotate;
    rotate.Rotate(90);
    CHECK(about == rotate);

    // Zero axes do nothing; non-unit axes are normalised.
    TransformationMatrix none;
    none.Rotate3d(0, 0, 0, 45);
    CHECK(none.IsIdentity());
    TransformationMatrix scaledAxis;
    scaledAxis.Rotate3d(0, 0, 5, 90);
    CHECK(scaledAxis == rotate);

    // Euler angles: rotating about x by 180 degrees flips y and z.
    TransformationMatrix euler;
    euler.Rotate3d(180, 0, 0);
    CHECK(euler.MapPoint(FloatPoint3D(1, 2, 3)) == FloatPoint3D(1, -2, -3));
    TransformationMatrix arbitrary;
    arbitrary.Rotate3d(1, 1, 0, 180);
    auto swapped = arbitrary.MapPoint(FloatPoint3D(1, 0, 0));
    CHECK_THAT(swapped.X(), WithinAbs(0, 1e-6));
    CHECK_THAT(swapped.Y(), WithinAbs(1, 1e-6));
    CHECK_THAT(swapped.Z(), WithinAbs(0, 1e-6));

    // A quarter turn about y shows the back face; a half turn about y does too, and none doesn't.
    TransformationMatrix halfTurn;
    halfTurn.Rotate3d(0, 1, 0, 180);
    CHECK(halfTurn.IsBackFaceVisible());
    CHECK_FALSE(TransformationMatrix().IsBackFaceVisible());

    // Perspective: a point pushed back along z is projected towards the origin.
    TransformationMatrix perspective;
    perspective.ApplyPerspective(100);
    CHECK(perspective.HasPerspective());
    CHECK_FALSE(TransformationMatrix().HasPerspective());
    perspective.Translate3d(0, 0, -100);
    auto projected = perspective.MapPoint(FloatPoint3D(50, 50, 0));
    CHECK_THAT(projected.X(), WithinAbs(25, 1e-4));
    CHECK_THAT(projected.Y(), WithinAbs(25, 1e-4));

    double x = 50, y = 50, z = 0, w = 1;
    perspective.Map4ComponentPoint(x, y, z, w);
    CHECK_THAT(x / w, WithinAbs(25, 1e-9));

    // ProjectPoint inverts the mapping onto the z=0 plane.
    TransformationMatrix tilted;
    tilted.Rotate3d(1, 0, 0, 60);
    auto back = tilted.ProjectPoint(FloatPoint(10, 5));
    CHECK_THAT(back.X(), WithinAbs(10, 1e-4));
    CHECK_THAT(back.Y(), WithinAbs(10, 1e-4));

    // Rotating the plane edge-on leaves no well-defined projection.
    TransformationMatrix edgeOn;
    edgeOn.Rotate3d(1, 0, 0, 90);
    CHECK(edgeOn.ProjectPoint(FloatPoint(1, 1)) == FloatPoint());

    bool clamped = true;
    auto quad = tilted.ProjectQuad(FloatQuad(FloatRect(0, 0, 10, 5)), &clamped);
    CHECK_FALSE(clamped);
    CHECK_THAT(quad.P3().Y(), WithinAbs(10, 1e-4));
    CHECK(tilted.ClampedBoundsOfProjectedQuad(FloatQuad(FloatRect(0, 0, 10, 5))) == LayoutRect(0, 0, 10, 10));
  }

  TEST_CASE("TransformationMatrix: decomposition", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix matrix;
    matrix.Translate3d(1, 2, 3);
    matrix.Rotate3d(1, 2, 3, 40);
    matrix.Scale3d(2, 3, 4);

    TransformationMatrix::Decomposed4Type decomposed;
    REQUIRE(matrix.Decompose4(decomposed));
    CHECK_THAT(decomposed.TranslateX, WithinAbs(1, 1e-9));
    CHECK_THAT(decomposed.ScaleZ, WithinAbs(4, 1e-9));
    CHECK_THAT(decomposed.PerspectiveW, WithinAbs(1, 1e-9));

    TransformationMatrix recomposed;
    recomposed.Recompose4(decomposed);
    CheckNear(recomposed, matrix, 1e-5);

    // The rotation round-trips through a quaternion.
    TransformationMatrix rotation;
    rotation.Rotate3d(1, 2, 3, 40);
    TransformationMatrix::Decomposed4Type rotationOnly;
    REQUIRE(rotation.Decompose4(rotationOnly));
    CheckNear(TransformationMatrix::FromQuaternion(rotationOnly.Quaternion), rotation, 1e-6);

    TransformationMatrix::Decomposed2Type identity2;
    REQUIRE(TransformationMatrix().Decompose2(identity2));
    CHECK(identity2.ScaleX == 1);
    CHECK(identity2.Angle == 0);

    // A singular matrix can't be decomposed.
    TransformationMatrix singular;
    singular.Scale3d(1, 1, 0);
    CHECK_FALSE(singular.Decompose4(decomposed));
    CHECK_FALSE(singular.IsInvertible());
    CHECK_FALSE(singular.Inverse());

    // Inverse of a general 3D matrix.
    auto inverse = matrix.Inverse();
    REQUIRE(inverse);
    CheckNear(matrix * *inverse, TransformationMatrix(), 1e-6);
  }

  TEST_CASE("TransformationMatrix: helpers", "[Graphics][TransformationMatrix]")
  {
    STATIC_CHECK(TransformationMatrix::Identity.IsIdentity());

    TransformationMatrix full(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    CHECK(full.Transpose() == TransformationMatrix(1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16));
    CHECK(full.To2dTransform() == TransformationMatrix(1, 2, 0, 4, 5, 6, 0, 8, 0, 0, 1, 0, 13, 14, 0, 16));
    CHECK(full.ToColumnMajorFloatArray()[5] == 6);
    CHECK_FALSE(full.IsAffine());

    TransformationMatrix flattened = full;
    flattened.Flatten();
    CHECK(flattened == TransformationMatrix(1, 2, 0, 4, 5, 6, 0, 8, 0, 0, 1, 0, 13, 14, 0, 16));
    TransformationMatrix affine = full;
    affine.MakeAffine();
    CHECK(affine.IsAffine());
    CHECK(affine.ToAffineTransform() == AffineTransform(1, 2, 5, 6, 13, 14));

    CHECK(TransformationMatrix(3, 4).IsIntegerTranslation());
    CHECK_FALSE(TransformationMatrix(3.5, 4).IsIntegerTranslation());
    CHECK_FALSE(TransformationMatrix().Translate3d(0, 0, 1).IsIntegerTranslation());
    CHECK(full.ContainsOnlyFiniteValues());
    CHECK_FALSE(TransformationMatrix(Limits<double>::infinity(), 0).ContainsOnlyFiniteValues());

    // Zoom keeps the linear part and scales translation and perspective.
    TransformationMatrix zoomed(1, 0, 0, 0.5, 0, 0, 1, 0, 0, 0, 1, 0, 10, 20, 30, 1);
    zoomed.Zoom(2);
    CHECK(zoomed == TransformationMatrix(1, 0, 0, 0.25, 0, 0, 1, 0, 0, 0, 1, 0, 20, 40, 60, 1));

    // TranslateRight post-multiplies.
    TransformationMatrix right;
    right.Scale(2);
    right.TranslateRight3d(1, 2, 3);
    CHECK(right.MapPoint(FloatPoint3D(1, 1, 1)) == FloatPoint3D(3, 4, 4));

    // Multiplying by an affine transform.
    TransformationMatrix withAffine;
    withAffine.MultiplyAffineTransform(AffineTransform::MakeTranslation(FloatSize(5, 6)));
    CHECK(withAffine == TransformationMatrix(5, 6));
    withAffine.MultiplyAffineTransform(AffineTransform::MakeScale(FloatSize(2, 2)));
    CHECK(withAffine == TransformationMatrix(2, 0, 0, 2, 5, 6));
    withAffine.MultiplyAffineTransform(AffineTransform());
    CHECK(withAffine == TransformationMatrix(2, 0, 0, 2, 5, 6));

    // Rotations by whole turns do nothing; snapping rounds tiny sines and cosines to zero.
    TransformationMatrix turn;
    turn.Rotate(720);
    CHECK(turn.IsIdentity());
    TransformationMatrix snapped;
    snapped.Rotate(90);
    CHECK(snapped.M11() == 0);
    TransformationMatrix unsnapped;
    unsnapped.Rotate(90, TransformationMatrix::RotationSnapping::None);
    CHECK(unsnapped.M11() != 0);
    CHECK_THAT(unsnapped.M11(), WithinAbs(0, 1e-15));

    // Mapping layout rects goes through enclosing.
    CHECK(TransformationMatrix().Scale(2).MapRect(LayoutRect(LayoutUnit(0.5f), 0, 1, 1))
          == LayoutRect(1, 0, 2, 2));

    auto projection = TransformationMatrix::FromProjection(std::numbers::pi / 2, 1, 1, 10);
    CHECK_THAT(projection.M11(), WithinAbs(1, 1e-6));
    CHECK_THAT(projection.M34(), WithinAbs(-1, 1e-9));
    auto symmetric = TransformationMatrix::FromProjection(std::numbers::pi / 4, std::numbers::pi / 4,
                                                          std::numbers::pi / 4, std::numbers::pi / 4, 1, 10);
    CheckNear(symmetric, projection, 1e-6);
  }
}
