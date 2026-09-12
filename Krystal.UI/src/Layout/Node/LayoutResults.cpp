#include <cmath>

#include "Krystal.Core/ComparisonHelpers.hpp"
#include "Krystal.UI/Layout/Node/LayoutResults.hpp"

namespace krys::UI
{
  bool LayoutResults::operator==(LayoutResults layout) const
  {
    bool isEqual =
      krys::InexactEquals(_position, layout._position) && krys::InexactEquals(_dimensions, layout._dimensions)
      && krys::InexactEquals(_margin, layout._margin) && krys::InexactEquals(_border, layout._border)
      && krys::InexactEquals(_padding, layout._padding) && _direction == layout._direction
      && _hadOverflow == layout._hadOverflow && LastOwnerDirection == layout.LastOwnerDirection
      && ConfigVersion == layout.ConfigVersion
      && NextCachedMeasurementsIndex == layout.NextCachedMeasurementsIndex
      && CachedLayout == layout.CachedLayout && ComputedFlexBasis == layout.ComputedFlexBasis;

    for (uint32_t i = 0; i < LayoutResults::MaxCachedMeasurements && isEqual; ++i)
    {
      isEqual = isEqual && CachedMeasurements[i] == layout.CachedMeasurements[i];
    }

    if (!krys::IsUndefined(_measuredDimensions[0]) || !krys::IsUndefined(layout._measuredDimensions[0]))
    {
      isEqual = isEqual && (_measuredDimensions[0] == layout._measuredDimensions[0]);
    }

    if (!krys::IsUndefined(_measuredDimensions[1]) || !krys::IsUndefined(layout._measuredDimensions[1]))
    {
      isEqual = isEqual && (_measuredDimensions[1] == layout._measuredDimensions[1]);
    }

    return isEqual;
  }
}