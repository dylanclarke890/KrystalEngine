#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class SVGResourcesCache;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::svg
{
  /// @brief What a document keeps for its SVG content.
  /// FIXME: The time containers and animations (addTimeContainer, startAnimations, pauseAnimations,
  /// unpauseAnimations, dispatchLoadEventToOutermostSVGElements), with SMIL (B11i); the elements to rebuild
  /// (addElementToRebuild, rebuildAllElementReferencesForTarget), with the first element that registers as
  /// referencing another (<textPath>, B11g); reportWarning and reportError.
  class SVGDocumentExtensions final
  {
    KRYS_NON_COPY_MOVABLE(SVGDocumentExtensions);

  private:
    WeakPtr<dom::Document> _document;
    UniquePtr<SVGResourcesCache> _resourcesCache;

  public:
    explicit SVGDocumentExtensions(dom::Document &) noexcept;
    ~SVGDocumentExtensions() noexcept;

    KRYS_NODISCARD SVGResourcesCache &ResourcesCache() noexcept
    {
      return *_resourcesCache;
    }
  };
}
