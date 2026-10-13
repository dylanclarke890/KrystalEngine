#include "Krystal.Booey/Graphics/Region.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <random>

namespace krys::boo::tests
{
  namespace
  {
    using Shape = Region::Shape;
    using SpanList = SmallList<Region::Span, 16>;
    using SegmentList = SmallList<int, 32>;

    // The shape's segments and spans, which upstream compares through their text dump.
    void CheckShape(const Region &region, std::initializer_list<int> segments,
                    std::initializer_list<Region::Span> spans)
    {
      auto [actualSegments, actualSpans] = region.DataForTesting().DataForTesting();
      CHECK(actualSegments == SegmentList(segments));
      CHECK(actualSpans == SpanList(spans));
    }

    void CheckRects(const Region &region, std::initializer_list<IntRect> rects)
    {
      CHECK(region.Rects() == SmallList<IntRect, 1>(rects));
    }

    bool IsValid(const Region &region)
    {
      auto [segments, spans] = region.DataForTesting().DataForTesting();
      return Shape::IsValidShape(Span<const int>(segments.data(), segments.size()),
                                 Span<const Region::Span>(spans.data(), spans.size()));
    }

    IntRect RandomRect(std::minstd_rand &rand)
    {
      std::uniform_int_distribution<> coord(Limits<int>::min(), Limits<int>::max());
      int x0 = coord(rand);
      int y0 = coord(rand);
      int x1 = coord(rand);
      int y1 = coord(rand);
      if (x0 > x1)
      {
        std::swap(x0, x1);
      }
      if (y0 > y1)
      {
        std::swap(y0, y1);
      }
      return {x0, y0, x1 - x0, y1 - y0};
    }
  }

  TEST_CASE("Region: ShapeEmptyIsRepresentable", "[Graphics][Region]")
  {
    CHECK(Shape::IsValidShape({}, {}));
    Shape s1 = Shape::CreateForTesting({}, {});
    CHECK(s1.IsEmpty());
    CHECK(Shape {} == s1);
    CHECK(Shape {IntRect {}} == s1);
    Region r1 = Region::CreateForTesting(Shape {s1});
    CHECK(r1.IsEmpty());
    CHECK(s1 == r1.DataForTesting());
  }

  TEST_CASE("Region: ShapeEmptyIsEmpty", "[Graphics][Region]")
  {
    CHECK(Shape().IsEmpty());
    CHECK(Shape(IntRect {}).IsEmpty());
  }

  TEST_CASE("Region: IsValidShapeFalse", "[Graphics][Region]")
  {
    auto isValid = [](List<int> segments, List<Region::Span> spans)
    {
      return Shape::IsValidShape(segments, spans);
    };

    CHECK_FALSE(isValid({}, {{0, 0}}));
    CHECK_FALSE(isValid({1}, {{0, 1}}));
    CHECK_FALSE(isValid({0, 1}, {}));
    CHECK_FALSE(isValid({0, 1}, {{0, 2}}));
    CHECK_FALSE(
      isValid({0, 28, 8, 10, 31, 20, 5, 5'607'747, 11, 639, 23, 25, 20, 9}, {{703, 12}, {2'463'700, 2}}));
  }

  TEST_CASE("Region: UniteTests1", "[Graphics][Region]")
  {
    Region r1;
    r1.Unite(IntRect {50, 40, 50, 40});
    r1.Unite(IntRect {5, 5, 45, 35});
    CheckRects(r1, {IntRect(5, 5, 45, 35), IntRect(50, 40, 50, 40)});

    Region r2;
    r2.Unite(IntRect {5, 40, 45, 40});
    r2.Unite(IntRect {50, 5, 50, 35});
    CheckRects(r2, {IntRect(50, 5, 50, 35), IntRect(5, 40, 45, 40)});

    Region r3;
    r3.Unite(r1);
    r3.Unite(r2);
    Region r4 {IntRect {5, 5, 95, 75}};
    CHECK(r4.Contains(r3));
    CHECK(r3.Contains(r4));
    CHECK(r4 == r3);
    CHECK(r4.IsRect());
    CHECK(r3.IsRect());
  }

  // Describes how the algorithm stores an individual rectangle.
  TEST_CASE("Region: ShapeFormatIndividual", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    CheckShape(r1, {0, 10}, {{0, 0}, {10, 2}});
  }

  // Describes how the algorithm stores disjoint rectangles.
  // The rect is marked as ended with the span that has y: as the end corner and segmentIndex the same as the
  // previous span. I.e the "y: 50 si: 2" part.
  TEST_CASE("Region: ShapeFormatDisjoint", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    r1.Unite(IntRect {50, 50, 60, 60});
    CheckShape(r1, {0, 10, 50, 110}, {{0, 0}, {10, 2}, {50, 2}, {110, 4}});
  }

  // Describes how the algorithm stores x-joint mergeable rectangles.
  // It merges them and produces single rect.
  TEST_CASE("Region: ShapeFormatTestJointXMergeable", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    r1.Unite(IntRect {10, 0, 10, 10});
    CheckShape(r1, {0, 20}, {{0, 0}, {10, 2}});
  }

  // Describes how the algorithm stores mergeable y-joint rectangles.
  // It merges them and produces a single rect.
  TEST_CASE("Region: ShapeFormatTestJointYMergeable", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    r1.Unite(IntRect {0, 10, 10, 10});
    CheckShape(r1, {0, 10}, {{0, 0}, {20, 2}});
  }

  // Describes how the algorithm stores x-joint rectangles.
  // It merges the horizontal parts and produces new rect for the leftover vertical part.
  TEST_CASE("Region: ShapeFormatTestJointX", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    r1.Unite(IntRect {10, 0, 10, 11});
    CheckShape(r1, {0, 20, 10, 20}, {{0, 0}, {10, 2}, {11, 4}});
  }

  // Describes how the algorithm stores y-joint rectangles.
  // It does not merge anything.
  TEST_CASE("Region: ShapeFormatTestJointY", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    r1.Unite(IntRect {0, 10, 11, 10});
    CheckShape(r1, {0, 10, 0, 11}, {{0, 0}, {10, 2}, {20, 4}});
  }

  // Describes how the algorithm always produces even number of segments.
  // Other algorithm could share the segments, but not this one.
  TEST_CASE("Region: ShapeFormatTestEvenSegments", "[Graphics][Region]")
  {
    Region r1 {IntRect {0, 0, 10, 10}};
    r1.Unite(IntRect {10, 10, 10, 10});
    CheckRects(r1, {IntRect(0, 0, 10, 10), IntRect(10, 10, 10, 10)});
    CheckShape(r1, {0, 10, 10, 20}, {{0, 0}, {10, 2}, {20, 4}});
  }

  // Describes how the algorithm always produces sorted segment array per Span.
  // The whole segment array is not sorted, only per Span regions of it.
  TEST_CASE("Region: ShapeFormatTestSortedSpan", "[Graphics][Region]")
  {
    Region r1 {IntRect {10, 0, 10, 10}};
    r1.Unite(IntRect {1, 1, 5, 5});
    CheckRects(r1,
               {IntRect(10, 0, 10, 1), IntRect(1, 1, 5, 5), IntRect(10, 1, 10, 5), IntRect(10, 6, 10, 4)});
    CheckShape(r1, {10, 20, 1, 6, 10, 20, 10, 20}, {{0, 0}, {1, 2}, {6, 6}, {10, 8}});
  }

  // Tests that Region operations never produce segment-span lists that fail isValidShape.
  TEST_CASE("Region: FuzzOperationsIsValidShape", "[Graphics][Region]")
  {
    constexpr int iterations = 5'000;
    // A fixed seed, so a failure can be reproduced.
    std::minstd_rand rand(20'261'013);
    std::uniform_int_distribution<> operationCount(0, 15);
    std::uniform_int_distribution<> operation(0, 2);

    bool allValid = true;
    for (int i = 0; i < iterations && allValid; ++i)
    {
      Region r;
      const int operations = operationCount(rand);
      for (int j = 0; j < operations; ++j)
      {
        auto rect = RandomRect(rand);
        switch (operation(rand))
        {
          case 0: r.Unite(rect); break;
          case 1: r.Intersect(rect); break;
          case 2: r.Subtract(rect); break;
        }
        (void)r.Rects(); // No crash.
        if (!IsValid(r))
        {
          allValid = false;
          break;
        }
      }
    }
    CHECK(allValid);
  }

  TEST_CASE("Region: IsValidShape1", "[Graphics][Region]")
  {
    Region r;
    r.Unite(IntRect {620'280'709, 86'198'313, 951'242'283, 733'368'814});
    r.Subtract(IntRect {416'621'960, 440'858'151, 1'275'303'923, 1'424'992'047});
    r.Subtract(IntRect {1'329'326'038, 360'395'968, 435'226'361, 1'588'476'209});
    REQUIRE(IsValid(r));
  }

  TEST_CASE("Region: IsValidShape2", "[Graphics][Region]")
  {
    Region r;
    r.Subtract(IntRect {384'928'833, 12'330'228, 959'619'016, 578'249'146});
    r.Intersect(IntRect {828'730'499, 921'697'549, 1'295'834'393, 903'531'184});
    r.Intersect(IntRect {290'731'592, 195'208'138, 497'331'448, 106'244'923});
    r.Unite(IntRect {587'928'172, 216'906'104, 1'428'192'965, 1'718'162'329});
    r.Subtract(IntRect {1'745'551'117, 534'538'086, 297'055'811, 1'154'752'629});
    REQUIRE(IsValid(r));
  }

  TEST_CASE("Region: Operations", "[Graphics][Region]")
  {
    // An L shape: a 20x10 bar with a 10x10 block below its left half.
    Region l {IntRect {0, 0, 20, 10}};
    l.Unite(IntRect {0, 10, 10, 10});
    CHECK_FALSE(l.IsRect());
    CHECK(l.Bounds() == IntRect(0, 0, 20, 20));
    CHECK(l.TotalArea() == 300);
    CHECK(l.GridSize() == 12);

    CHECK(l.Contains(IntPoint(15, 5)));
    CHECK(l.Contains(IntPoint(5, 15)));
    CHECK_FALSE(l.Contains(IntPoint(15, 15)));
    CHECK_FALSE(l.Contains(IntPoint(20, 5)));
    CHECK(l.Contains(Region(IntRect(0, 0, 5, 20))));
    CHECK_FALSE(l.Contains(Region(IntRect(5, 5, 10, 10))));

    CHECK(l.Intersects(Region(IntRect(5, 5, 10, 10))));
    CHECK_FALSE(l.Intersects(Region(IntRect(12, 12, 5, 5))));

    // Subtracting the missing corner changes nothing; subtracting the bar leaves the block.
    CHECK(Subtract(l, Region(IntRect(10, 10, 10, 10))) == l);
    Region block = Subtract(l, Region(IntRect(0, 0, 20, 10)));
    CHECK(block.IsRect());
    CHECK(block.Bounds() == IntRect(0, 10, 10, 10));

    // Intersecting with a rect clips the shape.
    Region clipped = Intersect(l, Region(IntRect(5, 5, 10, 10)));
    CheckRects(clipped, {IntRect(5, 5, 10, 5), IntRect(5, 10, 5, 5)});
    CHECK(Intersect(l, Region(IntRect(50, 50, 1, 1))).IsEmpty());

    Region moved = Translate(l, IntSize(5, -5));
    CHECK(moved.Bounds() == IntRect(5, -5, 20, 20));
    CheckRects(moved, {IntRect(5, -5, 20, 10), IntRect(5, 5, 10, 10)});
  }

  TEST_CASE("Region: Copy and move", "[Graphics][Region]")
  {
    Region l {IntRect {0, 0, 20, 10}};
    l.Unite(IntRect {0, 10, 10, 10});

    Region copy = l;
    CHECK(copy == l);
    copy.Translate(IntSize(1, 1));
    CHECK(copy != l);

    Region moved = krys::move(copy);
    CHECK(moved.Bounds() == IntRect(1, 1, 20, 20));

    Region assigned;
    assigned = l;
    CHECK(assigned == l);
    assigned = Region(IntRect(1, 2, 3, 4));
    CHECK(assigned.IsRect());
    CHECK(assigned == Region(IntRect(1, 2, 3, 4)));
  }
}
