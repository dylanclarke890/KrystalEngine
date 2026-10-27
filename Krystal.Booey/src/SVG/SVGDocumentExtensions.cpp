#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTimeContainer.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>

namespace krys::boo::svg
{
  namespace
  {
    // FIXME: Also when the page isn't visible or image animation is off, once the page knows about those.
    KRYS_NODISCARD bool AnimationsPausedForDocument(dom::Document &document) noexcept
    {
      return !document.GetPage();
    }
  }

  SVGDocumentExtensions::SVGDocumentExtensions(dom::Document &document) noexcept
      : _document(CreateWeakPtr(&document)), _resourcesCache(CreateUnique<SVGResourcesCache>()),
        _areAnimationsPaused(AnimationsPausedForDocument(document))
  {
  }

  void SVGDocumentExtensions::AddTimeContainer(SVGSvgElement &element) noexcept
  {
    std::erase_if(_timeContainers, [](const auto &container) { return !container; });
    if (std::ranges::none_of(_timeContainers,
                             [&](const auto &container) { return container.get() == &element; }))
    {
      _timeContainers.push_back(CreateWeakPtr(&element));
    }
    if (_areAnimationsPaused)
    {
      element.PauseAnimations();
    }
  }

  void SVGDocumentExtensions::RemoveTimeContainer(SVGSvgElement &element) noexcept
  {
    std::erase_if(_timeContainers,
                  [&](const auto &container) { return !container || container.get() == &element; });
  }

  List<Ref<SVGSvgElement>> SVGDocumentExtensions::TimeContainers() const noexcept
  {
    List<Ref<SVGSvgElement>> containers;
    for (const auto &container : _timeContainers)
    {
      if (container)
      {
        containers.push_back(ShareRef(*container));
      }
    }
    return containers;
  }

  void SVGDocumentExtensions::StartAnimations() noexcept
  {
    // FIXME: Eventually every "Time Container" will need a way to latch on to some global timer
    // starting animations for a document will do this "latching"
    // FIXME: We hold a ref pointers to prevent a shadow tree from getting removed out from underneath us.
    // In the future we should refactor the use-element to avoid this. See https://webkit.org/b/53704
    for (auto &element : TimeContainers())
    {
      element->TimeContainer().Begin();
    }
  }

  void SVGDocumentExtensions::PauseAnimations() noexcept
  {
    for (auto &container : TimeContainers())
    {
      container->PauseAnimations();
    }
    _areAnimationsPaused = true;
  }

  void SVGDocumentExtensions::UnpauseAnimations() noexcept
  {
    // If animations are paused at the document level, don't allow `this` to be unpaused.
    if (RefPtr document = ShareRefPtr(_document.get()); !document || AnimationsPausedForDocument(*document))
    {
      return;
    }

    for (auto &container : TimeContainers())
    {
      container->UnpauseAnimations();
    }
    _areAnimationsPaused = false;
  }

  void SVGDocumentExtensions::DispatchLoadEventToOutermostSVGElements() noexcept
  {
    for (auto &container : TimeContainers())
    {
      if (!container->IsOutermostSVGSVGElement())
      {
        continue;
      }
      container->SendLoadEventIfPossible();
    }
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
