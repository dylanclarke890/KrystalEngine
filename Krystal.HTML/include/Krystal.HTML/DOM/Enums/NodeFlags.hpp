#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Various flags for optimizing certain node checks.
  enum class NodeFlags : uint16
  {
    None = 0,
    IsCharacterData = 1 << 0,
    IsContainerNode = 1 << 1,
    IsShadowRoot = 1 << 2,
    IsUnknownElement = 1 << 3,
    IsHTMLElement = 1 << 4,
    IsSVGElement = 1 << 5,
    IsMathMLElement = 1 << 6,
    IsHTMLSlotElement = 1 << 7,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::NodeFlags, 9u);
