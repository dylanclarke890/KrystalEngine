#pragma once

#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"

namespace krys::boo::svg
{
  /// @brief The `<clipPath>` element: the shapes that clip the elements that reference it.
  /// @see https://drafts.fxtf.org/css-masking/#InterfaceSVGClipPathElement
  class SVGClipPathElement final : public SVGGraphicsElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGClipPathElement);

  private:
    Ref<SVGAnimatedEnumeration> _clipPathUnits;

  public:
    explicit SVGClipPathElement(dom::Document &document) noexcept;

    KRYS_NODISCARD SVGUnitTypes::SVGUnitType ClipPathUnits() const noexcept
    {
      return _clipPathUnits->CurrentValue<SVGUnitTypes::SVGUnitType>();
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &ClipPathUnitsAnimated() noexcept
    {
      return *_clipPathUnits;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGClipPathElement, SVGGraphicsElement>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGClipPathElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGClipPathElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
