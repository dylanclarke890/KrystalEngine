#pragma once

#include "Krystal.Web/Svg/Elements/SVGComponentTransferFunctionElement.hpp"

namespace krys::web
{
  /// @brief The `<feFuncA>` element: how its `<feComponentTransfer>` transfers the alpha channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFuncAElement
  class SVGFEFuncAElement final : public SVGComponentTransferFunctionElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFuncAElement);

  public:
    explicit SVGFEFuncAElement(Document &document) noexcept
        : SVGComponentTransferFunctionElement(document, SVGElementInterface::FEFuncA)
    {
    }

    KRYS_NODISCARD ComponentTransferChannel Channel() const noexcept final
    {
      return ComponentTransferChannel::Alpha;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::SVGFEFuncAElement)
  KRYS_NODISCARD static bool IsType(const krys::web::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncAElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
