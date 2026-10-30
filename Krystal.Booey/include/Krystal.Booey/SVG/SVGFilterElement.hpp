#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo::svg
{
  /// @brief The `<filter>` element: filter primitives applied to the elements that reference it.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFilterElement
  class SVGFilterElement final : public SVGElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFilterElement);

  private:
    Ref<SVGAnimatedEnumeration> _filterUnits;
    Ref<SVGAnimatedEnumeration> _primitiveUnits;
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;

  public:
    explicit SVGFilterElement(dom::Document &document) noexcept;

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType FilterUnits() const noexcept
    {
      return _filterUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PrimitiveUnits() const noexcept
    {
      return _primitiveUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
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

    KRYS_NODISCARD SVGAnimatedEnumeration &FilterUnitsAnimated() noexcept
    {
      return *_filterUnits;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &PrimitiveUnitsAnimated() noexcept
    {
      return *_primitiveUnits;
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

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGFilterElement, SVGElement, SVGURIReference>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &child) const noexcept final;

    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFilterElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFilterElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
