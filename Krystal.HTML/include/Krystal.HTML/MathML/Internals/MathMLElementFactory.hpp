#pragma once

#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
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