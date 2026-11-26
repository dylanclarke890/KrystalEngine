#include "Krystal.Web/Css/Geometry/DOMMatrix.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Css/Geometry/DOMPoint.hpp"
#include <cmath>

namespace krys::web::tests
{
  using Catch::Approx;

  TEST_CASE("DOMMatrix: made from nothing, lists and strings",
            "[CSS][Geometry][DOMMatrix][geometry-1:dom-dommatrixreadonly-dommatrixreadonly][geometry-1:"
            "create-a-2d-matrix]"
            "[geometry-1:create-a-3d-matrix][geometry-1:parse-a-string-into-an-abstract-matrix]"
            "[geometry-1:dom-dommatrix-setmatrixvalue]")
  {
    // With nothing, the matrix is the 2D identity.
    auto identity = DOMMatrix::Create().ReleaseValue();
    CHECK(identity->IsIdentity());
    CHECK(identity->GetIs2D());

    // Six values are a 2D matrix, sixteen a 3D one; other lengths throw.
    auto affine =
      DOMMatrix::Create(Variant<DOMString, List<double>>(List<double> {1, 2, 3, 4, 5, 6})).ReleaseValue();
    CHECK(affine->GetIs2D());
    CHECK(affine->A() == 1);
    CHECK(affine->D() == 4);
    CHECK(affine->F() == 6);
    CHECK(affine->M41() == 5);

    List<double> sixteen(16, 0.0);
    sixteen[0] = sixteen[5] = sixteen[10] = sixteen[15] = 1;
    sixteen[14] = 7;
    auto full = DOMMatrix::Create(Variant<DOMString, List<double>>(sixteen)).ReleaseValue();
    CHECK_FALSE(full->GetIs2D());
    CHECK(full->M43() == 7);

    CHECK(DOMMatrix::Create(Variant<DOMString, List<double>>(List<double> {1, 2, 3})).GetException().Code()
          == ExceptionCode::TypeError);

    SECTION("strings are parsed as `transform`")
    {
      auto translated =
        DOMMatrix::Create(Variant<DOMString, List<double>>(DOMString(u8"translate(10px, 20px) scale(2)")))
          .ReleaseValue();
      CHECK(translated->GetIs2D());
      CHECK(translated->A() == 2);
      CHECK(translated->E() == 10);
      CHECK(translated->F() == 20);

      // A 3D function makes a 3D matrix, even when it's affine.
      auto rotated = DOMMatrix::Create(Variant<DOMString, List<double>>(u8"rotateX(0deg)")).ReleaseValue();
      CHECK_FALSE(rotated->GetIs2D());

      // `none` and the empty string are the identity.
      CHECK(DOMMatrix::Create(Variant<DOMString, List<double>>(u8"none")).ReleaseValue()->IsIdentity());
      CHECK(DOMMatrix::Create(Variant<DOMString, List<double>>(DOMString())).ReleaseValue()->IsIdentity());

      // Lengths that need a style or a box, and things that aren't transforms, are syntax errors.
      for (auto string : {u8"translate(1em)", u8"translate(50%)", u8"red", u8"scale(2) junk"})
      {
        CHECK(DOMMatrix::Create(Variant<DOMString, List<double>>(string)).GetException().Code()
              == ExceptionCode::SyntaxError);
      }
    }

    SECTION("CSS-wide keywords aren't transforms")
    {
      for (auto string : {u8"inherit", u8"initial", u8"unset"})
      {
        CHECK(DOMMatrix::Create(Variant<DOMString, List<double>>(string)).GetException().Code()
              == ExceptionCode::SyntaxError);
      }
    }

    SECTION("SetMatrixValue replaces the matrix and returns it")
    {
      auto result = identity->SetMatrixValue(u8"matrix(1, 0, 0, 1, 3, 4)");
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value().get() == identity.get());
      CHECK(identity->E() == 3);
      CHECK(identity->F() == 4);
      CHECK(identity->GetIs2D());

      // A 3D transform makes it 3D, and the empty string makes it the 2D identity again.
      REQUIRE_FALSE(identity->SetMatrixValue(u8"translateZ(5px)").HasException());
      CHECK_FALSE(identity->GetIs2D());
      CHECK(identity->M43() == 5);
      REQUIRE_FALSE(identity->SetMatrixValue(u8"").HasException());
      CHECK(identity->IsIdentity());
      CHECK(identity->GetIs2D());

      // A failure throws and leaves it as it was.
      identity->TranslateSelf(3);
      CHECK(identity->SetMatrixValue(u8"nonsense").GetException().Code() == ExceptionCode::SyntaxError);
      CHECK(identity->E() == 3);
    }
  }

  TEST_CASE("DOMMatrix: validating and fixing up dictionaries",
            "[CSS][Geometry][DOMMatrix][geometry-1:matrix-validate-and-fixup-2d][geometry-1:matrix-validate-"
            "and-fixup]"
            "[geometry-1:create-a-dommatrixreadonly-from-the-dictionary][geometry-1:dom-dommatrix-frommatrix]"
            "[geometry-1:dom-dommatrixreadonly-frommatrix]")
  {
    // The letters fill in the matrix entries they alias.
    DOMMatrixInit init;
    init.A = 2;
    init.F = 5;
    REQUIRE_FALSE(DOMMatrixReadOnly::ValidateAndFixup(init).HasException());
    CHECK(init.M11 == 2.0);
    CHECK(init.M42 == 5.0);
    CHECK(init.M22 == 1.0);
    CHECK(init.Is2D == true);

    // Aliases that disagree throw; NaN agrees with NaN.
    DOMMatrix2DInit mismatched;
    mismatched.A = 1;
    mismatched.M11 = 2;
    CHECK(DOMMatrixReadOnly::ValidateAndFixup(mismatched).GetException().Code() == ExceptionCode::TypeError);
    DOMMatrix2DInit nan;
    nan.B = std::nan("");
    nan.M12 = std::nan("");
    CHECK_FALSE(DOMMatrixReadOnly::ValidateAndFixup(nan).HasException());

    // A 2D dictionary can't have 3D entries.
    DOMMatrixInit flat;
    flat.Is2D = true;
    flat.M33 = 2;
    CHECK(DOMMatrixReadOnly::ValidateAndFixup(flat).GetException().Code() == ExceptionCode::TypeError);

    // Without `is2D`, 3D entries make it 3D.
    DOMMatrixInit deep;
    deep.M34 = 1;
    REQUIRE_FALSE(DOMMatrixReadOnly::ValidateAndFixup(deep).HasException());
    CHECK(deep.Is2D == false);

    auto fromInit = DOMMatrix::FromMatrix(DOMMatrixInit {{.E = 7}}).ReleaseValue();
    CHECK(fromInit->GetIs2D());
    CHECK(fromInit->E() == 7);

    // A read-only one is made the same way, and 3D entries make it 3D.
    DOMMatrixInit threeD;
    threeD.M43 = 2;
    auto readOnly = DOMMatrixReadOnly::FromMatrix(threeD).ReleaseValue();
    CHECK_FALSE(readOnly->GetIs2D());
    CHECK(readOnly->M43() == 2);
    CHECK(DOMMatrixReadOnly::FromMatrix(flat).GetException().Code() == ExceptionCode::TypeError);
  }

  TEST_CASE(
    "DOMMatrix: operations",
    "[CSS][Geometry][DOMMatrix][geometry-1:immutable-transformation-methods]"
    "[geometry-1:mutable-transformation-methods][geometry-1:dommatrix-attributes][geometry-1:matrix-is-2d]"
    "[geometry-1:dom-dommatrixreadonly-translate][geometry-1:dom-dommatrixreadonly-scale]"
    "[geometry-1:dom-dommatrixreadonly-scalenonuniform][geometry-1:dom-dommatrixreadonly-scale3d]"
    "[geometry-1:dom-dommatrixreadonly-rotate][geometry-1:dom-dommatrixreadonly-rotatefromvector]"
    "[geometry-1:dom-dommatrixreadonly-rotateaxisangle][geometry-1:dom-dommatrixreadonly-skewx]"
    "[geometry-1:dom-dommatrixreadonly-skewy][geometry-1:dom-dommatrixreadonly-multiply]"
    "[geometry-1:dom-dommatrixreadonly-flipx][geometry-1:dom-dommatrixreadonly-flipy]"
    "[geometry-1:dom-dommatrixreadonly-inverse][geometry-1:dom-dommatrix-multiplyself]"
    "[geometry-1:dom-dommatrix-premultiplyself][geometry-1:dom-dommatrix-translateself]"
    "[geometry-1:dom-dommatrix-scaleself][geometry-1:dom-dommatrix-scale3dself]"
    "[geometry-1:dom-dommatrix-rotateself][geometry-1:dom-dommatrix-rotatefromvectorself]"
    "[geometry-1:dom-dommatrix-rotateaxisangleself][geometry-1:dom-dommatrix-skewxself]"
    "[geometry-1:dom-dommatrix-skewyself][geometry-1:dom-dommatrix-invertself]")
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
      CHECK(matrix->SkewY(45)->B() == Approx(1));

      auto nonUniform = matrix->ScaleNonUniform(2, 3);
      CHECK(nonUniform->A() == 2);
      CHECK(nonUniform->D() == 3);
      CHECK(nonUniform->GetIs2D());

      // The angle from (1, 0) to (0, 1) is 90 degrees; to (0, 0) it's 0.
      auto fromVector = matrix->RotateFromVector(0, 1);
      CHECK(fromVector->B() == Approx(1));
      CHECK(fromVector->C() == Approx(-1));
      CHECK(matrix->RotateFromVector(0, 0)->IsIdentity());

      // About the z axis it stays 2D; about another axis it doesn't.
      auto aboutZ = matrix->RotateAxisAngle(0, 0, 1, 90);
      CHECK(aboutZ->GetIs2D());
      CHECK(aboutZ->B() == Approx(1));
      auto aboutX = matrix->RotateAxisAngle(1, 0, 0, 90);
      CHECK_FALSE(aboutX->GetIs2D());
      CHECK(aboutX->M23() == Approx(1));
    }

    SECTION("multiplying and inverting")
    {
      matrix->TranslateSelf(5, 0);
      auto product = matrix->Multiply(DOMMatrixInit {{.A = 2, .D = 2}}).ReleaseValue();
      CHECK(product->A() == 2);
      CHECK(product->E() == 5);
      REQUIRE_FALSE(matrix->PreMultiplySelf(DOMMatrixInit {{.A = 2, .D = 2}}).HasException());
      CHECK(matrix->E() == 10);

      auto inverse = matrix->Inverse();
      CHECK(inverse->A() == Approx(0.5));
      CHECK(inverse->E() == Approx(-5));

      // A singular matrix inverts to NaNs, and is no longer 2D.
      auto singular =
        DOMMatrix::Create(Variant<DOMString, List<double>>(List<double> {0, 0, 0, 0, 0, 0})).ReleaseValue();
      singular->InvertSelf();
      CHECK(std::isnan(singular->A()));
      CHECK_FALSE(singular->GetIs2D());
      CHECK(singular->ToString().GetException().Code() == ExceptionCode::InvalidStateError);
    }

    SECTION("setting 3D entries makes it 3D")
    {
      matrix->SetM33(1);
      CHECK(matrix->GetIs2D());
      matrix->SetM13(-0.0);
      CHECK(matrix->GetIs2D());
      matrix->SetM13(0.5);
      CHECK_FALSE(matrix->GetIs2D());
    }

    SECTION("three rotations apply about z, then y, then x")
    {
      // Post-multiplying rotations about z by 50, y by 40 and x by 30 degrees.
      matrix->RotateSelf(30, 40, 50);
      CHECK_FALSE(matrix->GetIs2D());
      CHECK(matrix->M11() == Approx(0.492404));
      CHECK(matrix->M12() == Approx(0.586824));
      CHECK(matrix->M13() == Approx(-0.642788));
      CHECK(matrix->M21() == Approx(-0.456826));
      CHECK(matrix->M22() == Approx(0.802872));
      CHECK(matrix->M23() == Approx(0.383022));
      CHECK(matrix->M31() == Approx(0.740843));
      CHECK(matrix->M32() == Approx(0.105040));
      CHECK(matrix->M33() == Approx(0.663414));
    }

    SECTION("scaling about a z origin makes it 3D")
    {
      // The translations to and from the origin are translateSelf()s, which make it 3D for a z.
      matrix->ScaleSelf(2, null, 1, 0, 0, 5);
      CHECK_FALSE(matrix->GetIs2D());
      CHECK(matrix->M43() == Approx(0));
    }

    SECTION("an identity matrix's zeros can be negative")
    {
      auto negativeZeros =
        DOMMatrix::Create(Variant<DOMString, List<double>>(List<double> {1, -0.0, -0.0, 1, -0.0, -0.0}))
          .ReleaseValue();
      CHECK(negativeZeros->IsIdentity());
      negativeZeros->SetM44(std::nan(""));
      CHECK_FALSE(negativeZeros->IsIdentity());
    }
  }

  TEST_CASE("DOMMatrix: attributes",
            "[CSS][Geometry][DOMMatrix]"
            "[geometry-1:dom-dommatrixreadonly-m11][geometry-1:dom-dommatrixreadonly-m12]"
            "[geometry-1:dom-dommatrixreadonly-m13][geometry-1:dom-dommatrixreadonly-m14]"
            "[geometry-1:dom-dommatrixreadonly-m21][geometry-1:dom-dommatrixreadonly-m22]"
            "[geometry-1:dom-dommatrixreadonly-m23][geometry-1:dom-dommatrixreadonly-m24]"
            "[geometry-1:dom-dommatrixreadonly-m31][geometry-1:dom-dommatrixreadonly-m32]"
            "[geometry-1:dom-dommatrixreadonly-m33][geometry-1:dom-dommatrixreadonly-m34]"
            "[geometry-1:dom-dommatrixreadonly-m41][geometry-1:dom-dommatrixreadonly-m42]"
            "[geometry-1:dom-dommatrixreadonly-m43][geometry-1:dom-dommatrixreadonly-m44]"
            "[geometry-1:dom-dommatrixreadonly-a][geometry-1:dom-dommatrixreadonly-b]"
            "[geometry-1:dom-dommatrixreadonly-c][geometry-1:dom-dommatrixreadonly-d]"
            "[geometry-1:dom-dommatrixreadonly-e][geometry-1:dom-dommatrixreadonly-f]"
            "[geometry-1:dom-dommatrixreadonly-is2d][geometry-1:dom-dommatrixreadonly-isidentity]")
  {
    using Setter = void (DOMMatrix::*)(double) noexcept;
    using Getter = double (DOMMatrixReadOnly::*)() const noexcept;

    struct Element
    {
      Setter Set;
      Getter Get;
      // What the element is in the identity matrix, and whether setting it to anything else makes it 3D.
      double Identity;
      bool Is3D;
    };

    const Element elements[] = {
      {&DOMMatrix::SetM11, &DOMMatrixReadOnly::M11, 1, false},
      {&DOMMatrix::SetM12, &DOMMatrixReadOnly::M12, 0, false},
      {&DOMMatrix::SetM13, &DOMMatrixReadOnly::M13, 0, true},
      {&DOMMatrix::SetM14, &DOMMatrixReadOnly::M14, 0, true},
      {&DOMMatrix::SetM21, &DOMMatrixReadOnly::M21, 0, false},
      {&DOMMatrix::SetM22, &DOMMatrixReadOnly::M22, 1, false},
      {&DOMMatrix::SetM23, &DOMMatrixReadOnly::M23, 0, true},
      {&DOMMatrix::SetM24, &DOMMatrixReadOnly::M24, 0, true},
      {&DOMMatrix::SetM31, &DOMMatrixReadOnly::M31, 0, true},
      {&DOMMatrix::SetM32, &DOMMatrixReadOnly::M32, 0, true},
      {&DOMMatrix::SetM33, &DOMMatrixReadOnly::M33, 1, true},
      {&DOMMatrix::SetM34, &DOMMatrixReadOnly::M34, 0, true},
      {&DOMMatrix::SetM41, &DOMMatrixReadOnly::M41, 0, false},
      {&DOMMatrix::SetM42, &DOMMatrixReadOnly::M42, 0, false},
      {&DOMMatrix::SetM43, &DOMMatrixReadOnly::M43, 0, true},
      {&DOMMatrix::SetM44, &DOMMatrixReadOnly::M44, 1, true},
    };

    for (const auto &element : elements)
    {
      auto matrix = DOMMatrix::Create().ReleaseValue();
      CHECK(((*matrix).*element.Get)() == element.Identity);

      // Setting the identity's value (or -0 for 0) keeps it 2D and the identity.
      ((*matrix).*element.Set)(element.Identity == 0 ? -0.0 : element.Identity);
      CHECK(matrix->GetIs2D());
      CHECK(matrix->IsIdentity());

      // Another value is read back, makes it 3D if the element is a 3D one, and it's no longer the identity.
      ((*matrix).*element.Set)(0.5);
      CHECK(((*matrix).*element.Get)() == 0.5);
      CHECK(matrix->GetIs2D() == !element.Is3D);
      CHECK_FALSE(matrix->IsIdentity());
    }

    // The letters are the 2D elements' other names.
    auto matrix = DOMMatrix::Create().ReleaseValue();
    matrix->SetA(2);
    matrix->SetB(3);
    matrix->SetC(4);
    matrix->SetD(5);
    matrix->SetE(6);
    matrix->SetF(7);
    CHECK(matrix->M11() == 2);
    CHECK(matrix->M12() == 3);
    CHECK(matrix->M21() == 4);
    CHECK(matrix->M22() == 5);
    CHECK(matrix->M41() == 6);
    CHECK(matrix->M42() == 7);
    CHECK(matrix->GetIs2D());
  }

  TEST_CASE(
    "DOMMatrix: points, arrays and strings",
    "[CSS][Geometry][DOMMatrix][geometry-1:dom-dommatrixreadonly-transformpoint]"
    "[geometry-1:dom-dompointreadonly-matrixtransform][geometry-1:transform-a-point-with-a-matrix]"
    "[geometry-1:dom-dommatrixreadonly-fromfloat32array][geometry-1:dom-dommatrixreadonly-fromfloat64array]"
    "[geometry-1:dommatrixreadonly-stringification-behavior]"
    "[geometry-1:dom-dommatrixreadonly-tofloat32array][geometry-1:dom-dommatrixreadonly-tofloat64array]")
  {
    auto matrix = DOMMatrix::Create().ReleaseValue()->Translate(10, 20)->ScaleSelf(2);

    auto point = matrix->TransformPoint({.X = 1, .Y = 1});
    CHECK(point->X() == 12);
    CHECK(point->Y() == 22);
    CHECK(point->W() == 1);

    auto transformed = DOMPoint::Create(1, 1)->MatrixTransform(DOMMatrixInit {{.E = 3}}).ReleaseValue();
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
