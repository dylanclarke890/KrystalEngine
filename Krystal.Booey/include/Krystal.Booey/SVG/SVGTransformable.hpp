#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/SVGLocatable.hpp"
#include "Krystal.Booey/SVG/SVGTransformValue.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  class SVGTransform;

  /// @brief The elements with a `transform` attribute, whose local coordinate space is their animated
  /// transform; parses the transforms of a `transform` attribute.
  class SVGTransformable : public SVGLocatable
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

    KRYS_NODISCARD AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept override
    {
      return AnimatedLocalTransform();
    }

    KRYS_NODISCARD virtual AffineTransform AnimatedLocalTransform() const noexcept = 0;
  };
}
