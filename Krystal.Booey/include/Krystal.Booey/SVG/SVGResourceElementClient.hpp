#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderElement;
}

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief A renderer outside SVG that refers to an SVG element as a resource (a clip path from CSS), told
  /// when the element changes.
  class SVGResourceElementClient : public CanMakeWeakPtr<SVGResourceElementClient>,
                                   public CanMakeCheckedPtr<SVGResourceElementClient>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGResourceElementClient);

  public:
    virtual ~SVGResourceElementClient() noexcept = default;

    virtual void ResourceChanged(SVGElement &) noexcept = 0;

    KRYS_NODISCARD virtual const RenderElement &Renderer() const noexcept = 0;
  };
}
