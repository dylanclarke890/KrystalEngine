#include "Krystal.Booey/Geometry/DOMPoint.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Geometry/DOMMatrix.hpp"

namespace krys::boo::tests
{
  TEST_CASE("DOMPoint: made from coordinates and dictionaries",
            "[Geometry][DOMPoint][geometry-1:dom-dompointreadonly-dompointreadonly]"
            "[geometry-1:dom-dompointreadonly-frompoint][geometry-1:dom-dompoint-frompoint]"
            "[geometry-1:create-a-dompointreadonly-from-the-dictionary][geometry-1:DOMPoint]"
            "[geometry-1:dom-dompointreadonly-x][geometry-1:dom-dompointreadonly-y]"
            "[geometry-1:dom-dompointreadonly-z][geometry-1:dom-dompointreadonly-w]")
  {
    // The coordinates default to 0, and the w perspective to 1.
    auto origin = DOMPointReadOnly::Create();
    CHECK(origin->X() == 0);
    CHECK(origin->Y() == 0);
    CHECK(origin->Z() == 0);
    CHECK(origin->W() == 1);

    auto point = DOMPointReadOnly::FromPoint({.X = 1, .Z = 3});
    CHECK(point->X() == 1);
    CHECK(point->Y() == 0);
    CHECK(point->Z() == 3);
    CHECK(point->W() == 1);

    // A DOMPoint's coordinates can be changed.
    auto mutablePoint = DOMPoint::FromPoint({.Y = 2, .W = 4});
    mutablePoint->SetX(5);
    mutablePoint->SetZ(6);
    mutablePoint->SetW(7);
    CHECK(mutablePoint->X() == 5);
    CHECK(mutablePoint->Y() == 2);
    CHECK(mutablePoint->Z() == 6);
    CHECK(mutablePoint->W() == 7);
  }

  TEST_CASE("DOMPoint: transforming with a matrix",
            "[Geometry][DOMPoint][geometry-1:dom-dompointreadonly-matrixtransform]"
            "[geometry-1:transform-a-point-with-a-matrix]")
  {
    auto point = DOMPointReadOnly::Create(1, 2, 3, 1);

    // The point is the column vector (x, y, z, w), pre-multiplied by the matrix: a perspective term changes
    // w.
    DOMMatrixInit perspective;
    perspective.M41 = 10;
    perspective.M34 = 0.5;
    auto transformed = point->MatrixTransform(perspective).ReleaseValue();
    CHECK(transformed->X() == 11);
    CHECK(transformed->Y() == 2);
    CHECK(transformed->Z() == 3);
    CHECK(transformed->W() == 2.5);

    // The point itself isn't changed, and a dictionary that isn't valid throws.
    CHECK(point->X() == 1);
    CHECK(point->MatrixTransform(DOMMatrixInit {{.A = 1, .M11 = 2}}).GetException().Code()
          == ExceptionCode::TypeError);
  }
}
