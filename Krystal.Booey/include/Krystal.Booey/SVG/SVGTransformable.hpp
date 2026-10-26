#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/SVGTransformValue.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  class SVGTransform;

  /// @brief Parses the transforms of a `transform` attribute.
  // FIXME: The SVGLocatable base and the animated local transform come with SVGGraphicsElement (B11a3).
  class SVGTransformable
  {
  public:
    virtual ~SVGTransformable() noexcept;

    static bool ParseAndReplaceTransform(SVGTransformValue::SVGTransformType, dom::DOMStringView &buffer,
                                         SVGTransform &) noexcept;
    KRYS_NODISCARD static RefPtr<SVGTransform> ParseTransform(SVGTransformValue::SVGTransformType,
                                                              dom::DOMStringView &buffer) noexcept;

    KRYS_NODISCARD static Maybe<SVGTransformValue::SVGTransformType>
      ParseTransformTypeFromString(dom::DOMStringView) noexcept;
    KRYS_NODISCARD static Maybe<SVGTransformValue::SVGTransformType>
      ParseTransformType(dom::DOMStringView &buffer) noexcept;
  };
}
