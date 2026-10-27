#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::svg
{
  SVGDocumentExtensions::SVGDocumentExtensions(dom::Document &document) noexcept
      : _document(CreateWeakPtr(&document)), _resourcesCache(CreateUnique<SVGResourcesCache>())
  {
  }

  SVGDocumentExtensions::~SVGDocumentExtensions() noexcept = default;

  void SVGDocumentExtensions::AddElementToRebuild(SVGElement &element) noexcept
  {
    _rebuildElements.push_back(ShareRef(element));
  }

  void SVGDocumentExtensions::RemoveElementToRebuild(SVGElement &element) noexcept
  {
    RemoveFirstMatching(_rebuildElements, [&](auto &item) { return item.get() == &element; });
  }

  void SVGDocumentExtensions::RebuildElements() noexcept
  {
    auto shadowRebuildElements = std::exchange(_rebuildElements, {});
    for (auto &element : shadowRebuildElements)
    {
      element->SVGAttributeChanged(SVGNames::HrefAttr());
    }
  }

  void SVGDocumentExtensions::ClearTargetDependencies(SVGElement &referencedElement) noexcept
  {
    for (auto &element : referencedElement.ReferencingElements())
    {
      _rebuildElements.push_back(element);
      element->CallClearTarget();
    }
  }

  void SVGDocumentExtensions::RebuildAllElementReferencesForTarget(SVGElement &referencedElement) noexcept
  {
    for (auto &element : referencedElement.ReferencingElements())
    {
      element->SVGAttributeChanged(SVGNames::HrefAttr());
    }
  }
}
