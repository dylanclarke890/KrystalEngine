#pragma once

#include <cmath>

#include "Krystal.Core/ComparisonHelpers.hpp"
#include "Krystal.UI/Layout/Algorithm/SizingMode.hpp"
#include "Krystal.UI/Styles/Enums/SizingMode.hpp"

namespace krys::UI
{
  struct CachedMeasurement
  {
    float AvailableWidth {-1};
    float AvailableHeight {-1};
    SizingMode WidthSizingMode {SizingMode::MaxContent};
    SizingMode HeightSizingMode {SizingMode::MaxContent};

    float ComputedWidth {-1};
    float ComputedHeight {-1};

    bool operator==(CachedMeasurement measurement) const
    {
      bool isEqual =
        WidthSizingMode == measurement.WidthSizingMode && HeightSizingMode == measurement.HeightSizingMode;

      if (!krys::IsUndefined(AvailableWidth) || !krys::IsUndefined(measurement.AvailableWidth))
      {
        isEqual = isEqual && AvailableWidth == measurement.AvailableWidth;
      }

      if (!krys::IsUndefined(AvailableHeight) || !krys::IsUndefined(measurement.AvailableHeight))
      {
        isEqual = isEqual && AvailableHeight == measurement.AvailableHeight;
      }

      if (!krys::IsUndefined(ComputedWidth) || !krys::IsUndefined(measurement.ComputedWidth))
      {
        isEqual = isEqual && ComputedWidth == measurement.ComputedWidth;
      }

      if (!krys::IsUndefined(ComputedHeight) || !krys::IsUndefined(measurement.ComputedHeight))
      {
        isEqual = isEqual && ComputedHeight == measurement.ComputedHeight;
      }

      return isEqual;
    }
  };
}