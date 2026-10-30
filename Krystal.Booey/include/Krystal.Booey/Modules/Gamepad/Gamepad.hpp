#pragma once

#include "Krystal.Booey/Modules/Gamepad/GamepadHapticEffectType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class GamepadButton;
  class GamepadHapticActuator;
  class PlatformGamepad;

  /// @brief A gamepad as a document sees it: a snapshot of its buttons and axes, updated each time
  /// `getGamepads()` is called.
  /// @see https://w3c.github.io/gamepad/#gamepad-interface
  class Gamepad final : public RefCounted<Gamepad>, public CanMakeWeakPtr<Gamepad>
  {
    KRYS_NON_COPY_MOVABLE(Gamepad);

  private:
    WeakPtr<dom::Document> _document;
    utf8_string _id;
    uint32 _index;
    bool _connected;
    MonotonicTime::duration _timestamp;
    utf8_string _mapping;
    GamepadHapticEffectTypeSet _supportedEffectTypes;
    List<double> _axes;
    List<Ref<GamepadButton>> _buttons;
    RefPtr<GamepadHapticActuator> _vibrationActuator;

  public:
    Gamepad(dom::Document *document, const PlatformGamepad &platformGamepad) noexcept;
    ~Gamepad() noexcept;

    KRYS_NODISCARD static Ref<Gamepad> Create(dom::Document *document,
                                              const PlatformGamepad &platformGamepad) noexcept
    {
      return CreateRef<Gamepad>(document, platformGamepad);
    }

    /// @brief Says what kind of pad it is.
    /// @see https://w3c.github.io/gamepad/#dom-gamepad-id
    KRYS_NODISCARD const utf8_string &Id() const noexcept
    {
      return _id;
    }

    /// @see https://w3c.github.io/gamepad/#dom-gamepad-index
    KRYS_NODISCARD uint32 Index() const noexcept
    {
      return _index;
    }

    /// @see https://w3c.github.io/gamepad/#dom-gamepad-mapping
    KRYS_NODISCARD const utf8_string &Mapping() const noexcept
    {
      return _mapping;
    }

    /// @see https://w3c.github.io/gamepad/#dom-gamepad-connected
    KRYS_NODISCARD bool Connected() const noexcept
    {
      return _connected;
    }

    /// @brief When its data last changed, in milliseconds since the time origin of its document's window.
    /// @see https://w3c.github.io/gamepad/#dom-gamepad-timestamp
    KRYS_NODISCARD double Timestamp() const noexcept;

    /// @brief Each axis's value, from -1 to 1, with right and down positive.
    /// @see https://w3c.github.io/gamepad/#dom-gamepad-axes
    KRYS_NODISCARD const List<double> &Axes() const noexcept
    {
      return _axes;
    }

    /// @see https://w3c.github.io/gamepad/#dom-gamepad-buttons
    KRYS_NODISCARD const List<Ref<GamepadButton>> &Buttons() const noexcept
    {
      return _buttons;
    }

    KRYS_NODISCARD const GamepadHapticEffectTypeSet &SupportedEffectTypes() const noexcept
    {
      return _supportedEffectTypes;
    }

    /// @brief Takes @p platformGamepad's current values.
    void UpdateFromPlatformGamepad(const PlatformGamepad &platformGamepad) noexcept;

    void SetConnected(bool connected) noexcept
    {
      _connected = connected;
    }

    /// @brief Its rumble motors, if it has any.
    /// @see https://w3c.github.io/gamepad/#dom-gamepad-vibrationactuator
    KRYS_NODISCARD GamepadHapticActuator *VibrationActuator() const noexcept;
  };
}
