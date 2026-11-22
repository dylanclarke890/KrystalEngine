#pragma once

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Window.hpp"

namespace krys::boo::tests
{
  /// @brief Clicks @p input as a user does, with the window's transient activation that opening a picker
  /// needs.
  inline void UserClick(HTMLInputElement &input) noexcept
  {
    input.NodeDocument().Window()->NotifyActivated();
    input.Click();
  }

  /// @brief Calls `showPicker()` on @p input during a user gesture, which it needs.
  KRYS_NODISCARD inline ExceptionOr<void> ShowPicker(HTMLInputElement &input) noexcept
  {
    input.NodeDocument().Window()->NotifyActivated();
    return input.ShowPicker();
  }
}
