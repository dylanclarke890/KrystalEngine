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
            auto current = _document.GetBody().GetFlexDirection();
            _document.GetBody().SetFlexDirection(current == UI::FlexDirection::Row ? UI::FlexDirection::Column
                                                                                   : UI::FlexDirection::Row);
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

      _document.GetBody().SetBackgroundColour(Gfx::Colour {.0706f});
      _document.GetBody().SetFlexDirection(FlexDirection::Column);
      _document.GetBody().SetWrap(Wrap::NoWrap);
      _document.GetBody().SetPadding(20._px);

      const auto RandomColour = []()
      {
        return Gfx::Colour {Random::Float(0.f, 1.f), Random::Float(0.f, 1.f), Random::Float(0.f, 1.f), 1.f};
      };

      // === Parent box ===
      auto parentHandle = _document.CreateElement<Element>();
      _document.AddToBody(parentHandle);
      auto &parent = _document.GetByHandle<Element>(parentHandle);

      parent.SetWidth(600._px);
      parent.SetHeight(400._px);
      parent.SetMargin(20._px);
      parent.SetPadding(10._px);
      parent.SetBackgroundColour(Gfx::Colour {0.2f, 0.2f, 0.2f, 1.f});
      parent.SetBorderColour(Gfx::Colours::White);
      parent.SetBorderWidth(2._px);
      parent.SetFlexDirection(FlexDirection::Row);
      parent.SetWrap(Wrap::Wrap);

      // === Children (will auto-wrap) ===
      for (uint j = 0; j < 10; j++)
      {
        auto childHandle = _document.CreateElement<Element>();
        _document.AppendChild(parentHandle, childHandle);
        auto &child = _document.GetByHandle<Element>(childHandle);

        child.SetWidth(80._px);
        child.SetHeight(80._px);
        child.SetMargin(10._px);
        child.SetBackgroundColour(RandomColour());
        child.SetBorderColour(Gfx::Colours::Black);
        child.SetBorderWidth(1._px);
        child.SetBorderRadius(6._px);
      }
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render(_camera);
      UI::Compositor compositor(*_context->Renderer);
      compositor.Render(_document, (float)_width, (float)_height);
    }

    void OnUpdate(double deltaTime) noexcept override
    {
      _camera.Update(deltaTime, *_context->Input);
    }
  };
}