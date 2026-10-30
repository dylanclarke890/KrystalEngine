#pragma once

#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"

namespace krys::boo::svg
{
  class SVGComponentTransferFunctionElement;

  /// @brief The `<feComponentTransfer>` element: each of its input's channels, transferred by its function
  /// child for that channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEComponentTransferElement
  class SVGFEComponentTransferElement final : public SVGFilterPrimitiveStandardAttributes
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEComponentTransferElement);

  private:
    Ref<SVGAnimatedString> _in1;

  public:
    explicit SVGFEComponentTransferElement(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString In1() const noexcept
    {
      return _in1->CurrentValue();
    }
    KRYS_NODISCARD SVGAnimatedString &In1Animated() noexcept
    {
      return *_in1;
    }

    void TransferFunctionAttributeChanged(SVGComponentTransferFunctionElement &child,
                                          const dom::QualifiedName &attributeName) noexcept;

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEComponentTransferElement, SVGFilterPrimitiveStandardAttributes>;

  protected:
    bool SetFilterEffectAttributeFromChild(FilterEffect &effect, const dom::Element &child,
                                           const dom::QualifiedName &attributeName) noexcept final;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    KRYS_NODISCARD List<dom::DOMStringAtom> FilterEffectInputsNames() const noexcept final
    {
      return {dom::DOMStringAtom(In1())};
    }
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEComponentTransferElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEComponentTransferElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *svgElement = krys::DynamicDowncast<krys::boo::svg::SVGElement>(node);
    return svgElement && IsType(*svgElement);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
