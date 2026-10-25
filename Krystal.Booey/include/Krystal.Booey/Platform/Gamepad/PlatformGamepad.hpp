#pragma once

#include "Krystal.Booey/Modules/Gamepad/GamepadHapticEffectType.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  struct GamepadEffectParameters;

  /// @brief A gamepad as the platform sees it: its identity, and its axes' and buttons' current values.
  class PlatformGamepad : public CanMakeWeakPtr<PlatformGamepad>, public CanMakeCheckedPtr<PlatformGamepad>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(PlatformGamepad);

  protected:
    utf8_string _id;
    utf8_string _mapping;
    uint32 _index;
    MonotonicTime::duration _lastUpdateTime {};
    MonotonicTime::duration _connectTime {};
    GamepadHapticEffectTypeSet _supportedEffectTypes;

  public:
    virtual ~PlatformGamepad() noexcept = default;

    /// @brief Says what kind of pad it is.
    KRYS_NODISCARD const utf8_string &Id() const noexcept
    {
      return _id;
    }

    /// @brief The layout its buttons and axes follow: `standard` (`StandardGamepadMapping`), or empty for
    /// none.
    KRYS_NODISCARD const utf8_string &Mapping() const noexcept
    {
      return _mapping;
    }

    /// @brief Its place among the connected pads.
    KRYS_NODISCARD uint32 Index() const noexcept
    {
      return _index;
    }

    /// @brief When its values last changed.
    KRYS_NODISCARD virtual MonotonicTime::duration LastUpdateTime() const noexcept
    {
      return _lastUpdateTime;
    }

    /// @brief When it connected.
    KRYS_NODISCARD MonotonicTime::duration ConnectTime() const noexcept
    {
      return _connectTime;
    }

    KRYS_NODISCARD const GamepadHapticEffectTypeSet &SupportedEffectTypes() const noexcept
    {
      return _supportedEffectTypes;
    }

    /// @brief Each axis's value, from -1 to 1.
    KRYS_NODISCARD virtual const List<double> &AxisValues() const noexcept = 0;
    /// @brief Each button's value, from 0 (released) to 1 (fully pressed).
    KRYS_NODISCARD virtual const List<double> &ButtonValues() const noexcept = 0;

    /// @brief Plays @p type's effect, calling @p completionHandler with whether it played to the end.
    virtual void PlayEffect(GamepadHapticEffectType type, const GamepadEffectParameters &parameters,
                            Func<void(bool)> &&completionHandler) noexcept
    {
      (void)type;
      (void)parameters;
      completionHandler(false);
    }

    /// @brief Stops its effects, then calls @p completionHandler.
    virtual void StopEffects(Func<void()> &&completionHandler) noexcept
    {
      completionHandler();
    }

    /// @brief Where it comes from, for logging.
    KRYS_NODISCARD virtual utf8_stringview Source() const noexcept
    {
      return u8"Unknown";
    }

  protected:
    explicit PlatformGamepad(uint32 index) noexcept : _index(index)
    {
    }
  };
}
