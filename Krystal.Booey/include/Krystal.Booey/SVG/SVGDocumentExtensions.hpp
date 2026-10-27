#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
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
  class SVGElement;

  /// @brief What a document keeps for its SVG content.
  /// FIXME: The time containers and animations (addTimeContainer, startAnimations, pauseAnimations,
  /// unpauseAnimations, dispatchLoadEventToOutermostSVGElements), with SMIL (B11i); reportWarning and
  /// reportError.
  class SVGDocumentExtensions final
  {
    KRYS_NON_COPY_MOVABLE(SVGDocumentExtensions);

  private:
    WeakPtr<dom::Document> _document;
    UniquePtr<SVGResourcesCache> _resourcesCache;
    List<Ref<SVGElement>> _rebuildElements;

  public:
    explicit SVGDocumentExtensions(dom::Document &) noexcept;
    ~SVGDocumentExtensions() noexcept;

    KRYS_NODISCARD SVGResourcesCache &ResourcesCache() noexcept
    {
      return *_resourcesCache;
    }

    /// @brief Remembers @p element, whose reference target was removed, to look its target up again after the
    /// tree mutation.
    void AddElementToRebuild(SVGElement &element) noexcept;
    void RemoveElementToRebuild(SVGElement &element) noexcept;
    void RebuildElements() noexcept;

    void ClearTargetDependencies(SVGElement &referencedElement) noexcept;
    void RebuildAllElementReferencesForTarget(SVGElement &referencedElement) noexcept;
  };
}
