#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Core/Log/ILogger.hpp"
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
    }

    void OnShutdown() noexcept override
    {
      _renderer = nullptr;
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

      // A sidebar and a panel, a translucent overlay across both, and a square sliding back and forth.
      const float width = static_cast<float>(_width);
      const float height = static_cast<float>(_height);
      _renderer->FillRect({0.f, 0.f, 240.f, height}, SRGBA<uint8> {40, 44, 52});
      _renderer->FillRect({260.f, 20.f, width - 280.f, 160.f}, SRGBA<uint8> {70, 130, 180});
      _renderer->FillRect({120.f, 120.f, 400.f, 200.f}, SRGBA<uint8> {255, 140, 0, 128});

      const float slide = (std::sin(_time) * 0.5f + 0.5f) * (width - 340.f);
      _renderer->FillRect({260.f + slide, height - 100.f, 80.f, 80.f}, SRGBA<uint8> {220, 50, 70});

      _renderer->End();
    }
  };
}
