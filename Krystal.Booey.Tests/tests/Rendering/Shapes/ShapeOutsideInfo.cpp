#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Shapes/LayoutShape.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD const LayoutShape &ShapeOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto *info = BoxOf(fixture, id).GetShapeOutsideInfo();
      krys_assert(info);
      return info->ComputedShape();
    }

    KRYS_NODISCARD bool IsSegment(const LineSegment &segment, float left, float right) noexcept
    {
      return segment.IsValid && segment.LogicalLeft == left && segment.LogicalRight == right;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ShapeOutsideInfo: the shapes floats exclude",
                   "[Rendering][ShapeOutsideInfo]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { float: left; width: 100px; height: 100px }
      #circle { shape-outside: circle(50px at 50px 50px) }
      #margin { shape-outside: circle(50px at 50px 50px); shape-margin: 10px }
      #ellipse { shape-outside: ellipse(50px 25px at 50px 50px) }
      #polygon { shape-outside: polygon(0 0, 100px 0, 0 100px) }
      #inset { shape-outside: inset(10px 20px 30px 40px) }
      #content { shape-outside: content-box; padding: 10px }
      #none { }
      #static { float: none; shape-outside: circle() }
    </style><div id=circle></div><div id=margin></div><div id=ellipse></div><div id=polygon></div><div id=inset></div><div id=content></div><div id=none></div><div id=static></div>)html"};
    (void)fixture.Document->UpdateLayout();

    CHECK_FALSE(BoxOf(fixture, u8"none").GetShapeOutsideInfo());
    // Only floats have a shape outside.
    CHECK_FALSE(BoxOf(fixture, u8"static").GetShapeOutsideInfo());

    const auto &circle = ShapeOf(fixture, u8"circle");
    CHECK(circle.ShapeMarginLogicalBoundingBox() == LayoutRect(0, 0, 100, 100));
    CHECK(IsSegment(circle.GetExcludedInterval(10_lu, 10_lu), 10, 90));
    CHECK(IsSegment(circle.GetExcludedInterval(50_lu, 10_lu), 0, 100));
    CHECK(IsSegment(circle.GetExcludedInterval(80_lu, 10_lu), 10, 90));
    CHECK_FALSE(circle.GetExcludedInterval(110_lu, 10_lu).IsValid);
    CHECK(BoxOf(fixture, u8"circle").GetShapeOutsideInfo()->ShapeLogicalBottom() == 100_lu);

    // The margin grows the shape by 10px all round.
    const auto &margin = ShapeOf(fixture, u8"margin");
    CHECK(margin.ShapeMarginLogicalBoundingBox() == LayoutRect(-10, -10, 120, 120));
    CHECK(IsSegment(margin.GetExcludedInterval(50_lu, 10_lu), -10, 110));

    // y = 40 is 10 above the centre: x = 50 * sqrt(1 - 100 / 625) = 50 * 0.9165 = 45.83.
    const auto &ellipse = ShapeOf(fixture, u8"ellipse");
    CHECK(ellipse.ShapeMarginLogicalBoundingBox() == LayoutRect(0, 25, 100, 50));
    auto ellipseSegment = ellipse.GetExcludedInterval(30_lu, 10_lu);
    CHECK(ellipseSegment.IsValid);
    CHECK(std::abs(ellipseSegment.LogicalLeft - (50 - 45.8258f)) < 0.01f);
    CHECK(std::abs(ellipseSegment.LogicalRight - (50 + 45.8258f)) < 0.01f);

    const auto &polygon = ShapeOf(fixture, u8"polygon");
    CHECK(IsSegment(polygon.GetExcludedInterval(50_lu, 10_lu), 0, 50));
    CHECK(IsSegment(polygon.GetExcludedInterval(0_lu, 10_lu), 0, 100));

    const auto &inset = ShapeOf(fixture, u8"inset");
    CHECK(inset.ShapeMarginLogicalBoundingBox() == LayoutRect(40, 10, 40, 60));
    CHECK(IsSegment(inset.GetExcludedInterval(20_lu, 10_lu), 40, 80));
    CHECK_FALSE(inset.GetExcludedInterval(0_lu, 5_lu).IsValid);

    // The content box sits inside the 10px padding.
    const auto &content = ShapeOf(fixture, u8"content");
    CHECK(content.ShapeMarginLogicalBoundingBox() == LayoutRect(10, 10, 100, 100));
    CHECK(IsSegment(content.GetExcludedInterval(50_lu, 10_lu), 10, 110));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ShapeOutsideInfo: lines wrap around the shape",
                   "[Rendering][ShapeOutsideInfo]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #float { float: left; width: 100px; height: 100px; shape-outside: circle(50px at 50px 50px) }
    </style><body><div id=container><div id=float></div><div style="height: 200px"></div></div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &container = Downcast<RenderBlockFlow>(*fixture.ById(u8"container").Renderer());
    // The shape is computed during layout; lines only see it once it exists.
    (void)ShapeOf(fixture, u8"float");

    // A line from 10 to 20 overlaps the circle up to x = 90, a line from 50 to 60 up to the full width, and a
    // line below the circle not at all.
    CHECK(container.LogicalLeftOffsetForLine(10_lu, 10_lu) == 90_lu);
    CHECK(container.LogicalLeftOffsetForLine(50_lu, 10_lu) == 100_lu);
    CHECK(container.LogicalLeftOffsetForLine(150_lu, 10_lu) == 0_lu);
  }
}
