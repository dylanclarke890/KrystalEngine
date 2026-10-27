#pragma once

#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGZoomAndPan.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class SVGElement;
  class SVGTransformList;

  /// @brief The view an `svgView(...)` fragment identifier gives an `<svg>`: its view box, aspect ratio, zoom
  /// and pan, transform and target.
  /// @see https://svgwg.org/svg2-draft/linking.html#SVGFragmentIdentifiers
  class SVGViewSpec final : public RefCounted<SVGViewSpec>, public SVGFitToViewBox, public SVGZoomAndPan
  {
  private:
    WeakPtr<SVGElement> _contextElement;
    dom::DOMString _viewTargetString;
    Ref<SVGTransformList> _transform;

  public:
    KRYS_NODISCARD static Ref<SVGViewSpec> Create(SVGElement &contextElement) noexcept
    {
      return AdoptRef(*new SVGViewSpec(contextElement));
    }

    bool ParseViewSpec(dom::DOMStringView) noexcept;
    void Reset() noexcept;
    void ResetContextElement() noexcept
    {
      _contextElement = nullptr;
    }

    KRYS_NODISCARD RefPtr<SVGElement> ViewTarget() const noexcept;
    KRYS_NODISCARD const dom::DOMString &ViewTargetString() const noexcept
    {
      return _viewTargetString;
    }

    KRYS_NODISCARD dom::DOMString TransformString() const noexcept;
    KRYS_NODISCARD Ref<SVGTransformList> &Transform() noexcept
    {
      return _transform;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGViewSpec, SVGFitToViewBox>;

  private:
    explicit SVGViewSpec(SVGElement &) noexcept;
  };
}
