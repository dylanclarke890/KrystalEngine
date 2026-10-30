#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/SVG/SVGZoomAndPanType.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  /// @brief The `zoomAndPan` attribute of the elements that establish a viewport.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGZoomAndPan
  class SVGZoomAndPan
  {
    KRYS_NON_COPYABLE(SVGZoomAndPan);

  private:
    SVGZoomAndPanType _zoomAndPan {SVGPropertyTraits<SVGZoomAndPanType>::InitialValue()};

  public:
    // Forward declare enumerations in the W3C naming scheme, for IDL generation.
    enum : uint16
    {
      SVG_ZOOMANDPAN_UNKNOWN = SVGZoomAndPanUnknown,
      SVG_ZOOMANDPAN_DISABLE = SVGZoomAndPanDisable,
      SVG_ZOOMANDPAN_MAGNIFY = SVGZoomAndPanMagnify
    };

    virtual ~SVGZoomAndPan() noexcept = default;

    KRYS_NODISCARD SVGZoomAndPanType ZoomAndPan() const noexcept
    {
      return _zoomAndPan;
    }

    void SetZoomAndPan(SVGZoomAndPanType zoomAndPan) noexcept
    {
      _zoomAndPan = zoomAndPan;
    }

    /// @brief Setting the attribute through the DOM fails.
    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGZoomAndPan__zoomAndPan
    dom::ExceptionOr<void> SetZoomAndPan(uint16) noexcept
    {
      return dom::Exception {dom::ExceptionCode::NoModificationAllowedError};
    }

    void Reset() noexcept
    {
      _zoomAndPan = SVGPropertyTraits<SVGZoomAndPanType>::InitialValue();
    }

    void ParseAttribute(SVGElement &element, const dom::QualifiedName &attributeName,
                        Maybe<dom::DOMStringView> value) noexcept;

  protected:
    SVGZoomAndPan() noexcept = default;

    KRYS_NODISCARD static Maybe<SVGZoomAndPanType> ParseZoomAndPan(dom::DOMStringView &buffer) noexcept;
  };
}
