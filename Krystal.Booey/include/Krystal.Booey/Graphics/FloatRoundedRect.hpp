#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>

namespace krys::boo
{
  class Path;

  /// @brief A rectangle with rounded corners.
  class FloatRoundedRect
  {
  public:
    /// @brief The four corner radii.
    class Radii
    {
    private:
      FloatSize _topLeft;
      FloatSize _topRight;
      FloatSize _bottomLeft;
      FloatSize _bottomRight;

    public:
      Radii() noexcept = default;
      Radii(const FloatSize &topLeft, const FloatSize &topRight, const FloatSize &bottomLeft,
            const FloatSize &bottomRight) noexcept
          : _topLeft(topLeft), _topRight(topRight), _bottomLeft(bottomLeft), _bottomRight(bottomRight)
      {
      }

      Radii(const LayoutRoundedRect::Radii &intRadii) noexcept
          : _topLeft(intRadii.TopLeft()), _topRight(intRadii.TopRight()), _bottomLeft(intRadii.BottomLeft()),
            _bottomRight(intRadii.BottomRight())
      {
      }

      explicit Radii(float uniformRadius) noexcept
          : _topLeft(uniformRadius, uniformRadius), _topRight(uniformRadius, uniformRadius),
            _bottomLeft(uniformRadius, uniformRadius), _bottomRight(uniformRadius, uniformRadius)
      {
      }

      explicit Radii(float uniformRadiusWidth, float uniformRadiusHeight) noexcept
          : _topLeft(uniformRadiusWidth, uniformRadiusHeight),
            _topRight(uniformRadiusWidth, uniformRadiusHeight),
            _bottomLeft(uniformRadiusWidth, uniformRadiusHeight),
            _bottomRight(uniformRadiusWidth, uniformRadiusHeight)
      {
      }

      void SetTopLeft(const FloatSize &size) noexcept
      {
        _topLeft = size;
      }
      void SetTopRight(const FloatSize &size) noexcept
      {
        _topRight = size;
      }
      void SetBottomLeft(const FloatSize &size) noexcept
      {
        _bottomLeft = size;
      }
      void SetBottomRight(const FloatSize &size) noexcept
      {
        _bottomRight = size;
      }
      KRYS_NODISCARD const FloatSize &TopLeft() const noexcept
      {
        return _topLeft;
      }
      KRYS_NODISCARD const FloatSize &TopRight() const noexcept
      {
        return _topRight;
      }
      KRYS_NODISCARD const FloatSize &BottomLeft() const noexcept
      {
        return _bottomLeft;
      }
      KRYS_NODISCARD const FloatSize &BottomRight() const noexcept
      {
        return _bottomRight;
      }

      KRYS_NODISCARD bool IsZero() const noexcept;
      KRYS_NODISCARD bool HasEvenCorners() const noexcept;
      KRYS_NODISCARD bool IsUniformCornerRadius() const noexcept; // Including no radius.

      void Scale(float factor) noexcept;
      void Scale(float horizontalFactor, float verticalFactor) noexcept;
      void ExpandEvenIfZero(float size) noexcept;
      void Expand(float topWidth, float bottomWidth, float leftWidth, float rightWidth) noexcept;
      void Expand(float size) noexcept
      {
        Expand(size, size, size, size);
      }
      void Shrink(float topWidth, float bottomWidth, float leftWidth, float rightWidth) noexcept
      {
        Expand(-topWidth, -bottomWidth, -leftWidth, -rightWidth);
      }
      void Shrink(float size) noexcept
      {
        Shrink(size, size, size, size);
      }

      KRYS_NODISCARD bool operator==(const Radii &) const noexcept = default;
    };

  private:
    FloatRect _rect;
    Radii _radii;

  public:
    explicit FloatRoundedRect(const FloatRect & = FloatRect(), const Radii & = Radii()) noexcept;
    FloatRoundedRect(const FloatRect &, const FloatSize &topLeft, const FloatSize &topRight,
                     const FloatSize &bottomLeft, const FloatSize &bottomRight) noexcept;
    explicit FloatRoundedRect(const LayoutRoundedRect &) noexcept;
    FloatRoundedRect(float x, float y, float width, float height) noexcept;

    KRYS_NODISCARD const FloatRect &Rect() const noexcept
    {
      return _rect;
    }
    KRYS_NODISCARD const Radii &GetRadii() const noexcept
    {
      return _radii;
    }
    KRYS_NODISCARD bool IsRounded() const noexcept
    {
      return !_radii.IsZero();
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _rect.IsEmpty();
    }

    void SetRect(const FloatRect &rect) noexcept
    {
      _rect = rect;
    }
    void SetLocation(FloatPoint location) noexcept
    {
      _rect.SetLocation(location);
    }
    void SetRadii(const Radii &radii) noexcept
    {
      _radii = radii;
    }

    void Move(const FloatSize &size) noexcept
    {
      _rect.Move(size);
    }
    void Inflate(float size) noexcept
    {
      _rect.Inflate(size);
    }
    void ExpandRadii(float size) noexcept
    {
      _radii.Expand(size);
    }
    void ShrinkRadii(float size) noexcept
    {
      _radii.Shrink(size);
    }
    void InflateWithRadii(float size) noexcept;
    void AdjustRadii() noexcept;

    KRYS_NODISCARD FloatRect TopLeftCorner() const noexcept
    {
      return FloatRect(_rect.X(), _rect.Y(), _radii.TopLeft().Width(), _radii.TopLeft().Height());
    }
    KRYS_NODISCARD FloatRect TopRightCorner() const noexcept
    {
      return FloatRect(_rect.MaxX() - _radii.TopRight().Width(), _rect.Y(), _radii.TopRight().Width(),
                       _radii.TopRight().Height());
    }
    KRYS_NODISCARD FloatRect BottomLeftCorner() const noexcept
    {
      return FloatRect(_rect.X(), _rect.MaxY() - _radii.BottomLeft().Height(), _radii.BottomLeft().Width(),
                       _radii.BottomLeft().Height());
    }
    KRYS_NODISCARD FloatRect BottomRightCorner() const noexcept
    {
      return FloatRect(_rect.MaxX() - _radii.BottomRight().Width(),
                       _rect.MaxY() - _radii.BottomRight().Height(), _radii.BottomRight().Width(),
                       _radii.BottomRight().Height());
    }

    KRYS_NODISCARD bool IsRenderable() const noexcept;
    KRYS_NODISCARD bool XInterceptsAtY(float y, float &minXIntercept, float &maxXIntercept) const noexcept;

    KRYS_NODISCARD bool IntersectionIsRectangular(const FloatRect &) const noexcept;

    /// @brief The rounded rect as a path.
    KRYS_NODISCARD Path GetPath() const noexcept;

    KRYS_NODISCARD bool operator==(const FloatRoundedRect &) const noexcept = default;
  };

  KRYS_NODISCARD inline float CalcBorderRadiiConstraintScaleFor(const FloatRect &rect,
                                                                const FloatRoundedRect::Radii &radii) noexcept
  {
    // Constrain corner radii using CSS3 rules:
    // http://www.w3.org/TR/css3-background/#the-border-radius

    float factor = 1;
    float radiiSum;

    // top
    radiiSum = radii.TopLeft().Width() + radii.TopRight().Width(); // Casts to avoid integer overflow.
    if (radiiSum > rect.Width())
    {
      factor = std::min(rect.Width() / radiiSum, factor);
    }

    // bottom
    radiiSum = radii.BottomLeft().Width() + radii.BottomRight().Width();
    if (radiiSum > rect.Width())
    {
      factor = std::min(rect.Width() / radiiSum, factor);
    }

    // left
    radiiSum = radii.TopLeft().Height() + radii.BottomLeft().Height();
    if (radiiSum > rect.Height())
    {
      factor = std::min(rect.Height() / radiiSum, factor);
    }

    // right
    radiiSum = radii.TopRight().Height() + radii.BottomRight().Height();
    if (radiiSum > rect.Height())
    {
      factor = std::min(rect.Height() / radiiSum, factor);
    }

    krys_debug_assert(factor <= 1);
    return factor;
  }

  // Snip away rectangles from corners, roughly one per step length of arc.
  KRYS_NODISCARD Region ApproximateAsRegion(const FloatRoundedRect &, unsigned stepLength = 20) noexcept;
}
