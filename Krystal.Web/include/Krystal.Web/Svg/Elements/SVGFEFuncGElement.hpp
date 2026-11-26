#pragma once

#include "Krystal.Web/Svg/Elements/SVGComponentTransferFunctionElement.hpp"

namespace krys::web
{
  /// @brief The `<feFuncG>` element: how its `<feComponentTransfer>` transfers the green channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFuncGElement
  class SVGFEFuncGElement final : public SVGComponentTransferFunctionElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFuncGElement);

  public:
    explicit SVGFEFuncGElement(Document &document) noexcept
        : SVGComponentTransferFunctionElement(document, SVGElementInterface::FEFuncG)
    {
    }

    KRYS_NODISCARD ComponentTransferChannel Channel() const noexcept final
    {
      return ComponentTransferChannel::Green;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGFEFuncGElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncGElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
