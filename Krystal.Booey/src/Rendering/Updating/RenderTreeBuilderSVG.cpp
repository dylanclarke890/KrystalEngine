#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderSVG.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderInline.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  RenderTreeBuilder::SVG::SVG(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::SVG::Attach(LegacyRenderSVGRoot &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
  }

  void RenderTreeBuilder::SVG::Attach(LegacyRenderSVGContainer &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
  }

  void RenderTreeBuilder::SVG::Attach(RenderSVGInline &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.InlineBuilder().Attach(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(parent))
    {
      textAncestor->SubtreeChildWasAdded(&childToAdd);
    }
  }

  void RenderTreeBuilder::SVG::Attach(RenderSVGText &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    auto &childToAdd = *child;
    _builder.BlockFlowBuilder().Attach(parent, krys::move(child), beforeChild);
    SVGResourcesCache::ClientWasAddedToTree(childToAdd);
    parent.SubtreeChildWasAdded(&childToAdd);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGRoot &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(RenderSVGText &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);

    SmallList<SVGTextLayoutAttributes *, 2> affectedAttributes;
    parent.SubtreeChildWillBeRemoved(&child, affectedAttributes);
    auto takenChild =
      _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    parent.SubtreeChildWasRemoved(affectedAttributes);
    return takenChild;
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(RenderSVGInline &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);

    auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(parent);
    if (!textAncestor)
    {
      return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
    }

    SmallList<SVGTextLayoutAttributes *, 2> affectedAttributes;
    textAncestor->SubtreeChildWillBeRemoved(&child, affectedAttributes);
    auto takenChild = _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
    textAncestor->SubtreeChildWasRemoved(affectedAttributes);
    return takenChild;
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGContainer &parent,
                                                         RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }
}
