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
    KRYS_NODISCARD static RefPtr<MathMLElement> TryCreate(dom::Document &document,
                                                          const dom::QualifiedName &name) noexcept
    {
      // TODO(impl): MATHML-ELEMENTS
      (void)document;
      (void)name;
      return nullptr;
    }
  };
}