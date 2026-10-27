#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderSVG.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"

namespace krys::boo
{
  RenderTreeBuilder::SVG::SVG(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::SVG::Attach(LegacyRenderSVGRoot &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    // FIXME: SVGResourcesCache::clientWasAddedToTree, with resources (B11e).
  }

  void RenderTreeBuilder::SVG::Attach(LegacyRenderSVGContainer &parent, RenderPtr<RenderObject> child,
                                      RenderObject *beforeChild) noexcept
  {
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    // FIXME: SVGResourcesCache::clientWasAddedToTree, with resources (B11e).
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGRoot &parent, RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    // FIXME: SVGResourcesCache::clientWillBeRemovedFromTree, with resources (B11e).
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::SVG::Detach(LegacyRenderSVGContainer &parent,
                                                         RenderObject &child,
                                                         WillBeDestroyed willBeDestroyed) noexcept
  {
    // FIXME: SVGResourcesCache::clientWillBeRemovedFromTree, with resources (B11e).
    return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
  }
}
