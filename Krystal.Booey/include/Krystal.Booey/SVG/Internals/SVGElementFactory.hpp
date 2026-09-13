#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGDefsElement.hpp"
#include "Krystal.Booey/SVG/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/SVGStyleElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::svg
{
  class SVGElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<SVGElement> TryCreate(dom::Document &document, html::TagName elementName) noexcept
    {
      switch (elementName)
      {
        case html::TagName::circle:
        {
          return CreateRef<SVGCircleElement>(document);
        }
        case html::TagName::desc:
        {
          return CreateRef<SVGDescElement>(document);
        }
        case html::TagName::svg:
        {
          return CreateRef<SVGSvgElement>(document);
        }
        case html::TagName::script:
        {
          return CreateRef<SVGScriptElement>(document);
        }
        case html::TagName::title:
        {
          return CreateRef<SVGTitleElement>(document);
        }
        case html::TagName::x:
        case html::TagName::g:
        case html::TagName::a:
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