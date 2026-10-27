#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"

namespace krys::boo::svg
{
  SVGDocumentExtensions::SVGDocumentExtensions(dom::Document &document) noexcept
      : _document(CreateWeakPtr(&document)), _resourcesCache(CreateUnique<SVGResourcesCache>())
  {
  }

  SVGDocumentExtensions::~SVGDocumentExtensions() noexcept = default;
}
