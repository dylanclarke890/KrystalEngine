#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Engine/Application.hpp"
#include "Krystal.Engine/Events.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Keys.hpp"

#include <utility>

namespace Krys
{
  class Editor : public Application
  {
  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings)
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
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render();
    }
  };
}