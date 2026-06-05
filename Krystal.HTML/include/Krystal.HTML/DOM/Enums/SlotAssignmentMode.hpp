#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#enumdef-slotassignmentmode
  enum class SlotAssignmentMode : uint8
  {
    Manual,
    Named
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::SlotAssignmentMode, 2u);