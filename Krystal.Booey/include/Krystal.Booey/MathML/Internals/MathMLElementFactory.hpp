#pragma once

#include "Krystal.Booey/Html/Types/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class Document;
  class MathMLElement;

  /// @brief Creates the element for a tag in the MathML namespace.
  class MathMLElementFactory
  {
  public:
    /// @brief The element for @p elementName, or null if MathML Core has no such element.
    KRYS_NODISCARD static RefPtr<MathMLElement> TryCreate(Document &document,
                                                          html::TagName elementName) noexcept;
  };
}
