#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGFitToViewBox.hpp"
#include "Krystal.Booey/SVG/SVGMarkerTypes.hpp"

namespace krys::boo::svg
{
  class SVGAngle;

  /// @brief The `<marker>` element: graphics drawn at the vertices of the paths and shapes that reference it.
  /// @see https://svgwg.org/svg2-draft/painting.html#InterfaceSVGMarkerElement
  class SVGMarkerElement final : public SVGElement, public SVGFitToViewBox
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGMarkerElement);

  private:
    Ref<SVGAnimatedLength> _refX;
    Ref<SVGAnimatedLength> _refY;
    Ref<SVGAnimatedLength> _markerWidth;
    Ref<SVGAnimatedLength> _markerHeight;
    Ref<SVGAnimatedEnumeration> _markerUnits;
    Ref<SVGAnimatedAngle> _orientAngle;
    Ref<SVGAnimatedOrientType> _orientType;

  public:
    // Forward declare enumerations in the W3C naming scheme, for IDL generation.
    enum : uint16
    {
      SVG_MARKERUNITS_UNKNOWN = krys::ToUnderlying(SVGMarkerUnitsType::Unknown),
      SVG_MARKERUNITS_USERSPACEONUSE = krys::ToUnderlying(SVGMarkerUnitsType::UserSpaceOnUse),
      SVG_MARKERUNITS_STROKEWIDTH = krys::ToUnderlying(SVGMarkerUnitsType::StrokeWidth)
    };

    enum : uint16
    {
      SVG_MARKER_ORIENT_UNKNOWN = SVGMarkerOrientUnknown,
      SVG_MARKER_ORIENT_AUTO = SVGMarkerOrientAuto,
      SVG_MARKER_ORIENT_ANGLE = SVGMarkerOrientAngle,
      SVG_MARKER_ORIENT_AUTO_START_REVERSE = SVGMarkerOrientAutoStartReverse
    };

    explicit SVGMarkerElement(dom::Document &document) noexcept;

    KRYS_NODISCARD AffineTransform ViewBoxToViewTransform(float viewWidth, float viewHeight) const noexcept;

    KRYS_NODISCARD const SVGLengthValue &RefX() const noexcept
    {
      return _refX->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &RefY() const noexcept
    {
      return _refY->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &MarkerWidth() const noexcept
    {
      return _markerWidth->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &MarkerHeight() const noexcept
    {
      return _markerHeight->CurrentValue();
    }
    KRYS_NODISCARD SVGMarkerUnitsType MarkerUnits() const noexcept
    {
      return _markerUnits->CurrentValue<SVGMarkerUnitsType>();
    }
    KRYS_NODISCARD const SVGAngleValue &OrientAngle() const noexcept
    {
      return _orientAngle->CurrentValue();
    }
    KRYS_NODISCARD SVGMarkerOrientType OrientType() const noexcept
    {
      return _orientType->CurrentValue<SVGMarkerOrientType>();
    }

    KRYS_NODISCARD SVGAnimatedLength &RefXAnimated() noexcept
    {
      return *_refX;
    }
    KRYS_NODISCARD SVGAnimatedLength &RefYAnimated() noexcept
    {
      return *_refY;
    }
    KRYS_NODISCARD SVGAnimatedLength &MarkerWidthAnimated() noexcept
    {
      return *_markerWidth;
    }
    KRYS_NODISCARD SVGAnimatedLength &MarkerHeightAnimated() noexcept
    {
      return *_markerHeight;
    }
    KRYS_NODISCARD SVGAnimatedEnumeration &MarkerUnitsAnimated() noexcept
    {
      return *_markerUnits;
    }
    KRYS_NODISCARD SVGAnimatedAngle &OrientAngleAnimated() noexcept
    {
      return *_orientAngle;
    }
    KRYS_NODISCARD Ref<SVGAnimatedEnumeration> OrientTypeAnimated() noexcept
    {
      return _orientType;
    }

    KRYS_NODISCARD dom::DOMStringAtom Orient() const noexcept;
    void SetOrient(const dom::DOMStringAtom &) noexcept;
    void SetOrientToAuto() noexcept;
    void SetOrientToAngle(const SVGAngle &) noexcept;
    void SetOrientToAutoStartReverse() noexcept;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGMarkerElement, SVGElement, SVGFitToViewBox>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;
    void OnChildrenChanged() noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;
    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &) noexcept final
    {
      return true;
    }
    KRYS_NODISCARD bool NeedsPendingResourceHandling() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final;

    void InvalidateMarkerResource() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGMarkerElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGMarkerElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
