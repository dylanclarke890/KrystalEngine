#pragma once

#include "Krystal.Booey/HTML/Types/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class Document;
  class SVGElement;

  /// @brief Creates the element for a tag in the SVG namespace.
  class SVGElementFactory
  {
  public:
    /// @brief The element for @p elementName, or null if SVG has no such element.
    KRYS_NODISCARD static RefPtr<SVGElement> TryCreate(Document &document,
                                                       html::TagName elementName) noexcept;
  };
}
