#pragma once

#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/SVG/SVGElement.hpp"
#include "Krystal.HTML/SVG/SVGScriptElement.hpp"
#include "Krystal.HTML/SVG/SVGSvgElement.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Document;

  class SVGElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<SVGElement> TryCreate(Document &document, TagName elementName) noexcept
    {
      switch (elementName)
      {
        case TagName::svg:
        {
          return CreateRef<SVGSvgElement>(document);
        }
        case TagName::script:
        {
          return CreateRef<SVGScriptElement>(document);
        }
      }

      // TODO(impl): SVG-ELEMENTS
      return nullptr;
    }
  };
}