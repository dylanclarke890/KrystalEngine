#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;
  using Catch::Matchers::WithinULP;

  static void TestIdentity(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.M11(), WithinULP(1.0, 4));
    CHECK_THAT(transform.M12(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M13(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M14(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M21(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M22(), WithinULP(1.0, 4));
    CHECK_THAT(transform.M23(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M24(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M31(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M32(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M33(), WithinULP(1.0, 4));
    CHECK_THAT(transform.M34(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M41(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M42(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M43(), WithinULP(0.0, 4));
    CHECK_THAT(transform.M44(), WithinULP(1.0, 4));
  }

  static void TestGetAndSet(TransformationMatrix &transform)
  {
    transform.SetA(1.1);
    CHECK_THAT(transform.A(), WithinULP(1.1, 4));
    transform.SetB(2.2);
    CHECK_THAT(transform.B(), WithinULP(2.2, 4));
    transform.SetC(3.3);
    CHECK_THAT(transform.C(), WithinULP(3.3, 4));
    transform.SetD(4.4);
    CHECK_THAT(transform.D(), WithinULP(4.4, 4));
    transform.SetE(5.5);
    CHECK_THAT(transform.E(), WithinULP(5.5, 4));
    transform.SetF(6.6);
    CHECK_THAT(transform.F(), WithinULP(6.6, 4));

    transform.SetM11(1.1);
    CHECK_THAT(transform.M11(), WithinULP(1.1, 4));
    transform.SetM12(2.2);
    CHECK_THAT(transform.M12(), WithinULP(2.2, 4));
    transform.SetM13(3.3);
    CHECK_THAT(transform.M13(), WithinULP(3.3, 4));
    transform.SetM14(4.4);
    CHECK_THAT(transform.M14(), WithinULP(4.4, 4));
    transform.SetM21(5.5);
    CHECK_THAT(transform.M21(), WithinULP(5.5, 4));
    transform.SetM22(6.6);
    CHECK_THAT(transform.M22(), WithinULP(6.6, 4));
    transform.SetM23(7.7);
    CHECK_THAT(transform.M23(), WithinULP(7.7, 4));
    transform.SetM24(8.8);
    CHECK_THAT(transform.M24(), WithinULP(8.8, 4));
    transform.SetM31(9.9);
    CHECK_THAT(transform.M31(), WithinULP(9.9, 4));
    transform.SetM32(10.10);
    CHECK_THAT(transform.M32(), WithinULP(10.10, 4));
    transform.SetM33(11.11);
    CHECK_THAT(transform.M33(), WithinULP(11.11, 4));
    transform.SetM34(12.12);
    CHECK_THAT(transform.M34(), WithinULP(12.12, 4));
    transform.SetM41(13.13);
    CHECK_THAT(transform.M41(), WithinULP(13.13, 4));
    transform.SetM42(14.14);
    CHECK_THAT(transform.M42(), WithinULP(14.14, 4));
    transform.SetM43(15.15);
    CHECK_THAT(transform.M43(), WithinULP(15.15, 4));
    transform.SetM44(16.16);
    CHECK_THAT(transform.M44(), WithinULP(16.16, 4));
  }

  TEST_CASE("TransformationMatrix: DefaultConstruction", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test;

    TestIdentity(test);

    TestGetAndSet(test);

    REQUIRE_FALSE(test.IsIdentity());
  }

  static void TestAffineLikeConstruction(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(6.0, 4));
    CHECK_THAT(transform.B(), WithinULP(5.0, 4));
    CHECK_THAT(transform.C(), WithinULP(4.0, 4));
    CHECK_THAT(transform.D(), WithinULP(3.0, 4));
    CHECK_THAT(transform.E(), WithinULP(2.0, 4));
    CHECK_THAT(transform.F(), WithinULP(1.0, 4));
  }

  static void TestValueConstruction(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.M11(), WithinULP(16.0, 4));
    CHECK_THAT(transform.M12(), WithinULP(15.0, 4));
    CHECK_THAT(transform.M13(), WithinULP(14.0, 4));
    CHECK_THAT(transform.M14(), WithinULP(13.0, 4));
    CHECK_THAT(transform.M21(), WithinULP(12.0, 4));
    CHECK_THAT(transform.M22(), WithinULP(11.0, 4));
    CHECK_THAT(transform.M23(), WithinULP(10.0, 4));
    CHECK_THAT(transform.M24(), WithinULP(9.0, 4));
    CHECK_THAT(transform.M31(), WithinULP(8.0, 4));
    CHECK_THAT(transform.M32(), WithinULP(7.0, 4));
    CHECK_THAT(transform.M33(), WithinULP(6.0, 4));
    CHECK_THAT(transform.M34(), WithinULP(5.0, 4));
    CHECK_THAT(transform.M41(), WithinULP(4.0, 4));
    CHECK_THAT(transform.M42(), WithinULP(3.0, 4));
    CHECK_THAT(transform.M43(), WithinULP(2.0, 4));
    CHECK_THAT(transform.M44(), WithinULP(1.0, 4));
    REQUIRE_FALSE(transform.IsAffine());
  }

  TEST_CASE("TransformationMatrix: ValueConstruction", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test1(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestAffineLikeConstruction(test1);

    REQUIRE_FALSE(test1.IsIdentity());
    REQUIRE(test1.IsAffine());

    TransformationMatrix test2(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                               2.0, 1.0);

    TestValueConstruction(test2);

    TestGetAndSet(test2);

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsAffine());

    test2.MakeIdentity();

    TestIdentity(test2);

    REQUIRE(test2.IsIdentity());
  }

  TEST_CASE("TransformationMatrix: AffineTransformConstruction", "[Graphics][TransformationMatrix]")
  {
    AffineTransform affineTransform(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    TransformationMatrix test(affineTransform);

    TestAffineLikeConstruction(test);
    TestGetAndSet(test);

    REQUIRE_FALSE(test.IsIdentity());
  }

  TEST_CASE("TransformationMatrix: TransformMatrixConstruction", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix matrix(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                                2.0, 1.0);
    TransformationMatrix test(matrix);

    TestValueConstruction(test);
  }

  TEST_CASE("TransformationMatrix: Assignment", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    TransformationMatrix matrix(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                                2.0, 1.0);

    TestAffineLikeConstruction(test);

    test = matrix;

    TestValueConstruction(test);
  }

  TEST_CASE("TransformationMatrix: Identity", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    REQUIRE_FALSE(test.IsIdentity());
    REQUIRE_FALSE(test.IsIdentityOrTranslation());

    test.MakeIdentity();

    REQUIRE(test.IsIdentity());
    REQUIRE(test.IsIdentityOrTranslation());

    TestIdentity(test);
  }

  static void TestAffineVersion(TransformationMatrix affineTransformMatrix)
  {
    REQUIRE_FALSE(affineTransformMatrix.IsIdentity());
    REQUIRE_FALSE(affineTransformMatrix.IsIdentityOrTranslation());
    REQUIRE(affineTransformMatrix.IsAffine());

    CHECK_THAT(affineTransformMatrix.M11(), WithinULP(16.0, 4));
    CHECK_THAT(affineTransformMatrix.M12(), WithinULP(15.0, 4));
    CHECK_THAT(affineTransformMatrix.M13(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M14(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M21(), WithinULP(12.0, 4));
    CHECK_THAT(affineTransformMatrix.M22(), WithinULP(11.0, 4));
    CHECK_THAT(affineTransformMatrix.M23(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M24(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M31(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M32(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M33(), WithinULP(1.0, 4));
    CHECK_THAT(affineTransformMatrix.M34(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M41(), WithinULP(4.0, 4));
    CHECK_THAT(affineTransformMatrix.M42(), WithinULP(3.0, 4));
    CHECK_THAT(affineTransformMatrix.M43(), WithinULP(0.0, 4));
    CHECK_THAT(affineTransformMatrix.M44(), WithinULP(1.0, 4));
  }

  TEST_CASE("TransformationMatrix: AffineVersion", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                              2.0, 1.0);

    REQUIRE_FALSE(test.IsIdentity());
    REQUIRE_FALSE(test.IsIdentityOrTranslation());
    REQUIRE_FALSE(test.IsAffine());

    auto affineCopy = test.ToAffineTransform();

    TestAffineVersion(affineCopy);

    test.MakeAffine();

    TestAffineVersion(test);
  }

  TEST_CASE("TransformationMatrix: MapFloatPoint", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test;
    FloatPoint point(100.0f, 50.0f);

    auto mappedPoint = test.MapPoint(point);

    REQUIRE_THAT(mappedPoint.X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedPoint.Y(), WithinULP(50.0f, 4));

    test.SetD(2.0);

    auto mappedPoint2 = test.MapPoint(point);

    REQUIRE_THAT(mappedPoint2.X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedPoint2.Y(), WithinULP(100.0f, 4));

    test.SetA(0.5);

    auto mappedPoint3 = test.MapPoint(point);

    REQUIRE_THAT(mappedPoint3.X(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedPoint3.Y(), WithinULP(100.0f, 4));

    test.SetM22(0.5);

    auto mappedPoint4 = test.MapPoint(point);

    REQUIRE_THAT(mappedPoint4.X(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedPoint4.Y(), WithinULP(25.0f, 4));

    test.SetM11(2.0);

    auto mappedPoint5 = test.MapPoint(point);

    REQUIRE_THAT(mappedPoint5.X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedPoint5.Y(), WithinULP(25.0f, 4));
  }

  TEST_CASE("TransformationMatrix: MapIntPoint", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test;
    IntPoint point(100, 50);

    auto mappedPoint = test.MapPoint(point);

    REQUIRE(100 == mappedPoint.X());
    REQUIRE(50 == mappedPoint.Y());

    test.SetD(2.0);

    auto mappedPoint2 = test.MapPoint(point);

    REQUIRE(100 == mappedPoint2.X());
    REQUIRE(100 == mappedPoint2.Y());

    test.SetA(0.5);

    auto mappedPoint3 = test.MapPoint(point);

    REQUIRE(50 == mappedPoint3.X());
    REQUIRE(100 == mappedPoint3.Y());

    test.SetM22(0.5);

    auto mappedPoint4 = test.MapPoint(point);

    REQUIRE(50 == mappedPoint4.X());
    REQUIRE(25 == mappedPoint4.Y());

    test.SetM11(2.0);

    auto mappedPoint5 = test.MapPoint(point);

    REQUIRE(200 == mappedPoint5.X());
    REQUIRE(25 == mappedPoint5.Y());
  }

  TEST_CASE("TransformationMatrix: MapIntRect", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test;
    IntRect rect(10, 20, 200, 300);

    auto mappedRect = test.MapRect(rect);

    REQUIRE(10 == mappedRect.X());
    REQUIRE(20 == mappedRect.Y());
    REQUIRE(200 == mappedRect.Width());
    REQUIRE(300 == mappedRect.Height());

    test.SetD(2.0);

    auto mappedRect2 = test.MapRect(rect);

    REQUIRE(10 == mappedRect2.X());
    REQUIRE(40 == mappedRect2.Y());
    REQUIRE(200 == mappedRect2.Width());
    REQUIRE(600 == mappedRect2.Height());

    test.SetA(0.5);

    auto mappedRect3 = test.MapRect(rect);

    REQUIRE(5 == mappedRect3.X());
    REQUIRE(40 == mappedRect3.Y());
    REQUIRE(100 == mappedRect3.Width());
    REQUIRE(600 == mappedRect3.Height());

    test.SetM22(0.5);

    auto mappedRect4 = test.MapRect(rect);

    REQUIRE(5 == mappedRect4.X());
    REQUIRE(10 == mappedRect4.Y());
    REQUIRE(100 == mappedRect4.Width());
    REQUIRE(150 == mappedRect4.Height());

    test.SetM11(2.0);

    auto mappedRect5 = test.MapRect(rect);

    REQUIRE(20 == mappedRect5.X());
    REQUIRE(10 == mappedRect5.Y());
    REQUIRE(100 == mappedRect4.Width());
    REQUIRE(150 == mappedRect4.Height());
  }

  TEST_CASE("TransformationMatrix: MapFloatRect", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test;
    FloatRect rect(10.f, 20.0f, 200.0f, 300.0f);

    auto mappedRect = test.MapRect(rect);

    REQUIRE_THAT(mappedRect.X(), WithinULP(10.0f, 4));
    REQUIRE_THAT(mappedRect.Y(), WithinULP(20.0f, 4));
    REQUIRE_THAT(mappedRect.Width(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedRect.Height(), WithinULP(300.0f, 4));

    test.SetD(2.0);

    auto mappedRect2 = test.MapRect(rect);

    REQUIRE_THAT(mappedRect2.X(), WithinULP(10.0f, 4));
    REQUIRE_THAT(mappedRect2.Y(), WithinULP(40.0f, 4));
    REQUIRE_THAT(mappedRect2.Width(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedRect2.Height(), WithinULP(600.0f, 4));

    test.SetA(0.5);

    auto mappedRect3 = test.MapRect(rect);

    REQUIRE_THAT(mappedRect3.X(), WithinULP(5.0f, 4));
    REQUIRE_THAT(mappedRect3.Y(), WithinULP(40.0f, 4));
    REQUIRE_THAT(mappedRect3.Width(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedRect3.Height(), WithinULP(600.0f, 4));

    test.SetM22(0.5);

    auto mappedRect4 = test.MapRect(rect);

    REQUIRE_THAT(mappedRect4.X(), WithinULP(5.0f, 4));
    REQUIRE_THAT(mappedRect4.Y(), WithinULP(10.0f, 4));
    REQUIRE_THAT(mappedRect4.Width(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedRect4.Height(), WithinULP(150.0f, 4));

    test.SetM11(2.0);

    auto mappedRect5 = test.MapRect(rect);

    REQUIRE_THAT(mappedRect5.X(), WithinULP(20.0f, 4));
    REQUIRE_THAT(mappedRect5.Y(), WithinULP(10.0f, 4));
    REQUIRE_THAT(mappedRect4.Width(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedRect4.Height(), WithinULP(150.0f, 4));
  }

  TEST_CASE("TransformationMatrix: MapFloatQuad", "[Graphics][TransformationMatrix]")
  {
    FloatRect rect(100.0f, 100.0f, 100.0f, 50.0f);
    FloatQuad quad(rect);

    REQUIRE_THAT(quad.P1().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(quad.P1().Y(), WithinULP(100.0f, 4));
    REQUIRE_THAT(quad.P2().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(quad.P2().Y(), WithinULP(100.0f, 4));
    REQUIRE_THAT(quad.P3().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(quad.P3().Y(), WithinULP(150.0f, 4));
    REQUIRE_THAT(quad.P4().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(quad.P4().Y(), WithinULP(150.0f, 4));

    TransformationMatrix test;
    auto mappedQuad = test.MapQuad(quad);

    REQUIRE_THAT(mappedQuad.P1().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad.P1().Y(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad.P2().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad.P2().Y(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad.P3().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad.P3().Y(), WithinULP(150.0f, 4));
    REQUIRE_THAT(mappedQuad.P4().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad.P4().Y(), WithinULP(150.0f, 4));

    test.SetD(2.0);

    auto mappedQuad2 = test.MapQuad(quad);

    REQUIRE_THAT(mappedQuad2.P1().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad2.P1().Y(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad2.P2().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad2.P2().Y(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad2.P3().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad2.P3().Y(), WithinULP(300.0f, 4));
    REQUIRE_THAT(mappedQuad2.P4().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad2.P4().Y(), WithinULP(300.0f, 4));

    test.SetA(0.5);

    auto mappedQuad3 = test.MapQuad(quad);

    REQUIRE_THAT(mappedQuad3.P1().X(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad3.P1().Y(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad3.P2().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad3.P2().Y(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad3.P3().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad3.P3().Y(), WithinULP(300.0f, 4));
    REQUIRE_THAT(mappedQuad3.P4().X(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad3.P4().Y(), WithinULP(300.0f, 4));

    test.SetM22(0.5);

    auto mappedQuad4 = test.MapQuad(quad);

    REQUIRE_THAT(mappedQuad4.P1().X(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad4.P1().Y(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad4.P2().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad4.P2().Y(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad4.P3().X(), WithinULP(100.0f, 4));
    REQUIRE_THAT(mappedQuad4.P3().Y(), WithinULP(75.0f, 4));
    REQUIRE_THAT(mappedQuad4.P4().X(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad4.P4().Y(), WithinULP(75.0f, 4));

    test.SetM11(2.0);

    auto mappedQuad5 = test.MapQuad(quad);

    REQUIRE_THAT(mappedQuad5.P1().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad5.P1().Y(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad5.P2().X(), WithinULP(400.0f, 4));
    REQUIRE_THAT(mappedQuad5.P2().Y(), WithinULP(50.0f, 4));
    REQUIRE_THAT(mappedQuad5.P3().X(), WithinULP(400.0f, 4));
    REQUIRE_THAT(mappedQuad5.P3().Y(), WithinULP(75.0f, 4));
    REQUIRE_THAT(mappedQuad5.P4().X(), WithinULP(200.0f, 4));
    REQUIRE_THAT(mappedQuad5.P4().Y(), WithinULP(75.0f, 4));
  }

  static void TestDoubled(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(12.0, 4));
    CHECK_THAT(transform.B(), WithinULP(10.0, 4));
    CHECK_THAT(transform.C(), WithinULP(8.0, 4));
    CHECK_THAT(transform.D(), WithinULP(6.0, 4));
    CHECK_THAT(transform.E(), WithinULP(2.0, 4));
    CHECK_THAT(transform.F(), WithinULP(1.0, 4));
  }

  static void TestHalved(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(3.0, 4));
    CHECK_THAT(transform.B(), WithinULP(2.5, 4));
    CHECK_THAT(transform.C(), WithinULP(2.0, 4));
    CHECK_THAT(transform.D(), WithinULP(1.5, 4));
    CHECK_THAT(transform.E(), WithinULP(2.0, 4));
    CHECK_THAT(transform.F(), WithinULP(1.0, 4));
  }

  static void TestDoubled2(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.M11(), WithinULP(32.0, 4));
    CHECK_THAT(transform.M12(), WithinULP(30.0, 4));
    CHECK_THAT(transform.M13(), WithinULP(28.0, 4));
    CHECK_THAT(transform.M14(), WithinULP(26.0, 4));
    CHECK_THAT(transform.M21(), WithinULP(24.0, 4));
    CHECK_THAT(transform.M22(), WithinULP(22.0, 4));
    CHECK_THAT(transform.M23(), WithinULP(20.0, 4));
    CHECK_THAT(transform.M24(), WithinULP(18.0, 4));
    CHECK_THAT(transform.M31(), WithinULP(16.0, 4));
    CHECK_THAT(transform.M32(), WithinULP(14.0, 4));
    CHECK_THAT(transform.M33(), WithinULP(12.0, 4));
    CHECK_THAT(transform.M34(), WithinULP(10.0, 4));
    CHECK_THAT(transform.M41(), WithinULP(8.0, 4));
    CHECK_THAT(transform.M42(), WithinULP(6.0, 4));
    CHECK_THAT(transform.M43(), WithinULP(4.0, 4));
    CHECK_THAT(transform.M44(), WithinULP(2.0, 4));
  }

  static void TestHalved2(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.M11(), WithinULP(8.0, 4));
    CHECK_THAT(transform.M12(), WithinULP(7.5, 4));
    CHECK_THAT(transform.M13(), WithinULP(7.0, 4));
    CHECK_THAT(transform.M14(), WithinULP(6.5, 4));
    CHECK_THAT(transform.M21(), WithinULP(6.0, 4));
    CHECK_THAT(transform.M22(), WithinULP(5.5, 4));
    CHECK_THAT(transform.M23(), WithinULP(5.0, 4));
    CHECK_THAT(transform.M24(), WithinULP(4.5, 4));
    CHECK_THAT(transform.M31(), WithinULP(4.0, 4));
    CHECK_THAT(transform.M32(), WithinULP(3.5, 4));
    CHECK_THAT(transform.M33(), WithinULP(3.0, 4));
    CHECK_THAT(transform.M34(), WithinULP(2.5, 4));
    CHECK_THAT(transform.M41(), WithinULP(2.0, 4));
    CHECK_THAT(transform.M42(), WithinULP(1.5, 4));
    CHECK_THAT(transform.M43(), WithinULP(1.0, 4));
    CHECK_THAT(transform.M44(), WithinULP(0.5, 4));
  }

  TEST_CASE("TransformationMatrix: Multiply", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    TransformationMatrix identity;

    TestAffineLikeConstruction(test);

    test.Multiply(identity);

    TestAffineLikeConstruction(test);

    TransformationMatrix doubler(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);

    test.Multiply(doubler);

    TestDoubled(test);

    TransformationMatrix halver(0.5, 0.0, 0.0, 0.5, 0.0, 0.0);

    test.Multiply(halver);

    TestAffineLikeConstruction(test);

    test.Multiply(halver);

    TestHalved(test);

    TransformationMatrix test2(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                               2.0, 1.0);

    TestValueConstruction(test2);

    test2.Multiply(identity);

    TestValueConstruction(test2);

    TransformationMatrix doubler2(2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0,
                                  2.0);

    test2.Multiply(doubler2);

    TestDoubled2(test2);

    TransformationMatrix halver2(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0,
                                 0.5);

    test2.Multiply(halver2);

    TestValueConstruction(test2);

    test2.Multiply(halver2);

    TestHalved2(test2);

    TransformationMatrix test3(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    test3 *= identity;

    TestAffineLikeConstruction(test3);

    test3 *= doubler;

    TestDoubled(test3);

    const TransformationMatrix test4(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0,
                                     3.0, 2.0, 1.0);

    auto result1 = test4 * identity;

    TestValueConstruction(result1);

    auto result2 = test4 * doubler2;

    TestDoubled2(result2);
  }

  static void TestScaledByTwo(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.M11(), WithinULP(32.0, 4));
    CHECK_THAT(transform.M12(), WithinULP(30.0, 4));
    CHECK_THAT(transform.M13(), WithinULP(28.0, 4));
    CHECK_THAT(transform.M14(), WithinULP(26.0, 4));
    CHECK_THAT(transform.M21(), WithinULP(24.0, 4));
    CHECK_THAT(transform.M22(), WithinULP(22.0, 4));
    CHECK_THAT(transform.M23(), WithinULP(20.0, 4));
    CHECK_THAT(transform.M24(), WithinULP(18.0, 4));
    CHECK_THAT(transform.M31(), WithinULP(8.0, 4));
    CHECK_THAT(transform.M32(), WithinULP(7.0, 4));
    CHECK_THAT(transform.M33(), WithinULP(6.0, 4));
    CHECK_THAT(transform.M34(), WithinULP(5.0, 4));
    CHECK_THAT(transform.M41(), WithinULP(4.0, 4));
    CHECK_THAT(transform.M42(), WithinULP(3.0, 4));
    CHECK_THAT(transform.M43(), WithinULP(2.0, 4));
    CHECK_THAT(transform.M44(), WithinULP(1.0, 4));
  }

  static void TestScaledByHalf(const TransformationMatrix &transform)
  {
    CHECK_THAT(transform.M11(), WithinULP(8.0, 4));
    CHECK_THAT(transform.M12(), WithinULP(7.5, 4));
    CHECK_THAT(transform.M13(), WithinULP(7.0, 4));
    CHECK_THAT(transform.M14(), WithinULP(6.5, 4));
    CHECK_THAT(transform.M21(), WithinULP(6.0, 4));
    CHECK_THAT(transform.M22(), WithinULP(5.5, 4));
    CHECK_THAT(transform.M23(), WithinULP(5.0, 4));
    CHECK_THAT(transform.M24(), WithinULP(4.5, 4));
    CHECK_THAT(transform.M31(), WithinULP(8.0, 4));
    CHECK_THAT(transform.M32(), WithinULP(7.0, 4));
    CHECK_THAT(transform.M33(), WithinULP(6.0, 4));
    CHECK_THAT(transform.M34(), WithinULP(5.0, 4));
    CHECK_THAT(transform.M41(), WithinULP(4.0, 4));
    CHECK_THAT(transform.M42(), WithinULP(3.0, 4));
    CHECK_THAT(transform.M43(), WithinULP(2.0, 4));
    CHECK_THAT(transform.M44(), WithinULP(1.0, 4));
  }

  TEST_CASE("TransformationMatrix: Scale", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestAffineLikeConstruction(test);

    test.Scale(1.0);

    TestAffineLikeConstruction(test);

    test.Scale(2.0);

    TestDoubled(test);

    test.Scale(0.5);

    TestAffineLikeConstruction(test);

    test.Scale(0.5);

    TestHalved(test);

    TransformationMatrix test2(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                               2.0, 1.0);

    TestValueConstruction(test2);

    test2.Scale(2.0);

    TestScaledByTwo(test2);

    test2.Scale(0.5);

    TestValueConstruction(test2);

    test2.Scale(0.5);

    TestScaledByHalf(test2);
  }

  TEST_CASE("TransformationMatrix: ScaleUniformNonUniform", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                              2.0, 1.0);

    TestValueConstruction(test);

    test.ScaleNonUniform(1.0, 1.0);

    TestValueConstruction(test);

    test.ScaleNonUniform(2.0, 2.0);

    TestScaledByTwo(test);

    test.ScaleNonUniform(0.5, 0.5);

    TestValueConstruction(test);

    test.ScaleNonUniform(0.5, 0.5);

    TestScaledByHalf(test);
  }

  TEST_CASE("TransformationMatrix: Rotate", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    test.Rotate(360.0);

    TestAffineLikeConstruction(test);

    test.Rotate(180.0);

    static double epsilon = 0.0001;

    CHECK_THAT(test.A(), WithinAbs(-6.0, epsilon));
    CHECK_THAT(test.B(), WithinAbs(-5.0, epsilon));
    CHECK_THAT(test.C(), WithinAbs(-4.0, epsilon));
    CHECK_THAT(test.D(), WithinAbs(-3.0, epsilon));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Rotate(-180.0);

    TestAffineLikeConstruction(test);

    test.Rotate(90.0);

    CHECK_THAT(test.A(), WithinAbs(4.0, epsilon));
    CHECK_THAT(test.B(), WithinAbs(3.0, epsilon));
    CHECK_THAT(test.C(), WithinAbs(-6.0, epsilon));
    CHECK_THAT(test.D(), WithinAbs(-5.0, epsilon));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Rotate(-90.0);

    TestAffineLikeConstruction(test);
  }

  TEST_CASE("TransformationMatrix: TranslateXY", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    test.Translate(0.0, 0.0);

    TestAffineLikeConstruction(test);

    test.Translate(5.0, 0.0);

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(32.0, 4));
    CHECK_THAT(test.F(), WithinULP(26.0, 4));

    test.Translate(0.0, -1.2);

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(27.2, 4));
    CHECK_THAT(test.F(), WithinULP(22.4, 4));
  }

  TEST_CASE("TransformationMatrix: FlipX", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestAffineLikeConstruction(test);

    test.FlipX();

    CHECK_THAT(test.A(), WithinULP(-6.0, 4));
    CHECK_THAT(test.B(), WithinULP(-5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipX();

    TestAffineLikeConstruction(test);

    TransformationMatrix test2;

    TestIdentity(test2);

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());

    test2.FlipX();

    CHECK_THAT(test2.A(), WithinULP(-1.0, 4));
    CHECK_THAT(test2.B(), WithinULP(0.0, 4));
    CHECK_THAT(test2.C(), WithinULP(0.0, 4));
    CHECK_THAT(test2.D(), WithinULP(1.0, 4));
    CHECK_THAT(test2.E(), WithinULP(0.0, 4));
    CHECK_THAT(test2.F(), WithinULP(0.0, 4));

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());

    test2.FlipX();

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
  }

  TEST_CASE("TransformationMatrix: FlipY", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestAffineLikeConstruction(test);

    test.FlipY();

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(-4.0, 4));
    CHECK_THAT(test.D(), WithinULP(-3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipY();

    TestAffineLikeConstruction(test);

    TransformationMatrix test2;

    TestIdentity(test2);

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());

    test2.FlipY();

    CHECK_THAT(test2.A(), WithinULP(1.0, 4));
    CHECK_THAT(test2.B(), WithinULP(0.0, 4));
    CHECK_THAT(test2.C(), WithinULP(0.0, 4));
    CHECK_THAT(test2.D(), WithinULP(-1.0, 4));
    CHECK_THAT(test2.E(), WithinULP(0.0, 4));
    CHECK_THAT(test2.F(), WithinULP(0.0, 4));

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());

    test2.FlipY();

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
  }

  TEST_CASE("TransformationMatrix: FlipXandFlipY", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestAffineLikeConstruction(test);

    test.FlipX();

    CHECK_THAT(test.A(), WithinULP(-6.0, 4));
    CHECK_THAT(test.B(), WithinULP(-5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipY();

    CHECK_THAT(test.A(), WithinULP(-6.0, 4));
    CHECK_THAT(test.B(), WithinULP(-5.0, 4));
    CHECK_THAT(test.C(), WithinULP(-4.0, 4));
    CHECK_THAT(test.D(), WithinULP(-3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipX();

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(-4.0, 4));
    CHECK_THAT(test.D(), WithinULP(-3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipY();

    TestAffineLikeConstruction(test);

    TransformationMatrix test2;

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());

    test2.FlipX();

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());

    test2.FlipY();

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());

    test2.FlipX();

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());

    test2.FlipY();

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
  }

  TEST_CASE("TransformationMatrix: Skew", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestAffineLikeConstruction(test);

    test.Skew(360.0, 360.0);

    TestAffineLikeConstruction(test);

    test.Skew(0.0, 0.0);

    TestAffineLikeConstruction(test);

    test.Skew(180.0, 180.0);

    static double epsilon = 0.0001;

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinAbs(4.0, epsilon));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Skew(-180.0, -180.0);

    TestAffineLikeConstruction(test);
  }

  TEST_CASE("TransformationMatrix: Inverse", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test;

    auto inverse = test.Inverse();

    REQUIRE(inverse);

    CHECK_THAT(inverse->A(), WithinULP(1.0, 4));
    CHECK_THAT(inverse->B(), WithinULP(0.0, 4));
    CHECK_THAT(inverse->C(), WithinULP(0.0, 4));
    CHECK_THAT(inverse->D(), WithinULP(1.0, 4));
    CHECK_THAT(inverse->E(), WithinULP(0.0, 4));
    CHECK_THAT(inverse->F(), WithinULP(0.0, 4));

    auto test2 = test * inverse.value();

    TestIdentity(test2);
  }

  TEST_CASE("TransformationMatrix: NonInvertableBlend", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix from;
    TransformationMatrix to(2.7133590938, 0.0, 0.0, 0.0, 0.0, 2.4645137761, 0.0, 0.0, 0.0, 0.0, 0.00, 0.01,
                            0.02, 0.03, 0.04, 0.05);
    TransformationMatrix result;

    result = to;
    result.Blend(from, 0.25);
    CHECK(result == from);

    result = to;
    result.Blend(from, 0.75);
    CHECK(result == to);
  }

  TEST_CASE("TransformationMatrix: Blend", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix transform;

    TransformationMatrix scaled;
    scaled.Scale(2.0);

    transform.Blend(scaled, 50);

    static const double epsilon = 0.0001;
    CHECK_THAT(transform.M11(), WithinAbs(-48.0, epsilon));
    CHECK_THAT(transform.M12(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M13(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M14(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M21(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M22(), WithinAbs(-48.0, epsilon));
    CHECK_THAT(transform.M23(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M24(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M31(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M32(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M33(), WithinAbs(1.0, epsilon));
    CHECK_THAT(transform.M34(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M41(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M42(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M43(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M44(), WithinAbs(1.0, epsilon));
  }

  TEST_CASE("TransformationMatrix: Blend2", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix transform;

    TransformationMatrix scaled;
    scaled.Scale(2.0);

    transform.Blend2(scaled, 20);

    static const double epsilon = 0.0001;
    CHECK_THAT(transform.M11(), WithinAbs(-18.0, epsilon));
    CHECK_THAT(transform.M12(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M13(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M14(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M21(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M22(), WithinAbs(-18.0, epsilon));
    CHECK_THAT(transform.M23(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M24(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M31(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M32(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M33(), WithinAbs(1.0, epsilon));
    CHECK_THAT(transform.M34(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M41(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M42(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M43(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M44(), WithinAbs(1.0, epsilon));
  }

  TEST_CASE("TransformationMatrix: Blend4", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix transform;

    TransformationMatrix scaled;
    scaled.Scale(2.0);

    transform.Blend4(scaled, 30);

    static const double epsilon = 0.0001;
    CHECK_THAT(transform.M11(), WithinAbs(-28.0, epsilon));
    CHECK_THAT(transform.M12(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M13(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M14(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M21(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M22(), WithinAbs(-28.0, epsilon));
    CHECK_THAT(transform.M23(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M24(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M31(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M32(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M33(), WithinAbs(1.0, epsilon));
    CHECK_THAT(transform.M34(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M41(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M42(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M43(), WithinAbs(0.0, epsilon));
    CHECK_THAT(transform.M44(), WithinAbs(1.0, epsilon));
  }

  TEST_CASE("TransformationMatrix: Equality", "[Graphics][TransformationMatrix]")
  {
    TransformationMatrix test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    TransformationMatrix test2;

    REQUIRE_FALSE(test == test2);
    REQUIRE(test != test2);

    test.MakeIdentity();

    REQUIRE(test == test2);
    REQUIRE_FALSE(test != test2);

    TransformationMatrix test3(16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0,
                               2.0, 1.0);
    TransformationMatrix test4(test3);

    REQUIRE(test3 == test4);
    REQUIRE_FALSE(test3 != test4);

    test4.SetM43(1002.22);

    REQUIRE_FALSE(test3 == test4);
    REQUIRE(test3 != test4);
  }

  TEST_CASE("TransformationMatrix: MakeMapBetweenRects", "[Graphics][TransformationMatrix]")
  {
    FloatRect fromRect(10.0f, 10.0f, 100.0f, 100.0f);
    FloatRect toRect(70.0f, 70.0f, 200.0f, 50.0f);

    auto mapBetween = TransformationMatrix::RectToRect(fromRect, toRect);

    CHECK_THAT(mapBetween.A(), WithinULP(2.0, 4));
    CHECK_THAT(mapBetween.B(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.C(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.D(), WithinULP(0.5, 4));
    CHECK_THAT(mapBetween.E(), WithinULP(60.0, 4));
    CHECK_THAT(mapBetween.F(), WithinULP(60.0, 4));

    CHECK_THAT(mapBetween.M11(), WithinULP(2.0, 4));
    CHECK_THAT(mapBetween.M12(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M13(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M14(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M21(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M22(), WithinULP(0.5, 4));
    CHECK_THAT(mapBetween.M23(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M24(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M31(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M32(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M33(), WithinULP(1.0, 4));
    CHECK_THAT(mapBetween.M34(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M41(), WithinULP(60.0, 4));
    CHECK_THAT(mapBetween.M42(), WithinULP(60.0, 4));
    CHECK_THAT(mapBetween.M43(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.M44(), WithinULP(1.0, 4));
  }
}
