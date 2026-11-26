#pragma once

#include "Krystal.Web/Svg/Elements/SVGComponentTransferFunctionElement.hpp"

namespace krys::web
{
  /// @brief The `<feFuncR>` element: how its `<feComponentTransfer>` transfers the red channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFuncRElement
  class SVGFEFuncRElement final : public SVGComponentTransferFunctionElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFuncRElement);

  public:
    explicit SVGFEFuncRElement(Document &document) noexcept
        : SVGComponentTransferFunctionElement(document, SVGElementInterface::FEFuncR)
    {
    }

    KRYS_NODISCARD ComponentTransferChannel Channel() const noexcept final
    {
      return ComponentTransferChannel::Red;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGFEFuncRElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
