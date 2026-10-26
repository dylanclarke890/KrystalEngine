#pragma once

#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief Creates the element for a tag in the SVG namespace.
  class SVGElementFactory
  {
  public:
    /// @brief The element for @p elementName, or null if SVG has no such element.
    KRYS_NODISCARD static RefPtr<SVGElement> TryCreate(dom::Document &document,
                                                       html::TagName elementName) noexcept;
  };
}
