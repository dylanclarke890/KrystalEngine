#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffectGeometry.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedString.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FilterEffect;
  class GraphicsContext;
}

namespace krys::boo::svg
{
  /// @brief A filter primitive element: its subregion and result name, and the filter effect it creates.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFilterPrimitiveStandardAttributes
  class SVGFilterPrimitiveStandardAttributes : public SVGElement
  {
  private:
    RefPtr<FilterEffect> _effect;

    // Spec: If the x/y attribute is not specified, the effect is as if a value of "0%" were specified.
    // Spec: If the width/height attribute is not specified, the effect is as if a value of "100%" were
    // specified.
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedString> _result;

  public:
    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGFilterPrimitiveStandardAttributes, SVGElement>;

    KRYS_NODISCARD const SVGLengthValue &X() const noexcept
    {
      return _x->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Y() const noexcept
    {
      return _y->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Width() const noexcept
    {
      return _width->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Height() const noexcept
    {
      return _height->CurrentValue();
    }
    KRYS_NODISCARD dom::DOMString Result() const noexcept
    {
      return _result->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedLength &XAnimated() noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD SVGAnimatedLength &YAnimated() noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD SVGAnimatedLength &WidthAnimated() noexcept
    {
      return *_width;
    }
    KRYS_NODISCARD SVGAnimatedLength &HeightAnimated() noexcept
    {
      return *_height;
    }
    KRYS_NODISCARD SVGAnimatedString &ResultAnimated() noexcept
    {
      return *_result;
    }

    KRYS_NODISCARD EnumSet<FilterEffectGeometry::Flags> EffectGeometryFlags() const noexcept;

    KRYS_NODISCARD virtual List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual bool IsIdentity() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual IntOutsets Outsets(const FloatRect &, SVGUnitTypes::SVGUnitType) const noexcept
    {
      return {};
    }

    KRYS_NODISCARD RefPtr<FilterEffect> GetFilterEffect(const FilterEffectVector &inputs,
                                                        const GraphicsContext &destinationContext) noexcept;

    void PrimitiveAttributeChanged(const dom::QualifiedName &attribute) noexcept;
    void MarkFilterEffectForRepaint() noexcept;
    void MarkFilterEffectForRebuild() noexcept;

    static void InvalidateFilterPrimitiveParent(SVGElement *element) noexcept;

  protected:
    SVGFilterPrimitiveStandardAttributes(dom::Document &document,
                                         UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                         SVGElementInterface interface) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;
    void OnChildrenChanged() noexcept override;

    void PrimitiveAttributeOnChildChanged(const dom::Element &child,
                                          const dom::QualifiedName &attribute) noexcept;

    virtual bool SetFilterEffectAttribute(FilterEffect &, const dom::QualifiedName &) noexcept
    {
      return false;
    }
    virtual bool SetFilterEffectAttributeFromChild(FilterEffect &, const dom::Element &,
                                                   const dom::QualifiedName &) noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept = 0;

  private:
    KRYS_NODISCARD bool IsFilterEffect() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept override;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &) const noexcept override
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFilterPrimitiveStandardAttributes)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.IsFilterEffect();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
