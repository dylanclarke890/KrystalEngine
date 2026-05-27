#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class LiveCollectionFilter : uint8
  {
    All,
    Children,
    TagName,
  };
}