#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Gfx.Tests/TestDevice.hpp"
#include "Krystal.UI/Render2DConversions.hpp"
#include <filesystem>

namespace krys::ui::tests
{
  namespace
  {
    using Pixel = Array<uint8, 4>;

    /// @brief Draws into a target twice, recorded through Booey and replayed, and directly with Render2D, so
    /// the two can be compared pixel for pixel.
    class ReplayFixture
    {
    protected:
      UniquePtr<gfx::Device> _device;
      UniquePtr<gfx::Renderer2D> _renderer;
      FontFaceCache _fonts;
      gfx::TextureHandle _target {};
      static constexpr uint32 Width = 96u;
      static constexpr uint32 Height = 64u;

    public:
      ReplayFixture() : _device(gfx::tests::CreateTestDevice())
      {
        auto renderer = gfx::Renderer2D::Create(*_device);
        REQUIRE(renderer.has_value());
        _renderer = krys::move(*renderer);
        auto target =
          _device->CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                                  .Width = Width,
                                  .Height = Height,
                                  .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                                  .DebugName = u8"Target"});
        REQUIRE(target.has_value());
        _target = *target;
      }

      ~ReplayFixture()
      {
        _fonts.Clear();
        _renderer = nullptr;
        _device->Destroy(_target);
      }

      template <typename TFunc>
      KRYS_NODISCARD List<Pixel> Draw(TFunc &&draw)
      {
        _device->BeginFrame();
        auto &commands = _device->GetCommandBuffer();
        _renderer->Begin(commands, {.Texture = _target,
                                    .Width = Width,
                                    .Height = Height,
                                    .Format = gfx::TextureFormat::RGBA8Unorm,
                                    .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
        draw(*_renderer);
        _renderer->End();
        _device->Submit(commands);
        _device->EndFrame();

        List<Pixel> pixels(Width * Height);
        _device->ReadTexture(
          _target, gfx::TextureRegion::Whole2D(Width, Height),
          Span<byte>(reinterpret_cast<byte *>(pixels.data()), pixels.size() * sizeof(Pixel)));
        return pixels;
      }

      /// @brief What @p record draws, recorded into a display list and replayed.
      template <typename TFunc>
      KRYS_NODISCARD List<Pixel> Replayed(TFunc &&record)
      {
        boo::display_list::RecorderImpl recorder(boo::FloatSize {Width, Height});
        record(static_cast<boo::GraphicsContext &>(recorder));
        auto displayList = recorder.TakeDisplayList();
        return Draw(
          [&](gfx::Renderer2D &renderer)
          {
            GraphicsContextRender2D context(renderer, _fonts, {0, 0, Width, Height});
            context.DrawDisplayList(*displayList);
          });
      }

      KRYS_NODISCARD static bool Painted(const List<Pixel> &pixels)
      {
        for (const auto &pixel : pixels)
        {
          if (pixel[3])
          {
            return true;
          }
        }
        return false;
      }
    };

    KRYS_NODISCARD boo::Path Star()
    {
      boo::Path path;
      path.MoveTo({20, 2});
      path.AddLineTo({30, 38});
      path.AddLineTo({2, 14});
      path.AddLineTo({38, 14});
      path.AddLineTo({10, 38});
      path.CloseSubpath();
      return path;
    }
  }

  TEST_CASE_METHOD(ReplayFixture, "GraphicsContextRender2D: fills as Render2D does", "[UI][GPU]")
  {
    SECTION("Rects in a colour and with the brush, faded by the context's alpha")
    {
      auto replayed = Replayed(
        [](boo::GraphicsContext &context)
        {
          context.FillRect({4, 4, 20, 10}, boo::Color::red);
          context.SetFillColor(boo::Color::blue);
          context.SetAlpha(0.5f);
          context.FillRect({10, 20, 30, 10});
        });
      auto direct = Draw(
        [](gfx::Renderer2D &renderer)
        {
          renderer.FillRect({4, 4, 20, 10}, SRGBA<uint8> {255, 0, 0, 255});
          renderer.FillRect({10, 20, 30, 10}, SRGBA<uint8> {0, 0, 255, 128});
        });
      CHECK(Painted(direct));
      CHECK(replayed == direct);
    }

    SECTION("Rounded rects: circular corners analytically, elliptical ones as paths")
    {
      const boo::FloatRoundedRect circular({4, 4, 30, 20}, {6, 6}, {6, 6}, {6, 6}, {6, 6});
      const boo::FloatRoundedRect elliptical({40, 4, 40, 30}, {12, 6}, {12, 6}, {12, 6}, {12, 6});
      auto replayed = Replayed(
        [&](boo::GraphicsContext &context)
        {
          context.FillRoundedRect(circular, boo::Color::green);
          context.FillRoundedRect(elliptical, boo::Color::green);
        });
      auto direct = Draw(
        [&](gfx::Renderer2D &renderer)
        {
          renderer.FillRoundedRect(*ToCircularRoundedRect(circular), SRGBA<uint8> {0, 255, 0, 255});
          gfx::Path2D path;
          AddRoundedRect(path, elliptical);
          renderer.FillPath(path, gfx::FillRule::NonZero, SRGBA<uint8> {0, 255, 0, 255});
        });
      CHECK(replayed == direct);
    }

    SECTION("Paths with the fill rule, and gradients")
    {
      auto gradient =
        boo::Gradient::Create(boo::Gradient::LinearData {{0, 0}, {40, 0}},
                              {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied});
      gradient->AddColorStop({0, boo::Color::red});
      gradient->AddColorStop({1, boo::Color::blue});
      auto replayed = Replayed(
        [&](boo::GraphicsContext &context)
        {
          context.SetFillRule(boo::WindRule::EvenOdd);
          context.SetFillGradient(Ref<boo::Gradient>(gradient));
          context.FillPath(Star());
        });
      const auto gradient2D = ToGradient2D(*gradient);
      auto direct = Draw([&](gfx::Renderer2D &renderer)
                         { renderer.FillPath(ToPath2D(Star()), gfx::FillRule::EvenOdd, gradient2D); });
      CHECK(Painted(direct));
      CHECK(replayed == direct);
    }

    SECTION("A rect with a rounded hole")
    {
      const boo::FloatRoundedRect hole({10, 10, 30, 20}, {5, 5}, {5, 5}, {5, 5}, {5, 5});
      auto replayed = Replayed([&](boo::GraphicsContext &context)
                               { context.FillRectWithRoundedHole({4, 4, 42, 32}, hole, boo::Color::black); });
      auto direct = Draw(
        [&](gfx::Renderer2D &renderer)
        {
          renderer.FillRectWithRoundedHole({4, 4, 42, 32}, *ToCircularRoundedRect(hole),
                                           SRGBA<uint8> {0, 0, 0, 255});
        });
      CHECK(replayed == direct);
    }
  }

  TEST_CASE_METHOD(ReplayFixture, "GraphicsContextRender2D: strokes as Render2D does", "[UI][GPU]")
  {
    auto replayed = Replayed(
      [](boo::GraphicsContext &context)
      {
        context.SetStrokeColor(boo::Color::red);
        context.SetStrokeThickness(3);
        context.SetLineCap(boo::LineCap::Round);
        context.SetLineJoin(boo::LineJoin::Bevel);
        // An odd dash array repeats.
        context.SetLineDash({6, 3, 2}, 1);
        context.StrokePath(Star());
        context.StrokeRect({50, 10, 30, 30}, 2);
      });
    auto direct = Draw(
      [](gfx::Renderer2D &renderer)
      {
        renderer.StrokePath(ToPath2D(Star()),
                            {.Width = 3,
                             .Cap = gfx::LineCap::Round,
                             .Join = gfx::LineJoin::Bevel,
                             .MiterLimit = 4,
                             .Dashes = {6, 3, 2, 6, 3, 2},
                             .DashOffset = 1},
                            SRGBA<uint8> {255, 0, 0, 255});
        gfx::Path2D rect;
        rect.AddRect({50, 10, 30, 30});
        renderer.StrokePath(rect,
                            {.Width = 2,
                             .Cap = gfx::LineCap::Round,
                             .Join = gfx::LineJoin::Bevel,
                             .MiterLimit = 4,
                             .Dashes = {6, 3, 2, 6, 3, 2},
                             .DashOffset = 1},
                            SRGBA<uint8> {255, 0, 0, 255});
      });
    CHECK(Painted(direct));
    CHECK(replayed == direct);
  }

  TEST_CASE_METHOD(ReplayFixture, "GraphicsContextRender2D: transforms, clips and layers", "[UI][GPU]")
  {
    SECTION("Transforms and clips")
    {
      const boo::FloatRoundedRect clip({0, 0, 30, 30}, {8, 8}, {8, 8}, {8, 8}, {8, 8});
      auto replayed = Replayed(
        [&](boo::GraphicsContext &context)
        {
          context.Save();
          context.Translate(10, 5);
          context.Rotate(0.2f);
          context.ClipRoundedRect(clip);
          context.ClipOut(boo::FloatRect {10, 10, 5, 5});
          context.FillRect({0, 0, 40, 40}, boo::Color::blue);
          context.Restore();
          context.Clip({50, 0, 20, 20});
          context.FillRect({40, 0, 60, 60}, boo::Color::red);
        });
      auto direct = Draw(
        [&](gfx::Renderer2D &renderer)
        {
          renderer.Save();
          renderer.Translate(10, 5);
          renderer.Rotate(0.2f);
          renderer.ClipRoundedRect(*ToCircularRoundedRect(clip));
          renderer.ClipOutRect({10, 10, 5, 5});
          renderer.FillRect({0, 0, 40, 40}, SRGBA<uint8> {0, 0, 255, 255});
          renderer.Restore();
          renderer.ClipRect({50, 0, 20, 20});
          renderer.FillRect({40, 0, 60, 60}, SRGBA<uint8> {255, 0, 0, 255});
        });
      CHECK(replayed == direct);
    }

    SECTION("Transparency layers with opacity and blend modes")
    {
      auto replayed = Replayed(
        [](boo::GraphicsContext &context)
        {
          context.FillRect({0, 0, 60, 40}, boo::Color::yellow);
          context.BeginTransparencyLayer(0.5f);
          context.FillRect({10, 10, 30, 30}, boo::Color::blue);
          context.FillRect({20, 20, 30, 30}, boo::Color::blue);
          context.EndTransparencyLayer();
          context.BeginTransparencyLayer(boo::CompositeOperator::SourceOver, BlendMode::Multiply);
          context.FillRect({40, 5, 30, 30}, boo::Color::magenta);
          context.EndTransparencyLayer();
        });
      auto direct = Draw(
        [](gfx::Renderer2D &renderer)
        {
          renderer.FillRect({0, 0, 60, 40}, SRGBA<uint8> {255, 255, 0, 255});
          renderer.BeginLayer(0.5f);
          renderer.FillRect({10, 10, 30, 30}, SRGBA<uint8> {0, 0, 255, 255});
          renderer.FillRect({20, 20, 30, 30}, SRGBA<uint8> {0, 0, 255, 255});
          renderer.EndLayer();
          renderer.BeginLayer(1.f, BlendMode::Multiply);
          renderer.FillRect({40, 5, 30, 30}, SRGBA<uint8> {255, 0, 255, 255});
          renderer.EndLayer();
        });
      CHECK(replayed == direct);
    }

    SECTION("Clip bounds are in the current coordinates")
    {
      (void)Draw(
        [&](gfx::Renderer2D &renderer)
        {
          GraphicsContextRender2D context(renderer, _fonts, {0, 0, Width, Height});
          context.Clip({10, 10, 40, 30});
          context.Translate(10, 10);
          CHECK(context.ClipBounds() == boo::IntRect(0, 0, 40, 30));
        });
    }
  }

  TEST_CASE_METHOD(ReplayFixture, "GraphicsContextRender2D: shadows", "[UI][GPU]")
  {
    SECTION("A drop shadow follows each fill")
    {
      auto replayed = Replayed(
        [](boo::GraphicsContext &context)
        {
          context.SetDropShadow(
            {.Offset = {4, 6}, .Radius = 6, .Color = boo::Color(SRGBA<uint8> {0, 0, 0, 128})});
          context.FillRect({10, 10, 30, 20}, boo::Color::white);
        });
      auto direct = Draw(
        [](gfx::Renderer2D &renderer)
        {
          renderer.SetDropShadow({.Offset = {4, 6}, .BlurRadius = 6, .Colour = SRGBA<uint8> {0, 0, 0, 128}});
          renderer.FillRect({10, 10, 30, 20}, SRGBA<uint8> {255, 255, 255, 255});
        });
      CHECK(replayed == direct);
    }

    SECTION("A rect with a rounded hole and a shadow draws only the shadow")
    {
      const boo::FloatRoundedRect hole({10, 10, 40, 30}, {6, 6}, {6, 6}, {6, 6}, {6, 6});
      auto withShadow = Replayed(
        [&](boo::GraphicsContext &context)
        {
          context.ClipRoundedRect(hole);
          context.SetDropShadow({.Offset = {0, 3}, .Radius = 8, .Color = boo::Color::black});
          context.FillRectWithRoundedHole({0, 0, 60, 50}, hole, boo::Color::black);
        });
      // Inside the hole, the shadow is all there is; outside it, the clip leaves nothing.
      CHECK(Painted(withShadow));
      CHECK(withShadow[12 * Width + 30][3] > 0u);
      CHECK(withShadow[5 * Width + 5][3] == 0u);
      // The hole's middle is further from its edges than the blur reaches.
      CHECK(withShadow[25 * Width + 30][3] < withShadow[12 * Width + 30][3]);
    }
  }

  TEST_CASE_METHOD(ReplayFixture, "GraphicsContextRender2D: lines for borders and text", "[UI][GPU]")
  {
    SECTION("A text line is its rects")
    {
      auto replayed = Replayed(
        [](boo::GraphicsContext &context)
        {
          context.SetStrokeColor(boo::Color::red);
          const boo::FloatSegment segments[] {{0, 30}};
          context.DrawLinesForText({5, 10}, 2, segments, false, true, boo::StrokeStyle::SolidStroke);
        });
      auto direct = Draw(
        [](gfx::Renderer2D &renderer)
        {
          renderer.FillRect({5, 10, 30, 2}, SRGBA<uint8> {255, 0, 0, 255});
          renderer.FillRect({5, 14, 30, 2}, SRGBA<uint8> {255, 0, 0, 255});
        });
      CHECK(replayed == direct);
    }

    SECTION("A dotted border line has filled corners and dots between them, in the side's rect")
    {
      // The border painter passes the side rect's top-left and bottom-right corners.
      auto replayed = Replayed(
        [](boo::GraphicsContext &context)
        {
          context.SetStrokeColor(boo::Color::black);
          context.SetStrokeThickness(4);
          context.SetStrokeStyle(boo::StrokeStyle::DottedStroke);
          context.DrawLine({0, 8}, {80, 12});
        });
      auto painted = [&](uint32 x, uint32 y)
      {
        return replayed[y * Width + x][3] > 127u;
      };
      // The corners and the dots cover the same rows, the side's.
      for (const uint32 x : {1u, 78u})
      {
        CHECK_FALSE(painted(x, 7));
        CHECK(painted(x, 8));
        CHECK(painted(x, 11));
        CHECK_FALSE(painted(x, 12));
      }
      uint32 dotRows = 0;
      for (uint32 y = 6; y < 14; ++y)
      {
        bool any = false;
        for (uint32 x = 8; x < 72; ++x)
        {
          any = any || painted(x, y);
        }
        dotRows += any ? 1u : 0u;
        CHECK(any == (y >= 8 && y < 12));
      }
      CHECK(dotRows == 4u);
      // Gaps and dots alternate along the line.
      uint32 changes = 0;
      for (uint32 x = 1; x < 80; ++x)
      {
        changes += painted(x, 10) != painted(x - 1, 10) ? 1u : 0u;
      }
      CHECK(changes >= 10u);
    }
  }

  TEST_CASE_METHOD(ReplayFixture, "GraphicsContextRender2D: draws glyphs at their advances", "[UI][GPU]")
  {
    auto &database = boo::FontDatabase::Singleton();
    (void)database.AddFontFile(std::filesystem::path {KRYS_UI_TESTS_FONTS_DIR}
                               / "LiberationSans-Regular.ttf");
    auto faces = database.FacesInFamily(u8"Liberation Sans");
    REQUIRE_FALSE(faces.empty());
    auto font = boo::Font::Create(boo::FontPlatformData {ShareRef(*faces.front()), 20, false, false});

    const boo::GlyphBufferGlyph glyphs[] {
      static_cast<boo::GlyphBufferGlyph>(faces.front()->GlyphIndex(U'K')),
      static_cast<boo::GlyphBufferGlyph>(faces.front()->GlyphIndex(U'r')),
      static_cast<boo::GlyphBufferGlyph>(faces.front()->GlyphIndex(U'y'))};
    const boo::GlyphBufferAdvance advances[] {{13.5f, 0}, {7.25f, 0}, {10, 0}};
    auto replayed = Replayed(
      [&](boo::GraphicsContext &context)
      {
        context.SetFillColor(boo::Color::blue);
        context.DrawGlyphs(*font, glyphs, advances, {6, 30}, boo::FontSmoothingMode::Antialiased);
      });

    const auto *face = _fonts.FaceFor(*font);
    REQUIRE(face);
    auto direct = Draw(
      [&](gfx::Renderer2D &renderer)
      {
        const uint32 indices[] {glyphs[0], glyphs[1], glyphs[2]};
        const Vec2f positions[] {{0, 0}, {13.5f, 0}, {20.75f, 0}};
        renderer.DrawGlyphs(*face, 20, indices, positions, {6, 30}, SRGBA<uint8> {0, 0, 255, 255});
      });
    CHECK(Painted(direct));
    CHECK(replayed == direct);
    CHECK(_fonts.Size() == 1u);
  }
}
