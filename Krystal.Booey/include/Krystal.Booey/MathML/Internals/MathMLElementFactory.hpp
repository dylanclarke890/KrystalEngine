#pragma once

#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::mathml
{
  class MathMLElement;

  /// @brief Creates the element for a tag in the MathML namespace.
  class MathMLElementFactory
  {
  public:
    /// @brief The element for @p elementName, or null if MathML Core has no such element.
    KRYS_NODISCARD static RefPtr<MathMLElement> TryCreate(dom::Document &document,
                                                          html::TagName elementName) noexcept;
  };
}
