#pragma once

#include "Krystal.Booey/SVG/GradientAttributes.hpp"
#include "Krystal.Booey/SVG/SVGLengthValue.hpp"

namespace krys::boo::svg
{
  /// @brief A linear gradient's attributes, collected from it and the gradients it references.
  struct LinearGradientAttributes : GradientAttributes
  {
  private:
    // Properties
    SVGLengthValue _x1;
    SVGLengthValue _y1;
    SVGLengthValue _x2 {SVGLengthMode::Width, u8"100%"};
    SVGLengthValue _y2;

    // Property states
    bool _x1Set : 1 {false};
    bool _y1Set : 1 {false};
    bool _x2Set : 1 {false};
    bool _y2Set : 1 {false};

  public:
    KRYS_NODISCARD SVGLengthValue X1() const noexcept
    {
      return _x1;
    }
    KRYS_NODISCARD SVGLengthValue Y1() const noexcept
    {
      return _y1;
    }
    KRYS_NODISCARD SVGLengthValue X2() const noexcept
    {
      return _x2;
    }
    KRYS_NODISCARD SVGLengthValue Y2() const noexcept
    {
      return _y2;
    }

    void SetX1(SVGLengthValue value) noexcept
    {
      _x1 = value;
      _x1Set = true;
    }
    void SetY1(SVGLengthValue value) noexcept
    {
      _y1 = value;
      _y1Set = true;
    }
    void SetX2(SVGLengthValue value) noexcept
    {
      _x2 = value;
      _x2Set = true;
    }
    void SetY2(SVGLengthValue value) noexcept
    {
      _y2 = value;
      _y2Set = true;
    }

    KRYS_NODISCARD bool HasX1() const noexcept
    {
      return _x1Set;
    }
    KRYS_NODISCARD bool HasY1() const noexcept
    {
      return _y1Set;
    }
    KRYS_NODISCARD bool HasX2() const noexcept
    {
      return _x2Set;
    }
    KRYS_NODISCARD bool HasY2() const noexcept
    {
      return _y2Set;
    }
  };
}
