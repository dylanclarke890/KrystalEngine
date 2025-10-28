#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Debug/ScopedProfiler.hpp"
#include "Krystal.Engine/Application.hpp"
#include "Krystal.Gfx/Cameras/FirstPersonCamera.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Random.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.UI/Document.hpp"

namespace Krys
{
  class Editor : public Application
  {
  private:
    UI::Document _document;
    uint32 _width {0u};
    uint32 _height {0u};
    Gfx::FirstPersonCamera _camera;

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(settings.WindowSettings.Size.Width),
          _height(settings.WindowSettings.Size.Height),
          _camera({0.f, 0.f, 5.f}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, Maths::Radians(45.f),
                  (float)_width / (float)_height, 0.1f, 100.f)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(Log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");

      BindEvents();
      _context->GraphicsContext->Setup();

      SetupUI();
    }

    void BindEvents() noexcept
    {
      using namespace Events;

      _context->Events->On<KeyboardEvent>(
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

          if (event.Key() == Platform::Key::Space && event.State() == Platform::KeyState::Pressed)
          {
          }

          return true;
        });

      _context->Events->On<WindowResizeEvent>(
        [&](const auto &event)
        {
          _width = event.Width();
          _height = event.Height();
          _context->GraphicsContext->Resize(_width, _height);
          _camera.OnResize(_width, _height);
          return true;
        });

      _context->Events->On<WindowCloseEvent>(
        [&](const auto &event)
        {
          Stop();

          return true;
        });
    }

    void SetupUI()
    {
      using namespace Krys::UI;

      auto leftBox = _document.Create<Element>();
      _document.AppendChild(_document.Body(), leftBox);
      _document.SetStyle(leftBox,
                         [](auto node)
                         {
                           NodeStyleSetWidthPercent(node, 45.f);
                           NodeStyleSetMargin(node, Edge::Right, 5.f);
                           NodeStyleSetHeightPercent(node, 100.f);
                         });

      auto rightBox = _document.Create<Element>();
      _document.AppendChild(_document.Body(), rightBox);
      _document.SetStyle(rightBox,
                         [](auto node)
                         {
                           NodeStyleSetWidthPercent(node, 50.f);
                           NodeStyleSetMargin(node, Edge::Left, 5.f);
                           NodeStyleSetHeightPercent(node, 100.f);
                         });
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render(_camera);
    }

    void OnUpdate(double deltaTime) noexcept override
    {
      _camera.Update(deltaTime, *_context->Input);
    }
  };
}