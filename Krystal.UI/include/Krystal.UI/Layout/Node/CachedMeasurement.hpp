#pragma once

#include <cmath>

#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.UI/Layout/Algorithm/SizingMode.hpp"
#include "Krystal.UI/Styles/Enums/SizingMode.hpp"

namespace Krys::UI
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

      if (!Krys::IsUndefined(AvailableWidth) || !Krys::IsUndefined(measurement.AvailableWidth))
      {
        isEqual = isEqual && AvailableWidth == measurement.AvailableWidth;
      }

      if (!Krys::IsUndefined(AvailableHeight) || !Krys::IsUndefined(measurement.AvailableHeight))
      {
        isEqual = isEqual && AvailableHeight == measurement.AvailableHeight;
      }

      if (!Krys::IsUndefined(ComputedWidth) || !Krys::IsUndefined(measurement.ComputedWidth))
      {
        isEqual = isEqual && ComputedWidth == measurement.ComputedWidth;
      }

      if (!Krys::IsUndefined(ComputedHeight) || !Krys::IsUndefined(measurement.ComputedHeight))
      {
        isEqual = isEqual && ComputedHeight == measurement.ComputedHeight;
      }

      return isEqual;
    }
  };
}