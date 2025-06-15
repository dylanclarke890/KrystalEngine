#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Engine/Application.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/Keys.hpp"

#include <utility>

namespace Krys
{
  class Editor : public Application
  {
  public:
    explicit Editor(Unique<ApplicationContext> context) noexcept : Application(std::move(context))
    {
      _context->Logger->SetLevel(Log::Level::Info);
      _context->Logger->Info("Editor Application Initialized");
    }

    void OnInit() noexcept override
    {
      _context->Events->On<Platform::QuitEvent>(
        [&](const auto &event)
        {
          Stop();

          return true;
        });

      _context->Events->On<Platform::KeyboardEvent>(
        [&](const auto &event)
        {
          if (event.GetState() == Platform::KeyState::Pressed)
          {
            _context->Logger->Info("Key Pressed {}", event.GetKey());
          }

          if (event.GetKey() == Platform::Key::ESCAPE)
          {
            _context->Logger->Info("Escape key pressed, quitting application...");
            Stop();
          }

          return true;
        });

      _context->GraphicsContext->SetupTestTriangle();
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->DrawTestTriangle();
    }
  };
}