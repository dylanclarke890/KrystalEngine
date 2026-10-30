#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<mask>` element: graphics whose luminance or alpha masks the elements that reference it.
  /// @see https://drafts.fxtf.org/css-masking/#InterfaceSVGMaskElement
  class SVGMaskElement final : public SVGElement, public SVGTests
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMaskElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedEnumeration> _maskUnits;
    Ref<SVGAnimatedEnumeration> _maskContentUnits;

  public:
    explicit SVGMaskElement(dom::Document &document) noexcept;

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
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType MaskUnits() const noexcept
    {
      return _maskUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType MaskContentUnits() const noexcept
    {
      return _maskContentUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
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
    KRYS_NODISCARD SVGAnimatedEnumeration &MaskUnitsAnimated() noexcept
    {
      return *_maskUnits;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &MaskContentUnitsAnimated() noexcept
    {
      return *_maskContentUnits;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGMaskElement, SVGElement, SVGTests>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGMaskElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGMaskElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
