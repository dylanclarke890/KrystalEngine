#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo::svg
{
  struct PatternAttributes;

  /// @brief The `<pattern>` element: a tile of graphics repeated to fill or stroke a shape.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGPatternElement
  class SVGPatternElement final : public SVGElement,
                                  public SVGFitToViewBox,
                                  public SVGTests,
                                  public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGPatternElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedEnumeration> _patternUnits;
    Ref<SVGAnimatedEnumeration> _patternContentUnits;
    Ref<SVGAnimatedTransformList> _patternTransform;

  public:
    explicit SVGPatternElement(dom::Document &document) noexcept;

    void CollectPatternAttributes(PatternAttributes &) const noexcept;

    KRYS_NODISCARD AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept final;

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
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PatternUnits() const noexcept
    {
      return _patternUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PatternContentUnits() const noexcept
    {
      return _patternContentUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD const SVGTransformList &PatternTransform() const noexcept
    {
      return _patternTransform->CurrentValue();
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
    KRYS_NODISCARD SVGAnimatedEnumeration &PatternUnitsAnimated() noexcept
    {
      return *_patternUnits;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &PatternContentUnitsAnimated() noexcept
    {
      return *_patternContentUnits;
    }
    KRYS_NODISCARD SVGAnimatedTransformList &PatternTransformAnimated() noexcept
    {
      return *_patternTransform;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGPatternElement, SVGElement, SVGFitToViewBox, SVGTests, SVGURIReference>;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGPatternElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGPatternElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
