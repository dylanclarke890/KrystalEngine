#pragma once

#include "Krystal.Booey/SVG/SVGComponentTransferFunctionElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feFuncB>` element: how its `<feComponentTransfer>` transfers the blue channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFuncBElement
  class SVGFEFuncBElement final : public SVGComponentTransferFunctionElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFuncBElement);

  public:
    explicit SVGFEFuncBElement(dom::Document &document) noexcept
        : SVGComponentTransferFunctionElement(document, SVGElementInterface::FEFuncB)
    {
    }

    KRYS_NODISCARD ComponentTransferChannel Channel() const noexcept final
    {
      return ComponentTransferChannel::Blue;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEFuncBElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncBElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
