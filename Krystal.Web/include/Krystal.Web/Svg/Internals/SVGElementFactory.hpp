#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Html/Types/TagName.hpp"

namespace krys::web
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
