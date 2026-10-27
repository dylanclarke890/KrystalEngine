#pragma once

#include "Krystal.Booey/SVG/SVGComponentTransferFunctionElement.hpp"

namespace krys::boo::svg
{
  /// @brief The `<feFuncA>` element: how its `<feComponentTransfer>` transfers the alpha channel.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEFuncAElement
  class SVGFEFuncAElement final : public SVGComponentTransferFunctionElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEFuncAElement);

  public:
    explicit SVGFEFuncAElement(dom::Document &document) noexcept
        : SVGComponentTransferFunctionElement(document, SVGElementInterface::FEFuncA)
    {
    }

    KRYS_NODISCARD ComponentTransferChannel Channel() const noexcept final
    {
      return ComponentTransferChannel::Alpha;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEFuncAElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEFuncAElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
