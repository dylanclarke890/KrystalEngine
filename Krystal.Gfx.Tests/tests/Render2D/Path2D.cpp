#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::gfx::tests
{
  namespace
  {
    KRYS_NODISCARD List<PathVerb> VerbsOf(const Path2D &path)
    {
      return {path.Verbs().begin(), path.Verbs().end()};
    }

    KRYS_NODISCARD bool Same(Vec2f a, Vec2f b) noexcept
    {
      return a.x == b.x && a.y == b.y;
    }
  }

  TEST_CASE("Path2D: records segments and their points", "[Gfx][Render2D]")
  {
    Path2D path;
    CHECK(path.IsEmpty());
    path.MoveTo({0.f, 0.f});
    path.LineTo({1.f, 0.f});
    path.QuadTo({2.f, 0.f}, {2.f, 1.f});
    path.CubicTo({2.f, 2.f}, {1.f, 3.f}, {0.f, 3.f});
    path.Close();

    CHECK(VerbsOf(path)
          == List<PathVerb> {PathVerb::MoveTo, PathVerb::LineTo, PathVerb::QuadTo, PathVerb::CubicTo,
                             PathVerb::Close});
    REQUIRE(path.Points().size() == 7u);
    CHECK(Same(path.Points()[2], {2.f, 0.f}));
    CHECK(Same(path.Points()[6], {0.f, 3.f}));

    path.Clear();
    CHECK(path.IsEmpty());
  }

  TEST_CASE("Path2D: starts contours where segments need them", "[Gfx][Render2D]")
  {
    // A line without a current point starts a contour at its point.
    Path2D path;
    path.LineTo({1.f, 1.f});
    CHECK(VerbsOf(path) == List<PathVerb> {PathVerb::MoveTo, PathVerb::LineTo});
    CHECK(Same(path.Points()[0], {1.f, 1.f}));

    // After a close, the next segment continues from the closed contour's start.
    path.LineTo({4.f, 1.f});
    path.Close();
    path.LineTo({4.f, 4.f});
    CHECK(VerbsOf(path)
          == List<PathVerb> {PathVerb::MoveTo, PathVerb::LineTo, PathVerb::LineTo, PathVerb::Close,
                             PathVerb::MoveTo, PathVerb::LineTo});
    CHECK(Same(path.Points()[3], {1.f, 1.f}));

    // Closing twice, or with nothing open, adds nothing.
    path.Close();
    path.Close();
    CHECK(VerbsOf(path).back() == PathVerb::Close);
    CHECK(VerbsOf(path).size() == 7u);
  }

  TEST_CASE("Path2D: builds shapes", "[Gfx][Render2D]")
  {
    Path2D rect;
    rect.AddRect({1.f, 2.f, 3.f, 4.f});
    CHECK(VerbsOf(rect)
          == List<PathVerb> {PathVerb::MoveTo, PathVerb::LineTo, PathVerb::LineTo, PathVerb::LineTo,
                             PathVerb::Close});
    CHECK(Same(rect.Points()[2], {4.f, 6.f}));

    // Square corners need no curves; radii that don't fit are scaled down together.
    Path2D square;
    square.AddRoundedRect({0.f, 0.f, 10.f, 10.f}, {});
    CHECK(VerbsOf(square).size() == 6u);

    Path2D pill;
    pill.AddRoundedRect({0.f, 0.f, 20.f, 10.f},
                        {Vec2f {50.f, 50.f}, Vec2f {50.f, 50.f}, Vec2f {50.f, 50.f}, Vec2f {50.f, 50.f}});
    // The height limits the radii: two 50s across 10 pixels scale every radius to 5.
    CHECK(Same(pill.Points()[0], {5.f, 0.f}));
    CHECK(Same(pill.Points()[1], {15.f, 0.f}));

    Path2D ellipse;
    ellipse.AddEllipse({0.f, 0.f}, {2.f, 1.f});
    CHECK(VerbsOf(ellipse).size() == 6u);
    CHECK(Same(ellipse.Points()[0], {2.f, 0.f}));
    CHECK(Same(ellipse.Points()[3], {0.f, 1.f}));
  }

  TEST_CASE("Path2D: compares by its segments", "[Gfx][Render2D]")
  {
    Path2D a;
    a.AddRect({0.f, 0.f, 1.f, 1.f});
    Path2D b;
    b.AddRect({0.f, 0.f, 1.f, 1.f});
    CHECK(a == b);
    b.LineTo({2.f, 2.f});
    CHECK_FALSE(a == b);
  }
}
