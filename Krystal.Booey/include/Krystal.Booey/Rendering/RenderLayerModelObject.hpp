#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief A renderer that may have a layer.
  // FIXME: Repaint, animations, viewport-constrained objects and SVG (transforms, resources, clipping) come
  // with painting, animations, scrolling and SVG.
  class RenderLayerModelObject : public RenderElement
  {
    KRYS_NON_COPY_MOVABLE(RenderLayerModelObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayerModelObject);

  private:
    UniquePtr<RenderLayer> _layer;

  public:
    ~RenderLayerModelObject() noexcept override;

    void DestroyLayer() noexcept;

    KRYS_NODISCARD bool HasSelfPaintingLayer() const noexcept;
    KRYS_NODISCARD RenderLayer *Layer() const noexcept
    {
      return _layer.get();
    }

    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    KRYS_NODISCARD virtual bool RequiresLayer() const noexcept = 0;

    KRYS_NODISCARD TransformationMatrix *LayerTransform() const noexcept;

    virtual void UpdateLayerTransform() noexcept;
    virtual void ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                                const FloatRect &boundingBox,
                                EnumSet<style::TransformResolverOption> options) const noexcept = 0;
    void ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                        const FloatRect &boundingBox) const noexcept;

    KRYS_NODISCARD bool ShouldUsePositionedClipping() const noexcept
    {
      // FIXME: SVG foreign objects too, once there's SVG.
      return IsAbsolutelyPositioned();
    }

  protected:
    RenderLayerModelObject(Type type, dom::Element &element, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderLayerModelObject(Type type, dom::Document &document, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;

    void CreateLayer() noexcept;
    void WillBeDestroyed() noexcept override;

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
