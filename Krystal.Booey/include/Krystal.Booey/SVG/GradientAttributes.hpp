#pragma once

#include "Krystal.Booey/Graphics/GradientColorStops.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/SVGGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"

namespace krys::boo::svg
{
  /// @brief A gradient's attributes, collected from it and the gradients it references.
  struct GradientAttributes
  {
  private:
    // Properties
    AffineTransform _gradientTransform;
    GradientColorStops _stops;

    SVGSpreadMethodType _spreadMethod {SVGSpreadMethodPad};
    SVGUnitTypes::SVGUnitType _gradientUnits {SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX};

    // Property states
    bool _spreadMethodSet : 1 {false};
    bool _gradientUnitsSet : 1 {false};
    bool _gradientTransformSet : 1 {false};

  public:
    KRYS_NODISCARD SVGSpreadMethodType SpreadMethod() const noexcept
    {
      return _spreadMethod;
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType GradientUnits() const noexcept
    {
      return _gradientUnits;
    }
    KRYS_NODISCARD const AffineTransform &GradientTransform() const noexcept
    {
      return _gradientTransform;
    }
    KRYS_NODISCARD const GradientColorStops &Stops() const noexcept
    {
      return _stops;
    }

    void SetSpreadMethod(SVGSpreadMethodType value) noexcept
    {
      _spreadMethod = value;
      _spreadMethodSet = true;
    }

    void SetGradientUnits(SVGUnitTypes::SVGUnitType unitType) noexcept
    {
      _gradientUnits = unitType;
      _gradientUnitsSet = true;
    }

    void SetGradientTransform(const AffineTransform &value) noexcept
    {
      _gradientTransform = value;
      _gradientTransformSet = true;
    }

    void SetStops(GradientColorStops &&value) noexcept
    {
      _stops = krys::move(value);
    }

    KRYS_NODISCARD bool HasSpreadMethod() const noexcept
    {
      return _spreadMethodSet;
    }
    KRYS_NODISCARD bool HasGradientUnits() const noexcept
    {
      return _gradientUnitsSet;
    }
    KRYS_NODISCARD bool HasGradientTransform() const noexcept
    {
      return _gradientTransformSet;
    }
    KRYS_NODISCARD bool HasStops() const noexcept
    {
      return !_stops.IsEmpty();
    }
  };
}
