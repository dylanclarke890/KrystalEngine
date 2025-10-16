#include <cmath>

#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.UI/Node/LayoutResults.hpp"

namespace Krys::UI
{
  bool LayoutResults::operator==(LayoutResults layout) const
  {
    bool isEqual =
      Krys::InexactEquals(_position, layout._position) && Krys::InexactEquals(_dimensions, layout._dimensions)
      && Krys::InexactEquals(_margin, layout._margin) && Krys::InexactEquals(_border, layout._border)
      && Krys::InexactEquals(_padding, layout._padding) && _direction == layout._direction
      && _hadOverflow == layout._hadOverflow && LastOwnerDirection == layout.LastOwnerDirection
      && ConfigVersion == layout.ConfigVersion
      && NextCachedMeasurementsIndex == layout.NextCachedMeasurementsIndex
      && CachedLayout == layout.CachedLayout && ComputedFlexBasis == layout.ComputedFlexBasis;

    for (uint32_t i = 0; i < LayoutResults::MaxCachedMeasurements && isEqual; ++i)
    {
      isEqual = isEqual && CachedMeasurements[i] == layout.CachedMeasurements[i];
    }

    if (!Krys::IsUndefined(_measuredDimensions[0]) || !Krys::IsUndefined(layout._measuredDimensions[0]))
    {
      isEqual = isEqual && (_measuredDimensions[0] == layout._measuredDimensions[0]);
    }

    if (!Krys::IsUndefined(_measuredDimensions[1]) || !Krys::IsUndefined(layout._measuredDimensions[1]))
    {
      isEqual = isEqual && (_measuredDimensions[1] == layout._measuredDimensions[1]);
    }

    return isEqual;
  }
}