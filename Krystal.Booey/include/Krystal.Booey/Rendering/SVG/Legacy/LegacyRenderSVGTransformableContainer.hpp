#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"

namespace krys::boo::svg
{
  class SVGGraphicsElement;
}

namespace krys::boo
{
  /// @brief The renderer of a transformable SVG container: `<g>`, `<a>`, `<switch>` and `<use>`.
  class LegacyRenderSVGTransformableContainer final : public LegacyRenderSVGContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGTransformableContainer);

  private:
    bool _needsTransformUpdate : 1 {true};
    bool _didTransformToRootUpdate : 1 {false};
    AffineTransform _localTransform {};
    FloatSize _additionalTranslation {};
    FloatRect _lastTransformReferenceBoxRect {};

  public:
    LegacyRenderSVGTransformableContainer(svg::SVGGraphicsElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGTransformableContainer() noexcept override;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localTransform;
    }
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }
    KRYS_NODISCARD bool DidTransformToRootUpdate() noexcept override
    {
      return _didTransformToRootUpdate;
    }
    KRYS_NODISCARD const FloatSize &AdditionalTranslation() const noexcept
    {
      return _additionalTranslation;
    }

  private:
    KRYS_NODISCARD svg::SVGGraphicsElement &GraphicsElement() noexcept;
    void GetElement() const = delete;

    bool CalculateLocalTransform() noexcept override;
    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept override
    {
      return _localTransform;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGTransformableContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGTransformableContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
