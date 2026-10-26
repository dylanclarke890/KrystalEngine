#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/SVG/PropertyTestUtils.hpp"

namespace krys::boo::svg::tests
{
  TEST_CASE("SVGPointList: parsing and serialising", "[SVG][SVGPointList]")
  {
    auto list = SVGPointList::Create();
    CHECK(list->Parse(u8"0,0 10,5 , 20 30"));
    REQUIRE(list->NumberOfItems() == 3);
    CHECK(list->Items()[1]->X() == 10);
    CHECK(list->ValueAsString() == u8"0 0 10 5 20 30");

    // A trailing comma or an unpaired coordinate fails.
    CHECK_FALSE(list->Parse(u8"1 2,"));
    CHECK_FALSE(list->Parse(u8"1 2 3"));

    CHECK(list->Parse(u8""));
    CHECK(list->IsEmpty());
  }

  TEST_CASE("SVGPoint: coordinates and transforms", "[SVG][SVGPoint]")
  {
    auto point = SVGPoint::Create({1, 2});
    REQUIRE_FALSE(point->SetX(3).HasException());
    REQUIRE_FALSE(point->SetY(4).HasException());
    CHECK(point->Value() == FloatPoint(3, 4));

    auto matrix = SVGMatrix::Create(AffineTransform(2, 0, 0, 2, 10, 20));
    auto transformed = point->MatrixTransform(*matrix);
    CHECK(transformed->Value() == FloatPoint(16, 28));
    CHECK(point->Value() == FloatPoint(3, 4));
    CHECK(point->Clone()->Value() == FloatPoint(3, 4));
  }
}
