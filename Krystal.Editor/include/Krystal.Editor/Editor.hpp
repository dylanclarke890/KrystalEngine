#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Engine/Application.hpp"
#include "Krystal.Engine/Events.hpp"
#include "Krystal.Gfx/Cameras/PerspectiveCamera.hpp"
#include "Krystal.IO/Image.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Platform/Keys.hpp"

namespace Krys
{
  class Editor : public Application
  {
  private:
    Gfx::PerspectiveCamera _camera;

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings),
          _camera(Maths::Vec3(0.0f, 0.0f, 2.0f), Maths::Vec3(0.0f, 0.0f, 0.0f), Maths::Vec3(0.0f, 1.0f, 0.0f),
                  Maths::Radians(45.0f),
                  static_cast<float>(settings.WindowSettings.Size.Width)
                    / static_cast<float>(settings.WindowSettings.Size.Height),
                  0.1f, 100.0f)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(Log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");

      _context->Events->On<Engine::CloseEvent>(
        [&](const auto &event)
        {
          Stop();

          return true;
        });

      _context->Events->On<Engine::KeyboardEvent>(
        [&](const auto &event)
        {
          if (event.State() == Platform::KeyState::Pressed)
          {
            _context->Logger->Info("Key Pressed '{}'", event.Key());
          }

          if (event.Key() == Platform::Key::Escape)
          {
            _context->Logger->Info("ESC key pressed, quitting application...");
            Stop();
          }

          return true;
        });

      _context->GraphicsContext->Setup();

      {
        using namespace IO;

        auto baseDir = Path("data/assets/skyboxes/sky");
        auto skybox =
          LoadCubeMap(baseDir / Path("right.jpg"), baseDir / Path("left.jpg"), baseDir / Path("top.jpg"),
                      baseDir / Path("bottom.jpg"), baseDir / Path("front.jpg"), baseDir / Path("back.jpg"));
        assert(skybox.has_value());

        _context->GraphicsContext->SetSkybox(skybox.value());
      }
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render(_camera);
    }
  };
}