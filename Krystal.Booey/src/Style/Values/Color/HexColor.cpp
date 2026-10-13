#include "Krystal.Booey/Style/Values/Color/HexColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/HexColor.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ResolvedColor.hpp"

namespace krys::boo::style
{
  Color ToStyleColor(const css::HexColor &unresolved, ColorResolutionState &) noexcept
  {
    return {ResolvedColor {boo::Color {unresolved.Value}}};
  }
}
