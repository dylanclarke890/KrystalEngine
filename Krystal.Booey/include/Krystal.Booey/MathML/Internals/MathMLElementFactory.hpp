#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::mathml
{
  class CustomElementRegistry;
  class Document;
  class MathMLElement;

  class MathMLElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<MathMLElement> TryCreate(Document &document,
                                                          const QualifiedName &name) noexcept
    {
      // TODO(impl): MATHML-ELEMENTS
      return nullptr;
    }
  };
}