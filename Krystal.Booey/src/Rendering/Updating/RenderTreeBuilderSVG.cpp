#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderSVG.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"

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

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGRoot &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGContainer &parent,
                                                         RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    SVGResourcesCache::ClientWillBeRemovedFromTree(child);
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }
}
