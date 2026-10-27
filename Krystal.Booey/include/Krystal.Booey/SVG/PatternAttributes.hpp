#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"
#include "Krystal.Booey/SVG/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"
#include "Krystal.Booey/SVG/SVGUnitTypes.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  /// @brief A pattern's attributes, collected from it and the patterns it references.
  struct PatternAttributes
  {
  private:
    // Properties
    SVGLengthValue _x;
    SVGLengthValue _y;
    SVGLengthValue _width;
    SVGLengthValue _height;
    FloatRect _viewBox;
    SVGPreserveAspectRatioValue _preserveAspectRatio;
    SVGUnitTypes::SVGUnitType _patternUnits {SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX};
    SVGUnitTypes::SVGUnitType _patternContentUnits {SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE};
    AffineTransform _patternTransform;
    WeakPtr<const SVGPatternElement> _patternContentElement;

    // Property states
    bool _xSet : 1 {false};
    bool _ySet : 1 {false};
    bool _widthSet : 1 {false};
    bool _heightSet : 1 {false};
    bool _viewBoxSet : 1 {false};
    bool _preserveAspectRatioSet : 1 {false};
    bool _patternUnitsSet : 1 {false};
    bool _patternContentUnitsSet : 1 {false};
    bool _patternTransformSet : 1 {false};
    bool _patternContentElementSet : 1 {false};

  public:
    KRYS_NODISCARD SVGLengthValue X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD SVGLengthValue Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD SVGLengthValue Width() const noexcept
    {
      return _width;
    }
    KRYS_NODISCARD SVGLengthValue Height() const noexcept
    {
      return _height;
    }
    KRYS_NODISCARD FloatRect ViewBox() const noexcept
    {
      return _viewBox;
    }
    KRYS_NODISCARD SVGPreserveAspectRatioValue PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio;
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PatternUnits() const noexcept
    {
      return _patternUnits;
    }
    KRYS_NODISCARD SVGUnitTypes::SVGUnitType PatternContentUnits() const noexcept
    {
      return _patternContentUnits;
    }
    KRYS_NODISCARD const AffineTransform &PatternTransform() const noexcept
    {
      return _patternTransform;
    }
    KRYS_NODISCARD const SVGPatternElement *PatternContentElement() const noexcept
    {
      return _patternContentElement.get();
    }

    void SetX(SVGLengthValue value) noexcept
    {
      _x = value;
      _xSet = true;
    }

    void SetY(SVGLengthValue value) noexcept
    {
      _y = value;
      _ySet = true;
    }

    void SetWidth(SVGLengthValue value) noexcept
    {
      _width = value;
      _widthSet = true;
    }

    void SetHeight(SVGLengthValue value) noexcept
    {
      _height = value;
      _heightSet = true;
    }

    void SetViewBox(const FloatRect &value) noexcept
    {
      _viewBox = value;
      _viewBoxSet = true;
    }

    void SetPreserveAspectRatio(SVGPreserveAspectRatioValue value) noexcept
    {
      _preserveAspectRatio = value;
      _preserveAspectRatioSet = true;
    }

    void SetPatternUnits(SVGUnitTypes::SVGUnitType value) noexcept
    {
      _patternUnits = value;
      _patternUnitsSet = true;
    }

    void SetPatternContentUnits(SVGUnitTypes::SVGUnitType value) noexcept
    {
      _patternContentUnits = value;
      _patternContentUnitsSet = true;
    }

    void SetPatternTransform(const AffineTransform &value) noexcept
    {
      _patternTransform = value;
      _patternTransformSet = true;
    }

    void SetPatternContentElement(const SVGPatternElement *value) noexcept
    {
      _patternContentElement = CreateWeakPtr(value);
      _patternContentElementSet = true;
    }

    KRYS_NODISCARD bool HasX() const noexcept
    {
      return _xSet;
    }
    KRYS_NODISCARD bool HasY() const noexcept
    {
      return _ySet;
    }
    KRYS_NODISCARD bool HasWidth() const noexcept
    {
      return _widthSet;
    }
    KRYS_NODISCARD bool HasHeight() const noexcept
    {
      return _heightSet;
    }
    KRYS_NODISCARD bool HasViewBox() const noexcept
    {
      return _viewBoxSet;
    }
    KRYS_NODISCARD bool HasPreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatioSet;
    }
    KRYS_NODISCARD bool HasPatternUnits() const noexcept
    {
      return _patternUnitsSet;
    }
    KRYS_NODISCARD bool HasPatternContentUnits() const noexcept
    {
      return _patternContentUnitsSet;
    }
    KRYS_NODISCARD bool HasPatternTransform() const noexcept
    {
      return _patternTransformSet;
    }
    KRYS_NODISCARD bool HasPatternContentElement() const noexcept
    {
      return _patternContentElementSet;
    }
  };
}
