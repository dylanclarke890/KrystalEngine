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
  class SVGSvgElement;

  /// @brief What a document keeps for its SVG content: its resources, the elements whose references wait to
  /// be rebuilt, and its `<svg>` elements' time containers.
  /// FIXME: reportWarning and reportError, once there's a console.
  class SVGDocumentExtensions final
  {
    KRYS_NON_COPY_MOVABLE(SVGDocumentExtensions);

  private:
    WeakPtr<dom::Document> _document;
    UniquePtr<SVGResourcesCache> _resourcesCache;
    List<Ref<SVGElement>> _rebuildElements;
    // For SVG 1.2 support this will need to be made more general.
    List<WeakPtr<SVGSvgElement>> _timeContainers;
    bool _areAnimationsPaused;

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

    void AddTimeContainer(SVGSvgElement &) noexcept;
    void RemoveTimeContainer(SVGSvgElement &) noexcept;
    KRYS_NODISCARD List<Ref<SVGSvgElement>> TimeContainers() const noexcept;

    void StartAnimations() noexcept;
    void PauseAnimations() noexcept;
    void UnpauseAnimations() noexcept;
    void DispatchLoadEventToOutermostSVGElements() noexcept;

    KRYS_NODISCARD bool AreAnimationsPaused() const noexcept
    {
      return _areAnimationsPaused;
    }

    void ClearTargetDependencies(SVGElement &referencedElement) noexcept;
    void RebuildAllElementReferencesForTarget(SVGElement &referencedElement) noexcept;
  };
}
