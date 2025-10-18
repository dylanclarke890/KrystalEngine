#pragma once

#include "Krystal.Engine/Application.hpp"
#include "Krystal.Gfx/Cameras/FirstPersonCamera.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/Keys.hpp"

namespace Krys
{
  class Editor : public Application
  {
  private:
    Gfx::FirstPersonCamera _camera;
    UI::Document _document;

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings),
          _camera(Maths::Vec3(0.0f, 0.0f, 5.0f), Maths::Vec3(0.0f, 0.0f, 0.0f), Maths::Vec3(0.0f, 1.0f, 0.0f),
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
          _context->GraphicsContext->Resize(event.Width(), event.Height());
          _camera.OnResize(event.Width(), event.Height());
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

      auto testBoxHandle = _document.CreateElement<Element>();
      auto testBoxHandle2 = _document.CreateElement<Element>();
      auto testBoxHandle3 = _document.CreateElement<Element>();
      _document.Add(testBoxHandle);
      _document.Add(testBoxHandle2);
      _document.Add(testBoxHandle3);

      _document.GetBody().SetBackgroundColour(Gfx::Colours::White);
      _document.GetBody().SetPadding(5);
      _document.GetBody().SetFlexWrap(UI::FlexWrap::Wrap);

      auto &testBox = _document.GetByHandle<Element>(testBoxHandle);
      testBox.SetWidth(200.f);
      testBox.SetHeight(200.f);
      testBox.SetMargin(10);
      testBox.SetBackgroundColour(Gfx::Colours::Blue);

      auto &testBox2 = _document.GetByHandle<Element>(testBoxHandle2);
      testBox2.SetWidth(300.f);
      testBox2.SetHeight(300.f);
      testBox2.SetBackgroundColour(Gfx::Colours::Red);

      auto &testBox3 = _document.GetByHandle<Element>(testBoxHandle3);
      testBox3.SetWidth(300.f);
      testBox3.SetHeight(300.f);
      testBox3.SetBackgroundColour(Gfx::Colours::Green);
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render(_camera);

      _document.Layout((float)_context->Window->GetSize().Width, (float)_context->Window->GetSize().Height);
      _context->GraphicsContext->Render(_document);
    }

    void OnUpdate(double deltaTime) noexcept override
    {
      _camera.Update(deltaTime, *_context->Input);
    }
  };
}