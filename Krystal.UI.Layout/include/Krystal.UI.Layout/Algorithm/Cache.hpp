#pragma once

#include "Krystal.UI.Layout/Config/Config.hpp"
#include "Krystal.UI.Styles/Enums/SizingMode.hpp"

namespace Krys::UI::Layout
{
  bool CanUseCachedMeasurement(Styles::SizingMode widthMode, float availableWidth,
                               Styles::SizingMode heightMode, float availableHeight,
                               Styles::SizingMode lastWidthMode, float lastAvailableWidth,
                               Styles::SizingMode lastHeightMode, float lastAvailableHeight,
                               float lastComputedWidth, float lastComputedHeight, float marginRow,
                               float marginColumn, const Config *config);
}