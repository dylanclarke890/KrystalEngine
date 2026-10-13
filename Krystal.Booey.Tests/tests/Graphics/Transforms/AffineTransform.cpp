#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;
  using Catch::Matchers::WithinULP;

  static void TestGetAndSet(AffineTransform &affineTransform)
  {
    affineTransform.SetA(1.1);
    CHECK_THAT(affineTransform.A(), WithinULP(1.1, 4));
    affineTransform.SetB(2.2);
    CHECK_THAT(affineTransform.B(), WithinULP(2.2, 4));
    affineTransform.SetC(3.3);
    CHECK_THAT(affineTransform.C(), WithinULP(3.3, 4));
    affineTransform.SetD(4.4);
    CHECK_THAT(affineTransform.D(), WithinULP(4.4, 4));
    affineTransform.SetE(5.5);
    CHECK_THAT(affineTransform.E(), WithinULP(5.5, 4));
    affineTransform.SetF(6.6);
    CHECK_THAT(affineTransform.F(), WithinULP(6.6, 4));
  }

  static void TestIdentity(const AffineTransform &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(1.0, 4));
    CHECK_THAT(transform.B(), WithinULP(0.0, 4));
    CHECK_THAT(transform.C(), WithinULP(0.0, 4));
    CHECK_THAT(transform.D(), WithinULP(1.0, 4));
    CHECK_THAT(transform.E(), WithinULP(0.0, 4));
    CHECK_THAT(transform.F(), WithinULP(0.0, 4));
  }

  TEST_CASE("AffineTransform: DefaultConstruction", "[Graphics][AffineTransform]")
  {
    AffineTransform test;

    TestIdentity(test);
    TestGetAndSet(test);

    REQUIRE_FALSE(test.IsIdentity());
  }

  static void TestValueConstruction(const AffineTransform &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(6.0, 4));
    CHECK_THAT(transform.B(), WithinULP(5.0, 4));
    CHECK_THAT(transform.C(), WithinULP(4.0, 4));
    CHECK_THAT(transform.D(), WithinULP(3.0, 4));
    CHECK_THAT(transform.E(), WithinULP(2.0, 4));
    CHECK_THAT(transform.F(), WithinULP(1.0, 4));
  }

  static void TestDoubled(const AffineTransform &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(12.0, 4));
    CHECK_THAT(transform.B(), WithinULP(10.0, 4));
    CHECK_THAT(transform.C(), WithinULP(8.0, 4));
    CHECK_THAT(transform.D(), WithinULP(6.0, 4));
    CHECK_THAT(transform.E(), WithinULP(2.0, 4));
    CHECK_THAT(transform.F(), WithinULP(1.0, 4));
  }

  static void TestHalved(const AffineTransform &transform)
  {
    CHECK_THAT(transform.A(), WithinULP(3.0, 4));
    CHECK_THAT(transform.B(), WithinULP(2.5, 4));
    CHECK_THAT(transform.C(), WithinULP(2.0, 4));
    CHECK_THAT(transform.D(), WithinULP(1.5, 4));
    CHECK_THAT(transform.E(), WithinULP(2.0, 4));
    CHECK_THAT(transform.F(), WithinULP(1.0, 4));
  }

  TEST_CASE("AffineTransform: ValueConstruction", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);
    TestGetAndSet(test);

    REQUIRE_FALSE(test.IsIdentity());
    REQUIRE_FALSE(test.PreservesAxisAlignment());
  }

  TEST_CASE("AffineTransform: Identity", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    REQUIRE_FALSE(test.IsIdentity());
    REQUIRE_FALSE(test.IsIdentityOrTranslation());
    REQUIRE_FALSE(test.IsIdentityOrTranslationOrFlipped());
    REQUIRE_FALSE(test.PreservesAxisAlignment());

    test.MakeIdentity();

    REQUIRE(test.IsIdentity());
    REQUIRE(test.IsIdentityOrTranslation());
    REQUIRE(test.IsIdentityOrTranslationOrFlipped());
    REQUIRE(test.PreservesAxisAlignment());

    TestIdentity(test);
  }

  TEST_CASE("AffineTransform: MapFloatPoint", "[Graphics][AffineTransform]")
  {
    AffineTransform test;
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
  }

  TEST_CASE("AffineTransform: MapIntPoint", "[Graphics][AffineTransform]")
  {
    AffineTransform test;
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
  }

  TEST_CASE("AffineTransform: MapIntSize", "[Graphics][AffineTransform]")
  {
    AffineTransform test;
    IntSize size(200, 300);

    auto mappedSize = test.MapSize(size);

    REQUIRE(200 == mappedSize.Width());
    REQUIRE(300 == mappedSize.Height());

    test.SetD(2.0);

    auto mappedSize2 = test.MapSize(size);

    REQUIRE(200 == mappedSize2.Width());
    REQUIRE(600 == mappedSize2.Height());

    test.SetA(0.5);

    auto mappedSize3 = test.MapSize(size);

    REQUIRE(100 == mappedSize3.Width());
    REQUIRE(600 == mappedSize3.Height());
  }

  TEST_CASE("AffineTransform: MapFloatSize", "[Graphics][AffineTransform]")
  {
    AffineTransform test;
    FloatSize size(200.0f, 300.0f);

    auto mappedSize = test.MapSize(size);

    REQUIRE(200.0f == mappedSize.Width());
    REQUIRE(300.0f == mappedSize.Height());

    test.SetD(2.0);

    auto mappedSize2 = test.MapSize(size);

    REQUIRE(200.0f == mappedSize2.Width());
    REQUIRE(600.0f == mappedSize2.Height());

    test.SetA(0.5);

    auto mappedSize3 = test.MapSize(size);

    REQUIRE(100.0f == mappedSize3.Width());
    REQUIRE(600.0f == mappedSize3.Height());
  }

  TEST_CASE("AffineTransform: MapIntRect", "[Graphics][AffineTransform]")
  {
    AffineTransform test;
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
  }

  TEST_CASE("AffineTransform: MapFloatRect", "[Graphics][AffineTransform]")
  {
    AffineTransform test;
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
  }

  TEST_CASE("AffineTransform: MapFloatQuad", "[Graphics][AffineTransform]")
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

    AffineTransform test;
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
  }

  TEST_CASE("AffineTransform: Multiply", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    AffineTransform identity;

    TestValueConstruction(test);

    test.Multiply(identity);

    TestValueConstruction(test);

    AffineTransform doubler(2.0, 0.0, 0.0, 2.0, 0.0, 0.0);

    test.Multiply(doubler);

    TestDoubled(test);

    AffineTransform halver(0.5, 0.0, 0.0, 0.5, 0.0, 0.0);

    test.Multiply(halver);

    TestValueConstruction(test);

    test.Multiply(halver);

    TestHalved(test);
  }

  TEST_CASE("AffineTransform: Scale", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    test.Scale(1.0);

    TestValueConstruction(test);

    test.Scale(2.0);

    TestDoubled(test);

    test.Scale(0.5);

    TestValueConstruction(test);

    test.Scale(0.5);

    TestHalved(test);
  }

  TEST_CASE("AffineTransform: ScaleUniformNonUniform", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    test.ScaleNonUniform(1.0, 1.0);

    TestValueConstruction(test);

    test.ScaleNonUniform(2.0, 2.0);

    TestDoubled(test);

    test.ScaleNonUniform(0.5, 0.5);

    TestValueConstruction(test);

    test.ScaleNonUniform(0.5, 0.5);

    TestHalved(test);
  }

  TEST_CASE("AffineTransform: ScaleNonUniform", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    test.ScaleNonUniform(1.0, 2.0);

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(8.0, 4));
    CHECK_THAT(test.D(), WithinULP(6.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.ScaleNonUniform(1.0, 0.5);

    TestValueConstruction(test);

    test.ScaleNonUniform(2.0, 1.0);

    CHECK_THAT(test.A(), WithinULP(12.0, 4));
    CHECK_THAT(test.B(), WithinULP(10.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.ScaleNonUniform(0.5, 1.0);

    TestValueConstruction(test);

    test.ScaleNonUniform(0.5, 2.0);

    CHECK_THAT(test.A(), WithinULP(3.0, 4));
    CHECK_THAT(test.B(), WithinULP(2.5, 4));
    CHECK_THAT(test.C(), WithinULP(8.0, 4));
    CHECK_THAT(test.D(), WithinULP(6.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));
  }

  TEST_CASE("AffineTransform: ScaleFloatSize", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    FloatSize first(1.0f, 2.0f);

    test.Scale(first);

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(8.0, 4));
    CHECK_THAT(test.D(), WithinULP(6.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    FloatSize second(1.0f, 0.5f);

    test.Scale(second);

    TestValueConstruction(test);

    FloatSize third(2.0f, 1.0f);

    test.Scale(third);

    CHECK_THAT(test.A(), WithinULP(12.0, 4));
    CHECK_THAT(test.B(), WithinULP(10.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    FloatSize fourth(0.5f, 1.0f);

    test.Scale(fourth);

    TestValueConstruction(test);

    FloatSize fifth(0.5f, 2.0f);

    test.Scale(fifth);

    CHECK_THAT(test.A(), WithinULP(3.0, 4));
    CHECK_THAT(test.B(), WithinULP(2.5, 4));
    CHECK_THAT(test.C(), WithinULP(8.0, 4));
    CHECK_THAT(test.D(), WithinULP(6.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));
  }

  TEST_CASE("AffineTransform: Rotate", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    test.Rotate(360.0);

    TestValueConstruction(test);

    test.Rotate(180.0);

    static double epsilon = 0.0001;

    CHECK_THAT(test.A(), WithinAbs(-6.0, epsilon));
    CHECK_THAT(test.B(), WithinAbs(-5.0, epsilon));
    CHECK_THAT(test.C(), WithinAbs(-4.0, epsilon));
    CHECK_THAT(test.D(), WithinAbs(-3.0, epsilon));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Rotate(-180.0);

    TestValueConstruction(test);

    test.Rotate(90.0);

    CHECK_THAT(test.A(), WithinAbs(4.0, epsilon));
    CHECK_THAT(test.B(), WithinAbs(3.0, epsilon));
    CHECK_THAT(test.C(), WithinAbs(-6.0, epsilon));
    CHECK_THAT(test.D(), WithinAbs(-5.0, epsilon));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Rotate(-90.0);

    TestValueConstruction(test);
  }

  TEST_CASE("AffineTransform: TranslateXY", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    test.Translate(0.0, 0.0);

    TestValueConstruction(test);

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

  TEST_CASE("AffineTransform: TranslateFloatPoint", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    FloatPoint none;
    test.Translate(none);

    TestValueConstruction(test);

    FloatPoint first(5.0f, 0.0f);

    test.Translate(first);

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(32.0, 4));
    CHECK_THAT(test.F(), WithinULP(26.0, 4));

    FloatPoint second(0.0f, -1.2f);

    test.Translate(second);

    static double epsilon = 0.0001;

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinAbs(27.2, epsilon));
    CHECK_THAT(test.F(), WithinAbs(22.4, epsilon));

    AffineTransform test2;

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());

    test2.Translate(second);

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());
  }

  TEST_CASE("AffineTransform: Shear", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    test.Shear(0.0, 0.0);

    TestValueConstruction(test);

    test.Shear(2.0, 2.0);

    CHECK_THAT(test.A(), WithinULP(14.0, 4));
    CHECK_THAT(test.B(), WithinULP(11.0, 4));
    CHECK_THAT(test.C(), WithinULP(16.0, 4));
    CHECK_THAT(test.D(), WithinULP(13.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Shear(-1.0, 2.0);

    CHECK_THAT(test.A(), WithinULP(46.0, 4));
    CHECK_THAT(test.B(), WithinULP(37.0, 4));
    CHECK_THAT(test.C(), WithinULP(2.0, 4));
    CHECK_THAT(test.D(), WithinULP(2.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));
  }

  TEST_CASE("AffineTransform: FlipX", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    test.FlipX();

    CHECK_THAT(test.A(), WithinULP(-6.0, 4));
    CHECK_THAT(test.B(), WithinULP(-5.0, 4));
    CHECK_THAT(test.C(), WithinULP(4.0, 4));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipX();

    TestValueConstruction(test);

    AffineTransform test2;

    TestIdentity(test2);

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipX();

    CHECK_THAT(test2.A(), WithinULP(-1.0, 4));
    CHECK_THAT(test2.B(), WithinULP(0.0, 4));
    CHECK_THAT(test2.C(), WithinULP(0.0, 4));
    CHECK_THAT(test2.D(), WithinULP(1.0, 4));
    CHECK_THAT(test2.E(), WithinULP(0.0, 4));
    CHECK_THAT(test2.F(), WithinULP(0.0, 4));

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());
    // 'Flipped' just means in the Y direction
    REQUIRE_FALSE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipX();

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());
  }

  TEST_CASE("AffineTransform: FlipY", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    test.FlipY();

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinULP(-4.0, 4));
    CHECK_THAT(test.D(), WithinULP(-3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.FlipY();

    TestValueConstruction(test);

    AffineTransform test2;

    TestIdentity(test2);

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipY();

    CHECK_THAT(test2.A(), WithinULP(1.0, 4));
    CHECK_THAT(test2.B(), WithinULP(0.0, 4));
    CHECK_THAT(test2.C(), WithinULP(0.0, 4));
    CHECK_THAT(test2.D(), WithinULP(-1.0, 4));
    CHECK_THAT(test2.E(), WithinULP(0.0, 4));
    CHECK_THAT(test2.F(), WithinULP(0.0, 4));

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipY();

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());
  }

  TEST_CASE("AffineTransform: FlipXandFlipY", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

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

    TestValueConstruction(test);

    AffineTransform test2;

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipX();

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());
    REQUIRE_FALSE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipY();

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());
    // False here because X is also flipped.
    REQUIRE_FALSE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipX();

    REQUIRE_FALSE(test2.IsIdentity());
    REQUIRE_FALSE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());

    test2.FlipY();

    REQUIRE(test2.IsIdentity());
    REQUIRE(test2.IsIdentityOrTranslation());
    REQUIRE(test2.IsIdentityOrTranslationOrFlipped());
  }

  TEST_CASE("AffineTransform: Skew", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    TestValueConstruction(test);

    test.Skew(360.0, 360.0);

    TestValueConstruction(test);

    test.Skew(0.0, 0.0);

    TestValueConstruction(test);

    test.Skew(180.0, 180.0);

    static double epsilon = 0.0001;

    CHECK_THAT(test.A(), WithinULP(6.0, 4));
    CHECK_THAT(test.B(), WithinULP(5.0, 4));
    CHECK_THAT(test.C(), WithinAbs(4.0, epsilon));
    CHECK_THAT(test.D(), WithinULP(3.0, 4));
    CHECK_THAT(test.E(), WithinULP(2.0, 4));
    CHECK_THAT(test.F(), WithinULP(1.0, 4));

    test.Skew(-180.0, -180.0);

    TestValueConstruction(test);
  }

  TEST_CASE("AffineTransform: XandYScale", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);

    CHECK_THAT(test.XScale(), WithinAbs(7.8102, 0.0001));
    CHECK_THAT(test.YScale(), WithinAbs(5.0, 0.0001));
  }

  TEST_CASE("AffineTransform: Equality", "[Graphics][AffineTransform]")
  {
    AffineTransform test(6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    AffineTransform test2;

    REQUIRE_FALSE(test == test2);
    REQUIRE(test != test2);

    test.MakeIdentity();

    REQUIRE(test == test2);
    REQUIRE_FALSE(test != test2);
  }

  TEST_CASE("AffineTransform: Inverse", "[Graphics][AffineTransform]")
  {
    AffineTransform test;

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

  TEST_CASE("AffineTransform: Blend", "[Graphics][AffineTransform]")
  {
    AffineTransform test;

    AffineTransform test2;
    test2.Scale(2.0);

    test.Blend(test2, 50);

    CHECK_THAT(test.A(), WithinULP(-48.0, 4));
    CHECK_THAT(test.B(), WithinULP(0.0, 4));
    CHECK_THAT(test.C(), WithinULP(0.0, 4));
    CHECK_THAT(test.D(), WithinULP(-48.0, 4));
    CHECK_THAT(test.E(), WithinULP(0.0, 4));
    CHECK_THAT(test.F(), WithinULP(0.0, 4));
  }

  TEST_CASE("AffineTransform: Translation", "[Graphics][AffineTransform]")
  {
    auto test = AffineTransform::MakeTranslation({-5.0f, -7.0f});
    CHECK_THAT(test.A(), WithinULP(1.0, 4));
    CHECK_THAT(test.B(), WithinULP(0.0, 4));
    CHECK_THAT(test.C(), WithinULP(0.0, 4));
    CHECK_THAT(test.D(), WithinULP(1.0, 4));
    CHECK_THAT(test.E(), WithinULP(-5.0, 4));
    CHECK_THAT(test.F(), WithinULP(-7.0, 4));
  }

  TEST_CASE("AffineTransform: ToTransformationMatrix", "[Graphics][AffineTransform]")
  {
    AffineTransform transform;
    TransformationMatrix matrix = transform.ToTransformationMatrix();

    CHECK_THAT(matrix.M11(), WithinULP(1.0, 4));
    CHECK_THAT(matrix.M12(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M13(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M14(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M21(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M22(), WithinULP(1.0, 4));
    CHECK_THAT(matrix.M23(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M24(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M31(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M32(), WithinULP(0.0, 4));
    CHECK_THAT(matrix.M33(), WithinULP(1.0, 4));
    CHECK_THAT(matrix.M34(), WithinULP(0.0, 4));
  }

  TEST_CASE("AffineTransform: MakeMapBetweenRects", "[Graphics][AffineTransform]")
  {
    FloatRect fromRect(10.0f, 10.0f, 100.0f, 100.0f);
    FloatRect toRect(70.0f, 70.0f, 200.0f, 50.0f);

    auto mapBetween = MakeMapBetweenRects(fromRect, toRect);

    CHECK_THAT(mapBetween.A(), WithinULP(2.0, 4));
    CHECK_THAT(mapBetween.B(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.C(), WithinULP(0.0, 4));
    CHECK_THAT(mapBetween.D(), WithinULP(0.5, 4));
    CHECK_THAT(mapBetween.E(), WithinULP(60.0, 4));
    CHECK_THAT(mapBetween.F(), WithinULP(60.0, 4));
  }

  TEST_CASE("AffineTransform: Constexpr", "[Graphics][AffineTransform]")
  {
    static constexpr AffineTransform transform;
    STATIC_CHECK(transform == transform);
    static constexpr AffineTransform transform2(1, 2, 3, 4, 5, 6);
    STATIC_CHECK(transform2 == transform2);
  }
}
