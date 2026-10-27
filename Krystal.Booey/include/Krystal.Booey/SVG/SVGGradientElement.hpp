#pragma once

#include "Krystal.Booey/Graphics/GradientColorStops.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo::svg
{
  enum SVGSpreadMethodType : uint8
  {
    SVGSpreadMethodUnknown = 0,
    SVGSpreadMethodPad,
    SVGSpreadMethodReflect,
    SVGSpreadMethodRepeat
  };

  template <>
  struct SVGPropertyTraits<SVGSpreadMethodType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGSpreadMethodRepeat;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGSpreadMethodType type) noexcept
    {
      switch (type)
      {
        case SVGSpreadMethodUnknown:
        {
          return {};
        }
        case SVGSpreadMethodPad:
        {
          return u8"pad";
        }
        case SVGSpreadMethodReflect:
        {
          return u8"reflect";
        }
        case SVGSpreadMethodRepeat:
        {
          return u8"repeat";
        }
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGSpreadMethodType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"pad")
      {
        return SVGSpreadMethodPad;
      }
      if (value == u8"reflect")
      {
        return SVGSpreadMethodReflect;
      }
      if (value == u8"repeat")
      {
        return SVGSpreadMethodRepeat;
      }
      return SVGSpreadMethodUnknown;
    }
  };

  /// @brief The base of the gradient elements: their stops, spread method, units and transform.
  /// @see https://svgwg.org/svg2-draft/pservers.html#InterfaceSVGGradientElement
  class SVGGradientElement : public SVGElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGradientElement);

  private:
    Ref<SVGAnimatedEnumeration> _spreadMethod;
    Ref<SVGAnimatedEnumeration> _gradientUnits;
    Ref<SVGAnimatedTransformList> _gradientTransform;

  public:
    enum : uint8
    {
      SVG_SPREADMETHOD_UNKNOWN = SVGSpreadMethodUnknown,
      SVG_SPREADMETHOD_PAD = SVGSpreadMethodPad,
      SVG_SPREADMETHOD_REFLECT = SVGSpreadMethodReflect,
      SVG_SPREADMETHOD_REPEAT = SVGSpreadMethodRepeat
    };

    KRYS_NODISCARD GradientColorStops BuildStops() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGGradientElement, SVGElement, SVGURIReference>;

    KRYS_NODISCARD SVGSpreadMethodType SpreadMethod() const noexcept
    {
      return _spreadMethod->CurrentValue<SVGSpreadMethodType>();
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType GradientUnits() const noexcept
    {
      return _gradientUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD const SVGTransformList &GradientTransform() const noexcept
    {
      return _gradientTransform->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedEnumeration &SpreadMethodAnimated() noexcept
    {
      return *_spreadMethod;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &GradientUnitsAnimated() noexcept
    {
      return *_gradientUnits;
    }
    KRYS_NODISCARD SVGAnimatedTransformList &GradientTransformAnimated() noexcept
    {
      return *_gradientTransform;
    }

  protected:
    SVGGradientElement(dom::Document &, UniquePtr<SVGPropertyRegistry> &&, SVGElementInterface) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;

    void InvalidateGradientResource() noexcept;

  private:
    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept override
    {
      return false;
    }
    void OnChildrenChanged() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGGradientElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &element) noexcept
  {
    return element.IsSVGRadialGradientElement() || element.IsSVGLinearGradientElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
