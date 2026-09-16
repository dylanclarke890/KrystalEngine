#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html::Attributes
{
  /// @see https://html.spec.whatwg.org/#cors-settings-attribute
  enum class CORSSettingsState : uint8
  {
    NoCORS,
    Anonymous,
    UseCredentials,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::Attributes::CORSSettingsState, 3uz);