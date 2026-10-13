#include "Krystal.Booey/CSS/Values/Color/ContrastColorResolver.hpp"
#include "Krystal.Booey/Graphics/Color/ColorLuminance.hpp"

namespace krys::boo::css
{
  // https://drafts.csswg.org/css-color-5/#contrast-color
  boo::Color Resolve(const ContrastColorResolver &resolver) noexcept
  {
    // FIXME: Implement support for a non-naive resolution.
    auto luminance = RelativeLuminance(resolver.Color);
    auto contrastWithWhite = ContrastRatio(1.0, luminance);
    auto contrastWithBlack = ContrastRatio(0.0, luminance);
    return contrastWithWhite > contrastWithBlack ? boo::Color::white : boo::Color::black;
  }
}
