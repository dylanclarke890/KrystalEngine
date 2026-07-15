#pragma once

#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.HTML/SVG/SVGCircleElement.hpp"
#include "Krystal.HTML/SVG/SVGDefsElement.hpp"
#include "Krystal.HTML/SVG/SVGDescElement.hpp"
#include "Krystal.HTML/SVG/SVGElement.hpp"
#include "Krystal.HTML/SVG/SVGEllipseElement.hpp"
#include "Krystal.HTML/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.HTML/SVG/SVGGeometryElement.hpp"
#include "Krystal.HTML/SVG/SVGGraphicsElement.hpp"
#include "Krystal.HTML/SVG/SVGImageElement.hpp"
#include "Krystal.HTML/SVG/SVGLineElement.hpp"
#include "Krystal.HTML/SVG/SVGMetadataElement.hpp"
#include "Krystal.HTML/SVG/SVGPolygonElement.hpp"
#include "Krystal.HTML/SVG/SVGPolylineElement.hpp"
#include "Krystal.HTML/SVG/SVGRectElement.hpp"
#include "Krystal.HTML/SVG/SVGScriptElement.hpp"
#include "Krystal.HTML/SVG/SVGStyleElement.hpp"
#include "Krystal.HTML/SVG/SVGSvgElement.hpp"
#include "Krystal.HTML/SVG/SVGSwitchElement.hpp"
#include "Krystal.HTML/SVG/SVGSymbolElement.hpp"
#include "Krystal.HTML/SVG/SVGTitleElement.hpp"
#include "Krystal.HTML/SVG/SVGUnknownElement.hpp"
#include "Krystal.HTML/SVG/SVGUseElement.hpp"
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
        case TagName::circle:
        {
          return CreateRef<SVGCircleElement>(document);
        }
        case TagName::desc:
        {
          return CreateRef<SVGDescElement>(document);
        }
        case TagName::svg:
        {
          return CreateRef<SVGSvgElement>(document);
        }
        case TagName::script:
        {
          return CreateRef<SVGScriptElement>(document);
        }
        case TagName::title:
        {
          return CreateRef<SVGTitleElement>(document);
        }
        case TagName::x:
        case TagName::g:
        case TagName::a:
        {
          // TODO: return correct element
          return CreateRef<SVGElement>(document);
        }
      }

      // TODO(impl): SVG-ELEMENTS
      return nullptr;
    }
  };
}