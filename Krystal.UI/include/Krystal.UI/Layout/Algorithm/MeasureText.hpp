#pragma once

#include "Krystal.UI/Layout/Api/Forward.hpp"

namespace Krys::UI
{
  Size TextMeasureFunc(NodeConstRef node, float width, MeasureMode widthMode, float height,
                       MeasureMode heightMode);
}