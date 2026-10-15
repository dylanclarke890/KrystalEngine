#include "Krystal.Booey/Rendering/Style/SVGRenderStyleDefs.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleFillData::StyleFillData() noexcept
      : FillOpacity(style::ComputedStyle::InitialFillOpacity()), Fill(style::ComputedStyle::InitialFill()),
        VisitedLinkFill(style::ComputedStyle::InitialFill())
  {
  }

  inline StyleFillData::StyleFillData(const StyleFillData &other) noexcept
      : RefCounted<StyleFillData>(), FillOpacity(other.FillOpacity), Fill(other.Fill),
        VisitedLinkFill(other.VisitedLinkFill)
  {
  }

  Ref<StyleFillData> StyleFillData::Copy() const noexcept
  {
    return AdoptRef(*new StyleFillData(*this));
  }

  bool StyleFillData::operator==(const StyleFillData &other) const noexcept
  {
    return FillOpacity == other.FillOpacity && Fill == other.Fill && VisitedLinkFill == other.VisitedLinkFill;
  }

  StyleStrokeData::StyleStrokeData() noexcept
      : StrokeOpacity(style::ComputedStyle::InitialStrokeOpacity()),
        Stroke(style::ComputedStyle::InitialStroke()),
        VisitedLinkStroke(style::ComputedStyle::InitialStroke()),
        StrokeDashOffset(style::ComputedStyle::InitialStrokeDashOffset()),
        StrokeDashArray(style::ComputedStyle::InitialStrokeDashArray())
  {
  }

  inline StyleStrokeData::StyleStrokeData(const StyleStrokeData &other) noexcept
      : RefCounted<StyleStrokeData>(), StrokeOpacity(other.StrokeOpacity), Stroke(other.Stroke),
        VisitedLinkStroke(other.VisitedLinkStroke), StrokeDashOffset(other.StrokeDashOffset),
        StrokeDashArray(other.StrokeDashArray)
  {
  }

  Ref<StyleStrokeData> StyleStrokeData::Copy() const noexcept
  {
    return AdoptRef(*new StyleStrokeData(*this));
  }

  bool StyleStrokeData::operator==(const StyleStrokeData &other) const noexcept
  {
    return StrokeOpacity == other.StrokeOpacity && Stroke == other.Stroke
           && VisitedLinkStroke == other.VisitedLinkStroke && StrokeDashOffset == other.StrokeDashOffset
           && StrokeDashArray == other.StrokeDashArray;
  }

  StyleStopData::StyleStopData() noexcept
      : StopOpacity(style::ComputedStyle::InitialStopOpacity()),
        StopColor(style::ComputedStyle::InitialStopColor())
  {
  }

  inline StyleStopData::StyleStopData(const StyleStopData &other) noexcept
      : RefCounted<StyleStopData>(), StopOpacity(other.StopOpacity), StopColor(other.StopColor)
  {
  }

  Ref<StyleStopData> StyleStopData::Copy() const noexcept
  {
    return AdoptRef(*new StyleStopData(*this));
  }

  bool StyleStopData::operator==(const StyleStopData &other) const noexcept
  {
    return StopOpacity == other.StopOpacity && StopColor == other.StopColor;
  }

  StyleMiscData::StyleMiscData() noexcept
      : FloodOpacity(style::ComputedStyle::InitialFloodOpacity()),
        FloodColor(style::ComputedStyle::InitialFloodColor()),
        LightingColor(style::ComputedStyle::InitialLightingColor()),
        BaselineShift(style::ComputedStyle::InitialBaselineShift())
  {
  }

  inline StyleMiscData::StyleMiscData(const StyleMiscData &other) noexcept
      : RefCounted<StyleMiscData>(), FloodOpacity(other.FloodOpacity), FloodColor(other.FloodColor),
        LightingColor(other.LightingColor), BaselineShift(other.BaselineShift)
  {
  }

  Ref<StyleMiscData> StyleMiscData::Copy() const noexcept
  {
    return AdoptRef(*new StyleMiscData(*this));
  }

  bool StyleMiscData::operator==(const StyleMiscData &other) const noexcept
  {
    return FloodOpacity == other.FloodOpacity && FloodColor == other.FloodColor
           && LightingColor == other.LightingColor && BaselineShift == other.BaselineShift;
  }

  StyleInheritedResourceData::StyleInheritedResourceData() noexcept
      : MarkerStart(style::ComputedStyle::InitialMarkerStart()),
        MarkerMid(style::ComputedStyle::InitialMarkerMid()),
        MarkerEnd(style::ComputedStyle::InitialMarkerEnd())
  {
  }

  inline StyleInheritedResourceData::StyleInheritedResourceData(
    const StyleInheritedResourceData &other) noexcept
      : RefCounted<StyleInheritedResourceData>(), MarkerStart(other.MarkerStart), MarkerMid(other.MarkerMid),
        MarkerEnd(other.MarkerEnd)
  {
  }

  Ref<StyleInheritedResourceData> StyleInheritedResourceData::Copy() const noexcept
  {
    return AdoptRef(*new StyleInheritedResourceData(*this));
  }

  bool StyleInheritedResourceData::operator==(const StyleInheritedResourceData &other) const noexcept
  {
    return MarkerStart == other.MarkerStart && MarkerMid == other.MarkerMid && MarkerEnd == other.MarkerEnd;
  }

  StyleLayoutData::StyleLayoutData() noexcept
      : Cx(style::ComputedStyle::InitialCx()), Cy(style::ComputedStyle::InitialCy()),
        R(style::ComputedStyle::InitialR()), Rx(style::ComputedStyle::InitialRx()),
        Ry(style::ComputedStyle::InitialRy()), X(style::ComputedStyle::InitialX()),
        Y(style::ComputedStyle::InitialY()), D(style::ComputedStyle::InitialD())
  {
  }

  inline StyleLayoutData::StyleLayoutData(const StyleLayoutData &other) noexcept
      : RefCounted<StyleLayoutData>(), Cx(other.Cx), Cy(other.Cy), R(other.R), Rx(other.Rx), Ry(other.Ry),
        X(other.X), Y(other.Y), D(other.D)
  {
  }

  Ref<StyleLayoutData> StyleLayoutData::Copy() const noexcept
  {
    return AdoptRef(*new StyleLayoutData(*this));
  }

  bool StyleLayoutData::operator==(const StyleLayoutData &other) const noexcept
  {
    return Cx == other.Cx && Cy == other.Cy && R == other.R && Rx == other.Rx && Ry == other.Ry
           && X == other.X && Y == other.Y && D == other.D;
  }
}
