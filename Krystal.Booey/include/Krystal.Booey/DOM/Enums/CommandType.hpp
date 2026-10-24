#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief The state of a button's `command` attribute: the command it sends its `commandfor` element.
  /// @see https://html.spec.whatwg.org/#attr-button-command
  enum class CommandType : uint8
  {
    Invalid,

    /// @brief A command starting with `--`, which only the element's `command` listeners carry out.
    Custom,

    TogglePopover,
    HidePopover,
    ShowPopover,

    ShowModal,
    Close,
    RequestClose
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::CommandType, 8uz);
