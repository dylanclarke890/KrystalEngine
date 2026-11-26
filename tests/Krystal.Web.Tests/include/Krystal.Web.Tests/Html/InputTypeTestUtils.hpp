#pragma once

#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Window.hpp"

namespace krys::web::tests
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
