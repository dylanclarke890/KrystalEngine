#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.Editor/Application.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Keys.hpp"
#include <cmath>

namespace krys
{
  class Editor : public Application
  {
  private:
    uint32 _width {0u};
    uint32 _height {0u};
    float _time {0.f};
    UniquePtr<gfx::Renderer2D> _renderer {};
    gfx::TextureHandle _image {};

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(settings.WindowSettings.Size.Width),
          _height(settings.WindowSettings.Size.Height)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");
      BindEvents();

      auto renderer = gfx::Renderer2D::Create(*_context->Device);
      if (!renderer)
      {
        _context->Logger->Error("{}", renderer.error());
        return;
      }
      _renderer = krys::move(*renderer);
      CreateImage();
    }

    void OnShutdown() noexcept override
    {
      _renderer = nullptr;
      if (_image.IsValid())
      {
        _context->Device->Destroy(_image);
      }
    }

    /// @brief A 16-by-16 colour gradient with a checkerboard over it, since the editor has no image assets.
    void CreateImage() noexcept
    {
      constexpr uint32 size = 16u;
      Array<Array<uint8, 4>, size * size> texels {};
      for (uint32 y = 0u; y < size; ++y)
      {
        for (uint32 x = 0u; x < size; ++x)
        {
          const bool light = (x / 4u + y / 4u) % 2u == 0u;
          const auto shade = static_cast<uint8>(light ? 255u : 170u);
          texels[y * size + x] = {static_cast<uint8>(x * shade / (size - 1u)),
                                  static_cast<uint8>(y * shade / (size - 1u)), shade, 255u};
        }
      }

      auto image =
        _context->Device->CreateTexture({.Width = size, .Height = size, .DebugName = u8"EditorImage"});
      if (!image)
      {
        _context->Logger->Error("{}", image.error());
        return;
      }
      _image = *image;
      _context->Device->WriteTexture(
        _image, gfx::TextureRegion::Whole2D(size, size),
        Span<const byte>(reinterpret_cast<const byte *>(texels.data()), sizeof(texels)));
    }

    void BindEvents() noexcept
    {
      using namespace pal;

      _context->Events->On<KeyboardEvent>(
        [&](const auto &event)
        {
          if (event.State() == pal::KeyState::Pressed)
          {
            _context->Logger->Info("Key Pressed '{}'", event.Key());
          }

          if (event.Key() == pal::Key::Escape)
          {
            _context->Logger->Info("ESC key pressed, quitting application...");
            Stop();
          }
          return true;
        });

      _context->Events->On<WindowResizeEvent>(
        [&](const auto &event)
        {
          _width = event.Width();
          _height = event.Height();
          return true;
        });

      _context->Events->On<WindowCloseEvent>(
        [&](const auto &)
        {
          Stop();
          return true;
        });
    }

    void OnUpdate(seconds dt) noexcept override
    {
      _time += static_cast<float>(dt.count());
    }

    void OnRender(gfx::CommandBuffer &commands) noexcept override
    {
      if (!_renderer || _width == 0u || _height == 0u)
      {
        return;
      }

      _renderer->Begin(commands, {.Swapchain = _context->Swapchain,
                                  .Width = _width,
                                  .Height = _height,
                                  .Format = gfx::TextureFormat::RGBA8Unorm,
                                  .ClearColour = Array<float, 4> {0.02f, 0.02f, 0.03f, 1.f}});

      // A sidebar, a rounded panel, a translucent rounded overlay across both, and a rounded square sliding
      // back and forth (with a circle riding on it).
      const float width = static_cast<float>(_width);
      const float height = static_cast<float>(_height);
      _renderer->FillRect({0.f, 0.f, 240.f, height}, SRGBA<uint8> {40, 44, 52});
      _renderer->FillRoundedRect({{260.f, 20.f, width - 280.f, 160.f}, gfx::CornerRadii2D::Uniform(12.f)},
                                 SRGBA<uint8> {70, 130, 180});
      _renderer->FillRoundedRect({{120.f, 120.f, 400.f, 200.f}, {.TopLeft = 40.f, .BottomRight = 40.f}},
                                 SRGBA<uint8> {255, 140, 0, 128});

      const float slide = (std::sin(_time) * 0.5f + 0.5f) * (width - 340.f);
      _renderer->FillRoundedRect(
        {{260.f + slide, height - 100.f, 80.f, 80.f}, gfx::CornerRadii2D::Uniform(16.f)},
        SRGBA<uint8> {220, 50, 70});
      _renderer->FillRoundedRect(
        {{280.f + slide, height - 80.f, 40.f, 40.f}, gfx::CornerRadii2D::Uniform(20.f)},
        SRGBA<uint8> {255, 255, 255});

      // A list in the sidebar, clipped to a rounded rect, scrolling up forever.
      _renderer->Save();
      _renderer->ClipRoundedRect({{16.f, 16.f, 208.f, height - 32.f}, gfx::CornerRadii2D::Uniform(12.f)});
      const float rowHeight = 36.f;
      const float scroll = std::fmod(_time * 40.f, rowHeight * 2.f);
      _renderer->Translate(16.f, 16.f - scroll);
      for (uint32 row = 0u; static_cast<float>(row) * rowHeight < height + rowHeight * 2.f; ++row)
      {
        const bool even = row % 2u == 0u;
        _renderer->FillRoundedRect({{0.f, static_cast<float>(row) * rowHeight, 208.f, rowHeight - 6.f},
                                    gfx::CornerRadii2D::Uniform(6.f)},
                                   even ? SRGBA<uint8> {62, 68, 81} : SRGBA<uint8> {52, 57, 68});
      }
      _renderer->Restore();

      // A rounded square spinning about its centre.
      _renderer->Save();
      _renderer->Translate(width - 120.f, 300.f);
      _renderer->Rotate(_time);
      _renderer->FillRoundedRect({{-50.f, -50.f, 100.f, 100.f}, gfx::CornerRadii2D::Uniform(14.f)},
                                 SRGBA<uint8> {120, 200, 120});
      _renderer->Restore();

      // The image, scaled up with nearest filtering in the panel, and turning with linear filtering.
      if (_image.IsValid())
      {
        _renderer->DrawImage(_image, {280.f, 36.f, 128.f, 128.f}, {.Filter = gfx::SamplerFilter::Nearest});
        _renderer->Save();
        _renderer->Translate(width - 120.f, 480.f);
        _renderer->Rotate(-_time * 0.5f);
        _renderer->DrawImage(_image, {-48.f, -48.f, 96.f, 96.f}, {.Opacity = 0.85f});
        _renderer->Restore();
      }

      // Paths: one star under each fill rule (even-odd leaves the middle hollow), a panel with elliptical
      // corners, and a star turning, which needs a new mask every frame.
      const auto star = Star(50.f);
      _renderer->Save();
      _renderer->Translate(440.f, 100.f);
      _renderer->FillPath(star, gfx::FillRule::NonZero, SRGBA<uint8> {255, 215, 0});
      _renderer->Translate(120.f, 0.f);
      _renderer->FillPath(star, gfx::FillRule::EvenOdd, SRGBA<uint8> {255, 215, 0});
      _renderer->Restore();

      gfx::Path2D oval;
      oval.AddRoundedRect({280.f, 200.f, 260.f, 90.f},
                          {Vec2f {60.f, 30.f}, Vec2f {20.f, 40.f}, Vec2f {60.f, 30.f}, Vec2f {20.f, 40.f}});
      _renderer->FillPath(oval, gfx::FillRule::NonZero, SRGBA<uint8> {150, 90, 200, 220});

      // Borders: a frame around the image with a rounded hole (analytic), and one with elliptical inner
      // corners (a path filled under even-odd, so through a mask).
      // A drop shadow under the image's frame, clipped out of the frame so it never shows through it.
      _renderer->Save();
      _renderer->ClipOutRoundedRect({{272.f, 28.f, 144.f, 144.f}, gfx::CornerRadii2D::Uniform(4.f)});
      _renderer->FillRoundedRect({{278.f, 36.f, 144.f, 144.f}, gfx::CornerRadii2D::Uniform(6.f)},
                                 SRGBA<uint8> {0, 0, 0, 110});
      _renderer->Restore();
      _renderer->FillRectWithRoundedHole({272.f, 28.f, 144.f, 144.f},
                                         {{280.f, 36.f, 128.f, 128.f}, gfx::CornerRadii2D::Uniform(10.f)},
                                         SRGBA<uint8> {230, 230, 240});
      gfx::Path2D ovalBorder;
      ovalBorder.AddRect({560.f, 200.f, 200.f, 90.f});
      ovalBorder.AddRoundedRect({572.f, 206.f, 176.f, 78.f}, {Vec2f {40.f, 20.f}, Vec2f {40.f, 20.f},
                                                              Vec2f {40.f, 20.f}, Vec2f {40.f, 20.f}});
      _renderer->FillPath(ovalBorder, gfx::FillRule::EvenOdd, SRGBA<uint8> {230, 230, 240});

      _renderer->Save();
      _renderer->Translate(width - 120.f, 120.f);
      _renderer->Rotate(_time * 0.8f);
      _renderer->FillPath(star, gfx::FillRule::NonZero, SRGBA<uint8> {255, 255, 255, 200});
      _renderer->Restore();

      DrawStrokes();

      // Path clips: the image scrolling behind a turning star-shaped window, and a panel with a circle cut
      // out of it.
      if (_image.IsValid())
      {
        _renderer->Save();
        _renderer->Translate(340.f, 565.f);
        _renderer->Rotate(_time * 0.4f);
        _renderer->ClipPath(Star(45.f), gfx::FillRule::NonZero);
        _renderer->SetTransform({});
        const float drift = std::fmod(_time * 20.f, 32.f);
        for (int y = 0; y < 4; ++y)
        {
          for (int x = 0; x < 4; ++x)
          {
            _renderer->DrawImage(_image,
                                 {290.f + static_cast<float>(x) * 32.f - drift,
                                  515.f + static_cast<float>(y) * 32.f - drift, 32.f, 32.f},
                                 {.Filter = gfx::SamplerFilter::Nearest});
          }
        }
        _renderer->Restore();
      }

      gfx::Path2D hole;
      hole.AddEllipse({490.f, 565.f}, {22.f, 22.f});
      _renderer->Save();
      _renderer->ClipOutPath(hole, gfx::FillRule::NonZero);
      _renderer->FillRoundedRect({{420.f, 528.f, 140.f, 74.f}, gfx::CornerRadii2D::Uniform(10.f)},
                                 SRGBA<uint8> {90, 160, 120});
      _renderer->Restore();

      // Opacity per draw against opacity for a group: on the left each circle is half transparent, so their
      // overlap mixes; on the right they're opaque inside a layer that fades as one.
      _renderer->FillRoundedRect({{595.f, 535.f, 50.f, 50.f}, gfx::CornerRadii2D::Uniform(25.f)},
                                 SRGBA<uint8> {230, 60, 60, 128});
      _renderer->FillRoundedRect({{620.f, 550.f, 50.f, 50.f}, gfx::CornerRadii2D::Uniform(25.f)},
                                 SRGBA<uint8> {60, 90, 230, 128});
      _renderer->BeginLayer(0.65f + 0.35f * std::sin(_time * 1.5f));
      _renderer->FillRoundedRect({{690.f, 535.f, 50.f, 50.f}, gfx::CornerRadii2D::Uniform(25.f)},
                                 SRGBA<uint8> {230, 60, 60});
      _renderer->FillRoundedRect({{715.f, 550.f, 50.f, 50.f}, gfx::CornerRadii2D::Uniform(25.f)},
                                 SRGBA<uint8> {60, 90, 230});
      _renderer->EndLayer();

      DrawBlendModes();
      DrawGradients();

      _renderer->End();
    }

  private:
    /// @brief The gradient sample: a turning linear gradient, a radial one with a circling focal point and a
    /// turning conic one, then a bar interpolated in OKLCH the long way round and a subtle grey bar that
    /// shows the dithering.
    void DrawGradients() noexcept
    {
      using namespace gfx;
      const ExtendedSRGBA<float> orange {1.f, 0.55f, 0.f, 1.f};
      const ExtendedSRGBA<float> purple {0.45f, 0.2f, 0.9f, 1.f};

      // The line turns about the tile's centre.
      const Vec2f centre {1073.f, 593.f};
      const Vec2f along {33.f * std::cos(_time), 33.f * std::sin(_time)};
      const Gradient2D linear {
        .Shape = LinearGradient2D {{centre.x - along.x, centre.y - along.y},
                                   {centre.x + along.x, centre.y + along.y}},
        .Stops = {{.Offset = 0.f, .Colour = orange}, {.Offset = 1.f, .Colour = purple}}};
      _renderer->FillRoundedRect({{1040.f, 560.f, 66.f, 66.f}, CornerRadii2D::Uniform(10.f)}, linear);

      const Vec2f focus {1153.f + 14.f * std::cos(_time * 1.3f), 593.f + 14.f * std::sin(_time * 1.3f)};
      const Gradient2D radial {.Shape = RadialGradient2D {focus, {1153.f, 593.f}, 0.f, 33.f},
                               .Stops = {{.Offset = 0.f, .Colour = {1.f, 1.f, 1.f, 1.f}},
                                         {.Offset = 1.f, .Colour = {0.1f, 0.3f, 0.8f, 1.f}}}};
      _renderer->FillRoundedRect({{1120.f, 560.f, 66.f, 66.f}, CornerRadii2D::Uniform(33.f)}, radial);

      const Gradient2D conic {.Shape = ConicGradient2D {{1233.f, 593.f}, _time * 0.7f},
                              .Stops = {{.Offset = 0.f, .Colour = {1.f, 0.2f, 0.2f, 1.f}},
                                        {.Offset = 0.33f, .Colour = {1.f, 0.9f, 0.2f, 1.f}},
                                        {.Offset = 0.5f, .Colour = {1.f, 0.9f, 0.2f, 1.f}},
                                        {.Offset = 0.5f, .Colour = {0.2f, 0.7f, 1.f, 1.f}},
                                        {.Offset = 1.f, .Colour = {1.f, 0.2f, 0.2f, 1.f}}}};
      _renderer->FillRoundedRect({{1200.f, 560.f, 66.f, 66.f}, CornerRadii2D::Uniform(33.f)}, conic);

      const Gradient2D hues {
        .Shape = LinearGradient2D {{1040.f, 0.f}, {1266.f, 0.f}},
        .Stops = {{.Offset = 0.f, .Colour = {1.f, 0.f, 0.f, 1.f}},
                  {.Offset = 1.f, .Colour = {0.f, 0.f, 1.f, 1.f}}},
        .Interpolation = {.ColourSpace = ColourInterpolationMethod::OKLCH {HueInterpolationMethod::Longer},
                          .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}};
      _renderer->FillRoundedRect({{1040.f, 640.f, 226.f, 22.f}, CornerRadii2D::Uniform(4.f)}, hues);

      const Gradient2D greys {.Shape = LinearGradient2D {{1040.f, 0.f}, {1266.f, 0.f}},
                              .Stops = {{.Offset = 0.f, .Colour = {0.16f, 0.17f, 0.2f, 1.f}},
                                        {.Offset = 1.f, .Colour = {0.24f, 0.25f, 0.29f, 1.f}}}};
      _renderer->FillRoundedRect({{1040.f, 672.f, 226.f, 22.f}, CornerRadii2D::Uniform(4.f)}, greys);
    }

    /// @brief The blend mode sample: circles bobbing over the image in multiply, screen, difference and
    /// color, then two opaque squares in a layer composited with multiply.
    void DrawBlendModes() noexcept
    {
      if (_image.IsValid())
      {
        _renderer->DrawImage(_image, {600.f, 625.f, 400.f, 80.f});
      }

      const SRGBA<uint8> orange {255, 140, 0};
      const BlendMode modes[] = {BlendMode::Multiply, BlendMode::Screen, BlendMode::Difference,
                                 BlendMode::Color};
      for (int i = 0; i < 4; ++i)
      {
        const float bob = 10.f * std::sin(_time * 2.f + static_cast<float>(i));
        _renderer->Save();
        _renderer->SetBlendMode(modes[i]);
        _renderer->FillRoundedRect({{610.f + static_cast<float>(i) * 80.f, 635.f + bob, 60.f, 60.f},
                                    gfx::CornerRadii2D::Uniform(30.f)},
                                   orange);
        _renderer->Restore();
      }

      _renderer->BeginLayer(1.f, BlendMode::Multiply);
      _renderer->FillRect({925.f, 640.f, 40.f, 40.f}, SRGBA<uint8> {255, 220, 60});
      _renderer->FillRect({945.f, 655.f, 40.f, 40.f}, SRGBA<uint8> {80, 200, 255});
      _renderer->EndLayer();
    }

    /// @brief The stroke sample: joins, curves, dots, a dashed box, marching ants and a turning dashed line.
    void DrawStrokes() noexcept
    {
      const SRGBA<uint8> blue {120, 180, 255};
      const SRGBA<uint8> light {230, 230, 240};

      // The same zigzag under each join: miter, round and bevel.
      const gfx::LineJoin joins[] = {gfx::LineJoin::Miter, gfx::LineJoin::Round, gfx::LineJoin::Bevel};
      for (int i = 0; i < 3; ++i)
      {
        const float left = 290.f + static_cast<float>(i) * 150.f;
        gfx::Path2D zigzag;
        zigzag.MoveTo({left, 400.f});
        zigzag.LineTo({left + 30.f, 345.f});
        zigzag.LineTo({left + 60.f, 400.f});
        zigzag.LineTo({left + 90.f, 345.f});
        zigzag.LineTo({left + 120.f, 400.f});
        _renderer->StrokePath(zigzag, {.Width = 10.f, .Join = joins[i]}, blue);
      }

      // A wave of cubics with round caps.
      gfx::Path2D wave;
      wave.MoveTo({290.f, 450.f});
      for (int i = 0; i < 4; ++i)
      {
        const float left = 290.f + static_cast<float>(i) * 105.f;
        wave.CubicTo({left + 35.f, 420.f}, {left + 70.f, 480.f}, {left + 105.f, 450.f});
      }
      _renderer->StrokePath(wave, {.Width = 6.f, .Cap = gfx::LineCap::Round}, SRGBA<uint8> {255, 160, 90});

      // Round dots: zero-length dashes under round caps, drawn without masks.
      _renderer->StrokeLine({295.f, 505.f}, {705.f, 505.f},
                            {.Width = 6.f, .Cap = gfx::LineCap::Round, .Dashes = {0.f, 14.f}}, light);

      // A dashed box from four straight lines.
      const gfx::StrokeStyle dashed {.Width = 2.f, .Dashes = {8.f, 5.f}};
      const Vec2f corners[] = {{780.f, 345.f}, {1000.f, 345.f}, {1000.f, 470.f}, {780.f, 470.f}};
      for (int i = 0; i < 4; ++i)
      {
        _renderer->StrokeLine(corners[i], corners[(i + 1) % 4], dashed, light);
      }

      // Inside the box, the image scrolls behind a rounded clip that turns.
      if (_image.IsValid())
      {
        _renderer->Save();
        _renderer->Translate(890.f, 407.f);
        _renderer->Rotate(std::sin(_time * 0.7f) * 0.6f);
        _renderer->ClipRoundedRect({{-80.f, -42.f, 160.f, 84.f}, gfx::CornerRadii2D::Uniform(18.f)});
        _renderer->SetTransform({});
        const float scroll = std::fmod(_time * 30.f, 64.f);
        for (int y = 0; y < 4; ++y)
        {
          for (int x = 0; x < 5; ++x)
          {
            _renderer->DrawImage(_image,
                                 {770.f + static_cast<float>(x) * 64.f - scroll,
                                  335.f + static_cast<float>(y) * 64.f - scroll, 64.f, 64.f},
                                 {.Filter = gfx::SamplerFilter::Nearest});
          }
        }
        _renderer->Restore();
      }

      // Marching ants: a dashed circle whose pattern slides round it.
      gfx::Path2D circle;
      circle.AddEllipse({890.f, 560.f}, {50.f, 50.f});
      _renderer->StrokePath(circle, {.Width = 3.f, .Dashes = {10.f, 6.f}, .DashOffset = -_time * 24.f},
                            light);

      // A dashed line turning inside the circle, under the renderer's transform.
      _renderer->Save();
      _renderer->Translate(890.f, 560.f);
      _renderer->Rotate(_time);
      _renderer->StrokeLine({-38.f, 0.f}, {38.f, 0.f},
                            {.Width = 4.f, .Cap = gfx::LineCap::Square, .Dashes = {6.f, 6.f}}, blue);
      _renderer->Restore();
    }

    /// @brief A five-pointed star drawn in one stroke, so its contour crosses itself.
    KRYS_NODISCARD static gfx::Path2D Star(float radius) noexcept
    {
      gfx::Path2D path;
      for (int i = 0; i < 5; ++i)
      {
        // Every second point of a pentagon: the classic pentagram.
        const float angle = -1.5707963f + static_cast<float>(i * 2) * 1.2566371f;
        const Vec2f point {radius * std::cos(angle), radius * std::sin(angle)};
        if (i == 0)
        {
          path.MoveTo(point);
        }
        else
        {
          path.LineTo(point);
        }
      }
      path.Close();
      return path;
    }
  };
}
