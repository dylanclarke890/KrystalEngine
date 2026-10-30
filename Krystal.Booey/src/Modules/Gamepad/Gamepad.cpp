#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadButton.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadHapticActuator.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Platform/Gamepad/PlatformGamepad.hpp"

namespace krys::boo
{
  Gamepad::Gamepad(dom::Document *document, const PlatformGamepad &platformGamepad) noexcept
      : _document(CreateWeakPtr(document)), _id(platformGamepad.Id()), _index(platformGamepad.Index()),
        _connected(true), _timestamp(platformGamepad.LastUpdateTime()), _mapping(platformGamepad.Mapping()),
        _supportedEffectTypes(platformGamepad.SupportedEffectTypes()),
        _axes(platformGamepad.AxisValues().size(), 0.0)
  {
    if (platformGamepad.SupportedEffectTypes().Contains(GamepadHapticEffectType::DualRumble))
    {
      _vibrationActuator =
        GamepadHapticActuator::Create(document, GamepadHapticActuator::Type::DualRumble, *this);
    }

    auto buttonCount = platformGamepad.ButtonValues().size();
    _buttons.reserve(buttonCount);
    for (size_t i = 0; i < buttonCount; ++i)
    {
      _buttons.push_back(GamepadButton::Create());
    }
  }

  Gamepad::~Gamepad() noexcept = default;

  double Gamepad::Timestamp() const noexcept
  {
    auto *document = _document.get();
    auto *window = document ? document->Window() : nullptr;
    if (!window)
    {
      return 0;
    }
    return std::chrono::duration<double, std::milli>(_timestamp - window->TimeOrigin()).count();
  }

  GamepadHapticActuator *Gamepad::VibrationActuator() const noexcept
  {
    return _vibrationActuator.get();
  }

  void Gamepad::UpdateFromPlatformGamepad(const PlatformGamepad &platformGamepad) noexcept
  {
    for (size_t i = 0; i < _axes.size(); ++i)
    {
      _axes[i] = platformGamepad.AxisValues()[i];
    }
    for (size_t i = 0; i < _buttons.size(); ++i)
    {
      _buttons[i]->SetValue(platformGamepad.ButtonValues()[i]);
    }
    _timestamp = platformGamepad.LastUpdateTime();
  }
}
