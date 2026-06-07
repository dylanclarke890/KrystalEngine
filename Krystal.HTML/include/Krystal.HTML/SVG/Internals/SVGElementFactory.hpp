#pragma once

#include "Krystal.HTML/Types/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Document;
  class SVGElement;

  class SVGElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<SVGElement> TryCreate(Document &document, const QualifiedName &name) noexcept
    {
      // TODO(impl): SVG-ELEMENTS
      return nullptr;
    }
  };
}