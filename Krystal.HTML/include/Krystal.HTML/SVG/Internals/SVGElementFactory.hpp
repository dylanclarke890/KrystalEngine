#pragma once

#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/SVG/Enums/SVGTagName.hpp"
#include "Krystal.HTML/SVG/SVGElement.hpp"
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
    KRYS_NODISCARD static RefPtr<SVGElement> TryCreate(Document &document, TagName elementName) noexcept
    {
      switch (elementName)
      {
        case TagName::svg:
        {
          return AdoptRefPtr(new SVGElement(document, SVGTagName::svg));
        }
      }

      // TODO(impl): SVG-ELEMENTS
      return nullptr;
    }
  };
}