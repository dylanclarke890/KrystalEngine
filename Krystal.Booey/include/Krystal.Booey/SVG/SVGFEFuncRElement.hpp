#pragma once

#include "Krystal.Booey/SVG/SVGComponentTransferFunctionElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feFuncR>` element: how its `<feComponentTransfer>` transfers the red channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFuncRElement
  class SVGFEFuncRElement final : public SVGComponentTransferFunctionElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFuncRElement);

  public:
    explicit SVGFEFuncRElement(dom::Document &document) noexcept
        : SVGComponentTransferFunctionElement(document, SVGElementInterface::FEFuncR)
    {
    }

    KRYS_NODISCARD ComponentTransferChannel Channel() const noexcept final
    {
      return ComponentTransferChannel::Red;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEFuncRElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
