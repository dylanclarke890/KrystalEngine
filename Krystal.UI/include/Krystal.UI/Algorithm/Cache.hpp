#pragma once

#include "Krystal.UI/Config/Config.hpp"
#include "Krystal.UI/Enums/SizingMode.hpp"

namespace Krys::UI
{
  bool CanUseCachedMeasurement(SizingMode widthMode, float availableWidth, SizingMode heightMode,
                               float availableHeight, SizingMode lastWidthMode, float lastAvailableWidth,
                               SizingMode lastHeightMode, float lastAvailableHeight, float lastComputedWidth,
                               float lastComputedHeight, float marginRow, float marginColumn,
                               const Config *config);
}