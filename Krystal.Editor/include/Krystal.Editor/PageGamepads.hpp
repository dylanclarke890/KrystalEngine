#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadButton.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadEffectParameters.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadHapticActuator.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <format>
#include <string>

namespace krys::editor
{
  /// @brief The editor page's gamepad samples, made from C++: the arrow keys move the focus spatially within
  /// `#nav-grid` (as a pad does anywhere), and `#nav-log` shows what was pressed or chosen there;
  /// `#gamepad-readout` shows each pad the page can see (`getGamepads()`), with its pressed buttons and its
  /// axes, and `#rumble` rumbles them, `#rumble-log` saying how it went. A page without them is left alone.
  class PageGamepads
  {
    KRYS_NON_COPY_MOVABLE(PageGamepads);

  private:
    WeakPtr<boo::dom::Document> _document;
    List<boo::dom::EventListenerHandle> _listeners;
    /// @brief The animation frame the readout is shown at next, while the page can see a pad.
    int32 _frameRequest {0};

  public:
    explicit PageGamepads(boo::dom::Document &document) noexcept
        : _document(WeakPtr<boo::dom::Document>::WithRef(document))
    {
      if (!document.GetElementById(u8"nav-grid"))
      {
        return;
      }

      auto listen = [&](RefPtr<boo::dom::EventTarget> target, utf8_stringview type,
                        Func<void(boo::dom::Event &)> &&callback)
      {
        if (target)
        {
          _listeners.push_back(target->AddEventListener(boo::dom::DOMStringAtom(type), krys::move(callback)));
        }
      };

      // The grid: the arrow keys move the focus as the D-pad does, and the log shows what was pressed.
      RefPtr<boo::dom::EventTarget> grid = document.GetElementById(u8"nav-grid");
      listen(grid, u8"keydown",
             [this](boo::dom::Event &event) { MoveFocus(static_cast<boo::dom::KeyboardEvent &>(event)); });
      listen(grid, u8"click",
             [this](boo::dom::Event &event)
             {
               auto *target = TargetElement(event);
               if (target && target->LocalName() == u8"button")
               {
                 SetText(u8"nav-log",
                         std::format("Pressed “{}”", Narrow(target->TextContent().value_or(u8""))));
               }
             });
      listen(grid, u8"change",
             [this](boo::dom::Event &event)
             {
               auto *target = TargetElement(event);
               if (!target)
               {
                 return;
               }
               auto *select = DynamicDowncast<boo::html::HTMLSelectElement>(*target);
               auto value = select ? select->Value() : target->GetAttribute(u8"value").value_or(u8"");
               SetText(u8"nav-log", std::format("Chose “{}”", Narrow(value)));
             });

      // The pads: shown each frame while the page can see one (it can once a button is pressed).
      RefPtr<boo::dom::EventTarget> window = ShareRefPtr(document.Window());
      for (auto type : {u8"gamepadconnected", u8"gamepaddisconnected"})
      {
        listen(window, type, [this](boo::dom::Event &) { ShowGamepads(); });
      }
      RefPtr<boo::dom::EventTarget> rumble = document.GetElementById(u8"rumble");
      listen(rumble, u8"click", [this](boo::dom::Event &) { Rumble(); });

      ShowGamepads();
    }

    ~PageGamepads() noexcept
    {
      auto *window = _document ? _document->Window() : nullptr;
      if (window && _frameRequest)
      {
        window->CancelAnimationFrame(_frameRequest);
      }
    }

  private:
    KRYS_NODISCARD RefPtr<boo::dom::Element> ById(utf8_stringview id) const noexcept
    {
      return _document ? _document->GetElementById(id) : nullptr;
    }

    static void SetText(boo::dom::Element *element, const std::string &text) noexcept
    {
      if (element)
      {
        (void)element->TextContent(
          boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
    }

    void SetText(utf8_stringview id, const std::string &text) noexcept
    {
      SetText(ById(id).get(), text);
    }

    KRYS_NODISCARD static boo::dom::Element *TargetElement(boo::dom::Event &event) noexcept
    {
      auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
      return node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
    }

    KRYS_NODISCARD static std::string Narrow(boo::dom::DOMStringView text) noexcept
    {
      return std::string(text.begin(), text.end());
    }

    void MoveFocus(boo::dom::KeyboardEvent &event) noexcept
    {
      using FocusDirection = boo::dom::FocusDirection;
      if (!_document || event.CtrlKey() || event.AltKey() || event.MetaKey() || event.ShiftKey())
      {
        return;
      }
      auto key = event.Key();
      auto direction = key == u8"ArrowUp"      ? FocusDirection::Up
                       : key == u8"ArrowDown"  ? FocusDirection::Down
                       : key == u8"ArrowLeft"  ? FocusDirection::Left
                       : key == u8"ArrowRight" ? FocusDirection::Right
                                               : FocusDirection::None;
      if (direction != FocusDirection::None
          && boo::html::FocusAlgorithms::DirectionalNavigation(*_document, direction))
      {
        event.PreventDefault();
      }
    }

    /// @brief The pads the page can see.
    KRYS_NODISCARD List<RefPtr<boo::Gamepad>> Gamepads() const noexcept
    {
      List<RefPtr<boo::Gamepad>> gamepads;
      auto *window = _document ? _document->Window() : nullptr;
      if (!window)
      {
        return gamepads;
      }
      for (auto &gamepad : window->GetNavigator().GetGamepads())
      {
        if (gamepad)
        {
          gamepads.push_back(gamepad);
        }
      }
      return gamepads;
    }

    /// @brief Shows the pads the page can see, and keeps showing them each frame while there are any.
    void ShowGamepads() noexcept
    {
      // The standard layout's buttons, as an Xbox pad names them.
      static constexpr const char *buttonNames[] = {"A",  "B",    "X",    "Y",     "LB",   "RB",
                                                    "LT", "RT",   "View", "Menu",  "L3",   "R3",
                                                    "Up", "Down", "Left", "Right", "Guide"};
      auto gamepads = Gamepads();
      std::string text;
      for (auto &gamepad : gamepads)
      {
        text += std::format("Pad {}: {} ({})\n  Pressed:", gamepad->Index(), Narrow(gamepad->Id()),
                            gamepad->Mapping().empty() ? "no mapping" : Narrow(gamepad->Mapping()));
        bool anyPressed = false;
        auto &buttons = gamepad->Buttons();
        for (size_t i = 0; i < buttons.size(); ++i)
        {
          if (buttons[i]->Pressed())
          {
            text += std::format(" {}", i < std::size(buttonNames) ? buttonNames[i] : std::to_string(i));
            anyPressed = true;
          }
        }
        text += anyPressed ? "\n  Axes:" : " nothing\n  Axes:";
        for (auto axis : gamepad->Axes())
        {
          text += std::format(" {:.2f}", axis);
        }
        text += "\n";
      }
      if (gamepads.empty())
      {
        text = "Connect a pad and press a button on it to show it here.";
      }
      else
      {
        text.pop_back();
      }

      // Only a changed readout is set, so a still pad doesn't restyle the card each frame.
      if (auto readout = ById(u8"gamepad-readout"))
      {
        auto current = readout->TextContent().value_or(u8"");
        if (Narrow(current) != text)
        {
          SetText(readout.get(), text);
        }
      }

      auto *window = _document ? _document->Window() : nullptr;
      if (window && !gamepads.empty() && !_frameRequest)
      {
        _frameRequest = window->RequestAnimationFrame(
          [this](double)
          {
            _frameRequest = 0;
            ShowGamepads();
          });
      }
    }

    /// @brief Rumbles each pad the page can see for 600ms, both motors, and says how it went.
    void Rumble() noexcept
    {
      auto gamepads = Gamepads();
      if (gamepads.empty())
      {
        SetText(u8"rumble-log", "No pad to rumble: press a button on one first.");
        return;
      }
      SetText(u8"rumble-log", "Rumbling…");
      for (auto &gamepad : gamepads)
      {
        auto *actuator = gamepad->VibrationActuator();
        if (!actuator)
        {
          continue;
        }
        // The page may be gone by the time the effect ends, so the log is looked up again then.
        actuator->PlayEffect(
          boo::GamepadHapticEffectType::DualRumble,
          boo::GamepadEffectParameters {.Duration = 600, .StrongMagnitude = 1, .WeakMagnitude = 0.5},
          [document = _document,
           index = gamepad->Index()](boo::dom::ExceptionOr<boo::GamepadHapticsResult> &&result)
          {
            auto log = document ? document->GetElementById(u8"rumble-log") : nullptr;
            if (result.HasException())
            {
              SetText(log.get(),
                      std::format("Pad {} can't rumble: {}", index, Narrow(result.GetException().Message())));
            }
            else
            {
              SetText(
                log.get(),
                std::format("Pad {}: {}", index,
                            result.Value() == boo::GamepadHapticsResult::Complete ? "rumbled" : "cut short"));
            }
          });
      }
    }
  };
}
