#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML::Attributes
{
  /// @see https://html.spec.whatwg.org/#cors-settings-attribute
  enum class CORSSettingsState : uint8
  {
    NoCORS,
    Anonymous,
    UseCredentials,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::Attributes::CORSSettingsState, 3uz);