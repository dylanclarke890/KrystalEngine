#include "Krystal.Booey/CSS/DOMMatrix.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DOMPoint.hpp"
#include <cmath>

namespace krys::boo::tests
{
  using Catch::Approx;
  using css::DOMMatrix;
  using css::DOMMatrixReadOnly;

  TEST_CASE("DOMMatrix: made from nothing, lists and strings", "[CSS][DOMMatrix]")
  {
    // With nothing, the matrix is the 2D identity.
    auto identity = DOMMatrix::Create().ReleaseValue();
    CHECK(identity->IsIdentity());
    CHECK(identity->GetIs2D());

    // Six values are a 2D matrix, sixteen a 3D one; other lengths throw.
    auto affine = DOMMatrix::Create(Variant<dom::DOMString, List<double>>(List<double> {1, 2, 3, 4, 5, 6}))
                    .ReleaseValue();
    CHECK(affine->GetIs2D());
    CHECK(affine->A() == 1);
    CHECK(affine->D() == 4);
    CHECK(affine->F() == 6);
    CHECK(affine->M41() == 5);

    List<double> sixteen(16, 0.0);
    sixteen[0] = sixteen[5] = sixteen[10] = sixteen[15] = 1;
    sixteen[14] = 7;
    auto full = DOMMatrix::Create(Variant<dom::DOMString, List<double>>(sixteen)).ReleaseValue();
    CHECK_FALSE(full->GetIs2D());
    CHECK(full->M43() == 7);

    CHECK(
      DOMMatrix::Create(Variant<dom::DOMString, List<double>>(List<double> {1, 2, 3})).GetException().Code()
      == dom::ExceptionCode::TypeError);

    SECTION("strings are parsed as `transform`")
    {
      auto translated = DOMMatrix::Create(Variant<dom::DOMString, List<double>>(
                                            dom::DOMString(u8"translate(10px, 20px) scale(2)")))
                          .ReleaseValue();
      CHECK(translated->GetIs2D());
      CHECK(translated->A() == 2);
      CHECK(translated->E() == 10);
      CHECK(translated->F() == 20);

      // A 3D function makes a 3D matrix, even when it's affine.
      auto rotated =
        DOMMatrix::Create(Variant<dom::DOMString, List<double>>(dom::DOMString(u8"rotateX(0deg)")))
          .ReleaseValue();
      CHECK_FALSE(rotated->GetIs2D());

      // `none` and the empty string are the identity.
      CHECK(DOMMatrix::Create(Variant<dom::DOMString, List<double>>(dom::DOMString(u8"none")))
              .ReleaseValue()
              ->IsIdentity());
      CHECK(DOMMatrix::Create(Variant<dom::DOMString, List<double>>(dom::DOMString()))
              .ReleaseValue()
              ->IsIdentity());

      // Lengths that need a style or a box, and things that aren't transforms, are syntax errors.
      for (auto string : {u8"translate(1em)", u8"translate(50%)", u8"red", u8"scale(2) junk"})
      {
        CHECK(DOMMatrix::Create(Variant<dom::DOMString, List<double>>(dom::DOMString(string)))
                .GetException()
                .Code()
              == dom::ExceptionCode::SyntaxError);
      }
    }

    SECTION("SetMatrixValue replaces the matrix")
    {
      REQUIRE_FALSE(identity->SetMatrixValue(u8"matrix(1, 0, 0, 1, 3, 4)").HasException());
      CHECK(identity->E() == 3);
      CHECK(identity->F() == 4);
      CHECK(identity->SetMatrixValue(u8"nonsense").HasException());
      CHECK(identity->E() == 3);
    }
  }

  TEST_CASE("DOMMatrix: validating and fixing up dictionaries", "[CSS][DOMMatrix]")
  {
    // The letters fill in the matrix entries they alias.
    css::DOMMatrixInit init;
    init.A = 2;
    init.F = 5;
    REQUIRE_FALSE(DOMMatrixReadOnly::ValidateAndFixup(init).HasException());
    CHECK(init.M11 == 2.0);
    CHECK(init.M42 == 5.0);
    CHECK(init.M22 == 1.0);
    CHECK(init.Is2D == true);

    // Aliases that disagree throw; NaN agrees with NaN.
    css::DOMMatrix2DInit mismatched;
    mismatched.A = 1;
    mismatched.M11 = 2;
    CHECK(DOMMatrixReadOnly::ValidateAndFixup(mismatched).GetException().Code()
          == dom::ExceptionCode::TypeError);
    css::DOMMatrix2DInit nan;
    nan.B = std::nan("");
    nan.M12 = std::nan("");
    CHECK_FALSE(DOMMatrixReadOnly::ValidateAndFixup(nan).HasException());

    // A 2D dictionary can't have 3D entries.
    css::DOMMatrixInit flat;
    flat.Is2D = true;
    flat.M33 = 2;
    CHECK(DOMMatrixReadOnly::ValidateAndFixup(flat).GetException().Code() == dom::ExceptionCode::TypeError);

    // Without `is2D`, 3D entries make it 3D.
    css::DOMMatrixInit deep;
    deep.M34 = 1;
    REQUIRE_FALSE(DOMMatrixReadOnly::ValidateAndFixup(deep).HasException());
    CHECK(deep.Is2D == false);

    auto fromInit = DOMMatrix::FromMatrix(css::DOMMatrixInit {{.E = 7}}).ReleaseValue();
    CHECK(fromInit->GetIs2D());
    CHECK(fromInit->E() == 7);
  }

  TEST_CASE("DOMMatrix: operations", "[CSS][DOMMatrix]")
  {
    auto matrix = DOMMatrix::Create().ReleaseValue();

    SECTION("the read-only operations return new matrices")
    {
      auto translated = matrix->Translate(10, 20);
      CHECK(matrix->IsIdentity());
      CHECK(translated->E() == 10);
      CHECK(translated->F() == 20);
      CHECK(translated->GetIs2D());
      CHECK_FALSE(matrix->Translate(0, 0, 1)->GetIs2D());

      auto scaled = matrix->Scale(2);
      CHECK(scaled->A() == 2);
      CHECK(scaled->D() == 2);
      auto scaledAbout = matrix->Scale(2, 2, 1, 10, 10);
      CHECK(scaledAbout->E() == -10);
      CHECK_FALSE(matrix->Scale3d(2)->GetIs2D());

      // One angle rotates about z, in degrees.
      auto rotated = matrix->Rotate(90);
      CHECK(rotated->GetIs2D());
      CHECK(rotated->B() == Approx(1));
      CHECK(rotated->C() == Approx(-1));
      CHECK_FALSE(matrix->Rotate(10, 0, 0)->GetIs2D());

      CHECK(matrix->FlipX()->A() == -1);
      CHECK(matrix->FlipY()->D() == -1);
      CHECK(matrix->SkewX(45)->C() == Approx(1));
    }

    SECTION("multiplying and inverting")
    {
      matrix->TranslateSelf(5, 0);
      auto product = matrix->Multiply(css::DOMMatrixInit {{.A = 2, .D = 2}}).ReleaseValue();
      CHECK(product->A() == 2);
      CHECK(product->E() == 5);
      REQUIRE_FALSE(matrix->PreMultiplySelf(css::DOMMatrixInit {{.A = 2, .D = 2}}).HasException());
      CHECK(matrix->E() == 10);

      auto inverse = matrix->Inverse();
      CHECK(inverse->A() == Approx(0.5));
      CHECK(inverse->E() == Approx(-5));

      // A singular matrix inverts to NaNs, and is no longer 2D.
      auto singular =
        DOMMatrix::Create(Variant<dom::DOMString, List<double>>(List<double> {0, 0, 0, 0, 0, 0}))
          .ReleaseValue();
      singular->InvertSelf();
      CHECK(std::isnan(singular->A()));
      CHECK_FALSE(singular->GetIs2D());
      CHECK(singular->ToString().GetException().Code() == dom::ExceptionCode::InvalidStateError);
    }

    SECTION("setting 3D entries makes it 3D")
    {
      matrix->SetM33(1);
      CHECK(matrix->GetIs2D());
      matrix->SetM13(0.5);
      CHECK_FALSE(matrix->GetIs2D());
    }
  }

  TEST_CASE("DOMMatrix: points, arrays and strings", "[CSS][DOMMatrix]")
  {
    auto matrix = DOMMatrix::Create().ReleaseValue()->Translate(10, 20)->ScaleSelf(2);

    auto point = matrix->TransformPoint({.X = 1, .Y = 1});
    CHECK(point->X() == 12);
    CHECK(point->Y() == 22);
    CHECK(point->W() == 1);

    auto transformed =
      dom::DOMPoint::Create(1, 1)->MatrixTransform(css::DOMMatrixInit {{.E = 3}}).ReleaseValue();
    CHECK(transformed->X() == 4);
    CHECK(transformed->Y() == 1);

    auto array = matrix->ToFloat64Array();
    CHECK(array[0] == 2);
    CHECK(array[12] == 10);
    CHECK(array[15] == 1);
    auto roundTrip = DOMMatrix::FromFloat32Array(Span<const float>(matrix->ToFloat32Array())).ReleaseValue();
    CHECK_FALSE(roundTrip->GetIs2D());
    CHECK(roundTrip->M41() == 10);
    CHECK(DOMMatrix::FromFloat64Array(Span<const double>(array.data(), 6)).ReleaseValue()->GetIs2D());

    CHECK(matrix->ToString().Value() == u8"matrix(2, 0, 0, 2, 10, 20)");
    CHECK(DOMMatrix::Create().ReleaseValue()->Translate(0, 0, 1)->ToString().Value()
          == u8"matrix3d(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1)");
  }
}
