#pragma once

#include "Krystal.UI.Layout/Config/Config.hpp"
#include "Krystal.UI.Layout/Enums/SizingMode.hpp"

namespace Krys::UI::Layout
{
  bool CanUseCachedMeasurement(SizingMode widthMode, float availableWidth, SizingMode heightMode,
                               float availableHeight, SizingMode lastWidthMode, float lastAvailableWidth,
                               SizingMode lastHeightMode, float lastAvailableHeight, float lastComputedWidth,
                               float lastComputedHeight, float marginRow, float marginColumn,
                               const Config *config);
}