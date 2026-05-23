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
#include "Krystal.UI/Compositor.hpp"
#include "Krystal.UI/Document.hpp"

namespace Krys
{
  static UI::ElementHandle _test;

  class Editor : public Application
  {
  private:
    uint32 _width {0u};
    uint32 _height {0u};
    Gfx::FirstPersonCamera _camera;
    UI::Document _document;
    UI::Compositor _compositor;

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(settings.WindowSettings.Size.Width),
          _height(settings.WindowSettings.Size.Height),
          _camera({0.f, 0.f, 5.f}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, Maths::Radians(45.f),
                  (float)_width / (float)_height, 0.1f, 100.f),
          _document(*_context->GraphicsContext), _compositor(*_context->GraphicsContext, *_context->Renderer)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(Log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");

      BindEvents();
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
            _document.ElementStyleSetBackgroundColour(
              _test, Maths::Random::Choice<Gfx::ColourbPremultiplied>(
                       {Gfx::Colours::Red, Gfx::Colours::Green, Gfx::Colours::Blue, Gfx::Colours::Cyan,
                        Gfx::Colours::Magenta, Gfx::Colours::Yellow}));
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
      using namespace Krys::Maths;
      using namespace Krys::Gfx::Colours;

      _document.ElementStyleSetBackgroundColour(_document.Body(), Maroon);
      _document.ElementStyleSetFlexDirection(_document.Body(), FlexDirection::Row);
      _document.ElementStyleSetOpacity(_document.Body(), 0.8f);

      auto leftBox = _document.Create<Element>();
      _document.AppendChild(_document.Body(), leftBox);
      _document.ElementStyleSetWidth(leftBox, 600.f);
      _document.ElementStyleSetHeight(leftBox, 600.f);
      _document.ElementStyleSetBorderRadii(leftBox, {300.f, 300.f, 300.f, 300.f});
      _document.ElementStyleSetBorderWidths(leftBox, {0.f, 20.f, 20.f, 0.f});
      _document.ElementStyleSetBorderColours(leftBox, {Blue, Red, Orange, Black});
      _document.ElementStyleSetBackgroundColour(leftBox, Green);
      _document.ElementSetTextContent(leftBox, u8"Hello, Krystal UI!\nThis text should be on a newline!");
      _document.ElementStyleSetTextColour(leftBox, Black);
      _document.ElementStyleSetOpacity(leftBox, 0.8f);
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render(_camera);
      _compositor.Render(_document);
    }

    void OnUpdate(Seconds dt) noexcept override
    {
      _camera.Update(dt.count(), *_context->Input);
    }
  };
}