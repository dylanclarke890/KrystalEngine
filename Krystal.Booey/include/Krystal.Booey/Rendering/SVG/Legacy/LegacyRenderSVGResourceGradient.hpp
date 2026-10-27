#pragma once

#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/SVG/SVGGradientElement.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class GraphicsContext;

  /// @brief A gradient built for one client, and what it was built for.
  struct GradientData
  {
    struct Inputs
    {
      Maybe<FloatRect> ObjectBoundingBox;
      float TextPaintingScale = 1;

      KRYS_NODISCARD friend bool operator==(const Inputs &, const Inputs &) noexcept = default;
    };

    bool Invalidate(const Inputs &inputs) noexcept
    {
      if (this->GradientInputs != inputs)
      {
        Gradient = nullptr;
        UserspaceTransform = AffineTransform();
        this->GradientInputs = inputs;
      }
      return !Gradient;
    }

    RefPtr<boo::Gradient> Gradient;
    AffineTransform UserspaceTransform;
    Inputs GradientInputs;
  };

  /// @brief How a gradient is applied around what a client paints with it.
  class GradientApplier
  {
  public:
    GradientApplier() noexcept = default;
    virtual ~GradientApplier() noexcept = default;

    virtual bool ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&, const GradientData &,
                               EnumSet<RenderSVGResourceMode>) noexcept = 0;
    virtual void PostApplyResource(RenderElement &, GraphicsContext *&, const GradientData &,
                                   svg::SVGUnitTypes::SVGUnitType gradientUnits,
                                   const AffineTransform &gradientTransform, EnumSet<RenderSVGResourceMode>,
                                   const Path *, const RenderElement *) noexcept = 0;
  };

  /// @brief The renderer of a gradient element: builds the gradient each client paints with.
  class LegacyRenderSVGResourceGradient : public LegacyRenderSVGResourceContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGResourceGradient);

  private:
    HashMap<RenderObject *, UniquePtr<GradientData>> _gradientMap;

    UniquePtr<GradientApplier> _gradientApplier;
    bool _shouldCollectGradientAttributes {true};

  public:
    ~LegacyRenderSVGResourceGradient() noexcept override;

    KRYS_NODISCARD svg::SVGGradientElement &GradientElement() const noexcept
    {
      return static_cast<svg::SVGGradientElement &>(LegacyRenderSVGResourceContainer::GetElement());
    }

    void RemoveAllClientsFromCache() noexcept final;
    void RemoveAllClientsFromCacheAndMarkForInvalidationIfNeeded(
      bool markForInvalidation, VisitedRenderers *visitedRenderers) noexcept override;
    void RemoveClientFromCache(RenderElement &) noexcept final;

    EnumSet<ApplyResult> ApplyResource(RenderElement &, const RenderStyle &, GraphicsContext *&,
                                       EnumSet<RenderSVGResourceMode>) noexcept final;
    void PostApplyResource(RenderElement &, GraphicsContext *&, EnumSet<RenderSVGResourceMode>, const Path *,
                           const RenderElement *) noexcept final;
    KRYS_NODISCARD FloatRect ResourceBoundingBox(const RenderObject &, RepaintRectCalculation) noexcept final
    {
      return FloatRect();
    }

  protected:
    LegacyRenderSVGResourceGradient(Type, svg::SVGGradientElement &, RenderStyle &&) noexcept;

    KRYS_NODISCARD static GradientColorStops StopsByApplyingColorFilter(const GradientColorStops &,
                                                                        const RenderStyle &) noexcept;
    KRYS_NODISCARD static GradientSpreadMethod
      PlatformSpreadMethodFromSVGType(svg::SVGSpreadMethodType) noexcept;

  private:
    void GetElement() const = delete;

    KRYS_NODISCARD GradientData::Inputs ComputeInputs(RenderElement &,
                                                      EnumSet<RenderSVGResourceMode>) noexcept;
    KRYS_NODISCARD GradientData *GradientDataForRenderer(RenderElement &, const RenderStyle &,
                                                         EnumSet<RenderSVGResourceMode>) noexcept;

    KRYS_NODISCARD virtual svg::SVGUnitTypes::SVGUnitType GradientUnits() const noexcept = 0;
    KRYS_NODISCARD virtual AffineTransform GradientTransform() const noexcept = 0;
    virtual bool CollectGradientAttributes() noexcept = 0;
    KRYS_NODISCARD virtual Ref<Gradient> BuildGradient(const RenderStyle &) const noexcept = 0;
  };
}
