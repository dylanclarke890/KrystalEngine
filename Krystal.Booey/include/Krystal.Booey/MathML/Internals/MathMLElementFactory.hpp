#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::mathml
{
  class MathMLElement;

  class MathMLElementFactory
  {
  public:
    /// @brief Every element in the MathML namespace is a `MathMLElement`.
    KRYS_NODISCARD static RefPtr<MathMLElement> TryCreate(dom::Document &document,
                                                          const dom::QualifiedName &name) noexcept;
  };
}