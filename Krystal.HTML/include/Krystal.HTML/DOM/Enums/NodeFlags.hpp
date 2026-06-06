#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Various flags for optimizing certain node checks.
  enum class NodeFlags : uint16
  {
    None = 0,
    IsConnected = 1 << 1,
    IsInShadowTree = 1 << 2,
    IsCharacterData = 1 << 3,
    IsContainerNode = 1 << 4,
    IsShadowRoot = 1 << 5,
    IsUnknownElement = 1 << 6,
    IsHTMLElement = 1 << 7,
    IsSVGElement = 1 << 8,
    IsMathMLElement = 1 << 9,
    IsHTMLSlotElement = 1 << 10,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::NodeFlags, 10u);
