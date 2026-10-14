#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperDeprecatedCSSValueConversion.hpp"

namespace krys::boo::style
{
  Maybe<css::ToLengthConversionData>
    DeprecatedLengthConversionCreateCSSToLengthConversionData(RefPtr<dom::Element>) noexcept
  {
    // FIXME: Elements have no renderer until there's a render tree, so there's no style to resolve lengths
    // against. Once there is, this builds the conversion data from the element's renderer style, the document
    // element's renderer style, the parent renderer's style and the render view (container query units are
    // still to be investigated).
    return null;
  }
}
