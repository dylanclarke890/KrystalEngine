#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include <numbers>

namespace krys::boo::tests
{
  namespace
  {
    using namespace display_list;

    KRYS_NODISCARD utf8_string AsText(RecorderImpl &recorder)
    {
      return recorder.TakeDisplayList()->AsText({});
    }

    KRYS_NODISCARD List<std::string_view> ItemNames(const DisplayList &displayList)
    {
      List<std::string_view> names;
      for (const auto &item : displayList.Items())
      {
        names.push_back(std::visit([]<typename TItem>(const TItem &) { return TItem::Name; }, item));
      }
      return names;
    }

    KRYS_NODISCARD List<std::string_view> ItemNames(RecorderImpl &recorder)
    {
      return ItemNames(*recorder.TakeDisplayList());
    }

    KRYS_NODISCARD Ref<Gradient> RedToBlue()
    {
      auto gradient =
        Gradient::Create(Gradient::LinearData {{0, 0}, {10, 0}},
                         {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied});
      gradient->AddColorStop({0, Color::red});
      gradient->AddColorStop({1, Color::blue});
      return gradient;
    }

    KRYS_NODISCARD Path Triangle()
    {
      Path path;
      path.MoveTo({0, 0});
      path.AddLineTo({10, 0});
      path.AddQuadCurveTo({10, 10}, {0, 10});
      path.CloseSubpath();
      return path;
    }
  }

  TEST_CASE("RecorderImpl: records drawing and the state it uses", "[Graphics][DisplayList]")
  {
    RecorderImpl recorder({100, 100});
    recorder.Save();
    recorder.Translate(10, 20);
    recorder.SetFillColor(Color::red);
    recorder.FillRect({0, 0, 10, 10});
    recorder.FillRect({0, 0, 10, 10}, Color::blue);
    recorder.SetDropShadow({.Offset = {2, 3}, .Radius = 4, .Color = Color::black});
    recorder.FillRoundedRect(FloatRoundedRect({0, 0, 20, 20}, {5, 5}, {5, 5}, {5, 5}, {5, 5}), Color::green);
    recorder.Restore();

    CHECK(AsText(recorder) == u8R"(
(save)
(translate
  (x 10.00)
  (y 20.00))
(set-inline-fill-color
  (color #FF0000))
(fill-rect
  (rect at (0,0) size 10x10)
  (requiresClipToRect 1))
(fill-rect-with-color
  (rect at (0,0) size 10x10)
  (color #0000FF))
(set-state
  (change-flags [drop-shadow])
  (drop-shadow 
    (offset width=2 height=3)
    (radius 4.00)
    (color #000000)
    (shadows-use-legacy-radius 0)
    (opacity 1.00)))
(fill-rounded-rect
  (rect at (0,0) size 20x20
    (top-left width=5 height=5)
    (top-right width=5 height=5)
    (bottom-left width=5 height=5)
    (bottom-right width=5 height=5))
  (color #00FF00)
  (blend-mode normal))
(restore))");
  }

  TEST_CASE("RecorderImpl: records paths, gradients, layers and text lines", "[Graphics][DisplayList]")
  {
    RecorderImpl recorder({100, 100});
    recorder.ClipPath(Triangle(), WindRule::NonZero);
    recorder.ConcatCTM(AffineTransform(2, 0, 0, 2, 5, 5));
    // The stroke is already black, so only its thickness changes.
    recorder.SetStrokeColor(Color::black);
    recorder.SetStrokeThickness(2);
    recorder.StrokePath(Triangle());
    auto gradient = RedToBlue();
    recorder.FillRect({0, 0, 10, 10}, *gradient);
    recorder.BeginTransparencyLayer(0.5f);
    recorder.DrawLineForText({0, 10, 50, 1}, false);
    recorder.EndTransparencyLayer();

    CHECK(AsText(recorder) == u8R"(
(clip-path
  (path move to (0,0), add line to (10,0), add quad curve to (10,10) (0,10), close subpath)
  (wind-rule NON-ZERO))
(concatenate-ctm
  (ctm {m=((2.00,0.00)(0.00,2.00)) t=(5.00,5.00)}))
(set-inline-stroke
  (thickness 2.00))
(stroke-path
  (path move to (0,0), add line to (10,0), add quad curve to (10,10) (0,10), close subpath))
(fill-rect-with-gradient
  (rect at (0,0) size 10x10)
  (gradient 
    (p0 (0,0))
    (p1 (10,0))
    (color-interpolation-method sRGB unpremultiplied)
    (spread-method pad)
    (stops [#FF0000 at 0.00, #0000FF at 1.00])))
(begin-transparency-layer
  (opacity 0.50))
(draw-lines-for-text
  (point (0,10))
  (thickness 1.00)
  (double 0)
  (lineSegments [
    (
      (begin 0.00)
      (end 50.00))])
  (is-printing 0)
  (double 0))
(end-transparency-layer))");
  }

  TEST_CASE("RecorderImpl: tracks the transform", "[Graphics][DisplayList]")
  {
    RecorderImpl recorder({100, 100});
    GraphicsContext &context = recorder;

    SECTION("Transforms that change nothing aren't recorded")
    {
      context.Translate(0, 0);
      context.Scale(1);
      context.Rotate(2 * std::numbers::pi_v<float>);
      context.ConcatCTM({});
      CHECK(context.GetCTM().IsIdentity());
      CHECK(ItemNames(recorder).empty());
    }

    SECTION("Transforms compose, and a restore brings the transform back")
    {
      context.Save();
      context.Translate(10, 20);
      context.Scale(2);
      CHECK(context.GetCTM() == AffineTransform(2, 0, 0, 2, 10, 20));
      context.SetCTM(AffineTransform(1, 0, 0, 1, 5, 5));
      CHECK(context.GetCTM() == AffineTransform(1, 0, 0, 1, 5, 5));
      context.Restore();
      CHECK(context.GetCTM().IsIdentity());
      CHECK(ItemNames(recorder)
            == List<std::string_view> {"save", "translate", "scale", "set-ctm", "restore"});
    }

    SECTION("Unmatched restores and layer ends are ignored")
    {
      context.Restore();
      context.EndTransparencyLayer();
      CHECK(ItemNames(recorder).empty());
    }
  }

  TEST_CASE("RecorderImpl: tracks the clip bounds", "[Graphics][DisplayList]")
  {
    RecorderImpl recorder({100, 100});
    GraphicsContext &context = recorder;
    CHECK(context.ClipBounds() == IntRect(0, 0, 100, 100));

    context.Clip({10, 10, 50, 50});
    CHECK(context.ClipBounds() == IntRect(10, 10, 50, 50));

    // Clip bounds are in the current coordinates.
    context.Translate(10, 10);
    CHECK(context.ClipBounds() == IntRect(0, 0, 50, 50));

    // Clipping out leaves the bounds as they were.
    context.ClipOut(FloatRect {0, 0, 20, 20});
    CHECK(context.ClipBounds() == IntRect(0, 0, 50, 50));

    // A path clips to its bounding rect.
    context.ClipPath(Triangle());
    CHECK(context.ClipBounds() == IntRect(0, 0, 10, 10));

    // Resetting goes back to the initial clip, recorded as a clip to it.
    context.ResetClip();
    CHECK(context.ClipBounds() == IntRect(-10, -10, 100, 100));
    CHECK(ItemNames(recorder)
          == List<std::string_view> {"clip", "translate", "clip-out", "clip-path", "reset-clip", "clip"});
  }

  TEST_CASE("RecorderImpl: records state changes when something is drawn", "[Graphics][DisplayList]")
  {
    RecorderImpl recorder({100, 100});
    GraphicsContext &context = recorder;

    SECTION("Changes made before drawing are recorded once, as they end up")
    {
      context.SetFillColor(Color::red);
      context.SetFillColor(Color::blue);
      context.SetStrokeThickness(3);
      context.FillRect({0, 0, 10, 10});
      CHECK(ItemNames(recorder)
            == List<std::string_view> {"set-inline-fill-color", "set-inline-stroke", "fill-rect"});
    }

    SECTION("Changes back to what was last drawn with aren't recorded")
    {
      context.SetFillColor(Color::red);
      context.FillRect({0, 0, 10, 10});
      context.SetFillColor(Color::blue);
      context.SetFillColor(Color::red);
      context.FillRect({0, 0, 10, 10});
      CHECK(ItemNames(recorder)
            == List<std::string_view> {"set-inline-fill-color", "fill-rect", "fill-rect"});
    }

    SECTION("Changes that can't be inlined record the whole state")
    {
      context.SetFillGradient(RedToBlue());
      context.FillRect({0, 0, 10, 10});
      context.SetAlpha(0.5f);
      context.FillRect({0, 0, 10, 10});
      CHECK(ItemNames(recorder)
            == List<std::string_view> {"set-state", "fill-rect", "set-state", "fill-rect"});
    }

    SECTION("A transparency layer starts from what was last drawn with")
    {
      context.SetFillColor(Color::red);
      context.FillRect({0, 0, 10, 10});
      context.BeginTransparencyLayer(0.5f);
      context.SetFillColor(Color::blue);
      context.SetFillColor(Color::red);
      context.FillRect({0, 0, 10, 10});
      context.EndTransparencyLayer();
      CHECK(ItemNames(recorder)
            == List<std::string_view> {"set-inline-fill-color", "fill-rect", "begin-transparency-layer",
                                       "fill-rect", "end-transparency-layer"});
    }

    SECTION("A restore brings the state back")
    {
      context.Save();
      context.SetFillColor(Color::red);
      context.FillRect({0, 0, 10, 10});
      context.Restore();
      CHECK(context.FillColor() == Color::black);
    }
  }

  TEST_CASE("RecorderImpl: records glyphs", "[Graphics][DisplayList]")
  {
    auto &database = FontDatabase::Singleton();
    (void)database.AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts" / "Ahem.ttf");
    auto faces = database.FacesInFamily(u8"Ahem");
    REQUIRE(faces.size() == 1u);
    auto font = Font::Create(FontPlatformData {ShareRef(*faces.front()), 16, false, false});

    RecorderImpl recorder({100, 100});
    const GlyphBufferGlyph glyphs[] {1, 2, 3};
    const GlyphBufferAdvance advances[] {{16, 0}, {16, 0}, {16, 0}};
    recorder.DrawGlyphs(*font, glyphs, advances, {5, 20}, FontSmoothingMode::Antialiased);

    auto displayList = recorder.TakeDisplayList();
    REQUIRE(displayList->Items().size() == 1u);
    const auto &item = std::get<display_list::DrawGlyphs>(displayList->Items()[0]);
    CHECK(item.GetFont().get() == font.get());
    CHECK(item.Glyphs() == List<GlyphBufferGlyph> {1, 2, 3});
    CHECK(item.Advances().size() == 3u);
    CHECK(item.LocalAnchor() == FloatPoint(5, 20));
    CHECK(displayList->AsText({}) == u8R"(
(draw-glyphs
  (local-anchor (5,20))
  (font-smoothing-mode antialiased)
  (length 3)))");
  }

  TEST_CASE("RecorderImpl: replays display lists", "[Graphics][DisplayList]")
  {
    RecorderImpl source({100, 100});
    source.SetFillColor(Color::red);
    source.FillRect({0, 0, 10, 10});
    source.Translate(5, 5);
    source.FillRoundedRect(FloatRoundedRect({0, 0, 20, 20}, {5, 5}, {5, 5}, {5, 5}, {5, 5}), Color::green);
    source.FillRectWithRoundedHole({0, 0, 30, 30}, FloatRoundedRect({5, 5, 20, 20}), Color::blue);
    auto displayList = source.TakeDisplayList();

    SECTION("Drawing a display list into a recorder records it as one item")
    {
      RecorderImpl recorder({100, 100});
      GraphicsContext &context = recorder;
      context.DrawDisplayList(*displayList);
      CHECK(ItemNames(recorder) == List<std::string_view> {"draw-display-list"});
    }

    SECTION("Appending a display list replays its items")
    {
      RecorderImpl recorder({100, 100});
      recorder.AppendDisplayList(*displayList);
      CHECK(static_cast<GraphicsContext &>(recorder).GetCTM() == AffineTransform(1, 0, 0, 1, 5, 5));
      CHECK(recorder.TakeDisplayList()->AsText({}) == displayList->AsText({}));
    }
  }

  TEST_CASE("RecorderImpl: keeps placeholders for drawing at playback", "[Graphics][DisplayList]")
  {
    RecorderImpl source({100, 100});
    source.DrawPlaceholder([](GraphicsContext &context) { context.FillRect({0, 0, 1, 1}, Color::red); });
    auto displayList = source.TakeDisplayList();

    // Each playback draws what the placeholder draws.
    RecorderImpl recorder({100, 100});
    recorder.AppendDisplayList(*displayList);
    recorder.AppendDisplayList(*displayList);
    CHECK(ItemNames(recorder) == List<std::string_view> {"fill-rect-with-color", "fill-rect-with-color"});
  }
}
