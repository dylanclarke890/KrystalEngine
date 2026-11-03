#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Element;
  class Event;

  enum class SimulatedClickMouseEventOptions : uint8
  {
    SendNoEvents,
    SendMouseUpDownEvents,
  };

  enum class SimulatedClickVisualOptions : uint8
  {
    DoNotShowPressedLook,
    ShowPressedLook,
  };

  enum class SimulatedClickSource : uint8
  {
    Bindings,
    UserAgent,
  };

  /// @brief Dispatches a click at @p element, as a press of Space or Enter or `click()` does, with the press
  /// and release before it if @p mouseEventOptions asks for them.
  /// @return False if the element is a disabled form control, or is already being clicked.
  bool SimulateClick(Element &element, Event *underlyingEvent,
                     SimulatedClickMouseEventOptions mouseEventOptions,
                     SimulatedClickVisualOptions visualOptions, SimulatedClickSource source) noexcept;
}
