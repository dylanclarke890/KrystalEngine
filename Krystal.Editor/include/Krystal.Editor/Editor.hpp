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
                                  .Format = gfx::TextureFormat::RGBA8Srgb,
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

      // A list in the sidebar, clipped to it, scrolling up forever.
      _renderer->Save();
      _renderer->ClipRect({16.f, 16.f, 208.f, height - 32.f});
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

      _renderer->End();
    }

  private:
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
