#include "Krystal.Booey/Style/Values/Grid/GridTrackSizes.hpp"

#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo::style
{
  const GridTrackSize &GridTrackSizeDefaulter::operator()() const noexcept
  {
    static NeverDestroyed<GridTrackSize> staticValue {css::keywords::Auto {}};
    return staticValue.get();
  }
}
