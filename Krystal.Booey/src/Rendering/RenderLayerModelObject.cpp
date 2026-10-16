#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"

namespace krys::boo
{
  RenderLayerModelObject::RenderLayerModelObject(Type type, dom::Element &element, RenderStyle &&style,
                                                 EnumSet<TypeFlag> baseTypeFlags,
                                                 TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsLayerModelObject,
                      typeSpecificFlags)
  {
    krys_debug_assert(IsRenderLayerModelObject());
  }

  RenderLayerModelObject::RenderLayerModelObject(Type type, dom::Document &document, RenderStyle &&style,
                                                 EnumSet<TypeFlag> baseTypeFlags,
                                                 TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderElement(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsLayerModelObject,
                      typeSpecificFlags)
  {
    krys_debug_assert(IsRenderLayerModelObject());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderLayerModelObject::~RenderLayerModelObject() noexcept = default;

  void RenderLayerModelObject::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    // FIXME: Remember whether the renderer was floating, had a layer and was transformed, and clear the
    // layer's clip rects, once there are layers.
    RenderElement::StyleWillChange(diff, newStyle);
  }

  void RenderLayerModelObject::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    UpdateFromStyle();
    RenderElement::StyleDidChange(diff, oldStyle);

    // FIXME: Create or destroy the layer, relayout parents of out-of-flow boxes whose display type changed,
    // and update the SVG layer and transform state, once there are layers and layout.
  }
}
