#include "Krystal.Booey/Style/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"

namespace krys::boo::style
{
  Color ToStyleColor(const css::LightDarkColor &unresolved, ColorResolutionState &state) noexcept
  {
    ColorResolutionStateNester nester {state};

    Ref<const dom::Document> document = state.Document;
    if (DocumentUseDarkAppearance(*document, &state.Style))
    {
      return ToStyleColor(unresolved.DarkColor, state);
    }
    return ToStyleColor(unresolved.LightColor, state);
  }
}
