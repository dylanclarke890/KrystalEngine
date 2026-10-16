#pragma once

#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A renderer that may have a layer.
  // FIXME: Layers (RenderLayer, creation and destruction, repaint, composited bounds, transforms, SVG layers)
  // come with layout and painting; hasLayer() is always false until then.
  class RenderLayerModelObject : public RenderElement
  {
    KRYS_NON_COPY_MOVABLE(RenderLayerModelObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayerModelObject);

  public:
    ~RenderLayerModelObject() noexcept override;

  protected:
    RenderLayerModelObject(Type type, dom::Element &element, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderLayerModelObject(Type type, dom::Document &document, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    /// @brief Sets the state bits that follow from the renderer's style.
    virtual void UpdateFromStyle() noexcept
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderLayerModelObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderLayerModelObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
