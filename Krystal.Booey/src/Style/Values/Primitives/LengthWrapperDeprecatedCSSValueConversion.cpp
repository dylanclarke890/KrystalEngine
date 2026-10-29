#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperDeprecatedCSSValueConversion.hpp"

namespace krys::boo::style
{
  Maybe<css::ToLengthConversionData>
    DeprecatedLengthConversionCreateCSSToLengthConversionData(RefPtr<dom::Element>) noexcept
  {
    // FIXME: Build the conversion data from the element's renderer style, the document element's renderer
    // style, the parent renderer's style and the render view (container query units are still to be
    // investigated); until then lengths aren't resolved.
    return null;
  }
}
