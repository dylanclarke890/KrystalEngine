#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::boo
{
  FloatRoundedRect::FloatRoundedRect(const LayoutRoundedRect &rect) noexcept
      : _rect(rect.Rect()), _radii(rect.GetRadii())
  {
  }

  FloatRoundedRect::FloatRoundedRect(float x, float y, float width, float height) noexcept
      : _rect(x, y, width, height)
  {
  }

  FloatRoundedRect::FloatRoundedRect(const FloatRect &rect, const Radii &radii) noexcept
      : _rect(rect), _radii(radii)
  {
  }

  FloatRoundedRect::FloatRoundedRect(const FloatRect &rect, const FloatSize &topLeft,
                                     const FloatSize &topRight, const FloatSize &bottomLeft,
                                     const FloatSize &bottomRight) noexcept
      : _rect(rect), _radii(topLeft, topRight, bottomLeft, bottomRight)
  {
  }

  bool FloatRoundedRect::Radii::IsZero() const noexcept
  {
    return _topLeft.IsZero() && _topRight.IsZero() && _bottomLeft.IsZero() && _bottomRight.IsZero();
  }

  bool FloatRoundedRect::Radii::HasEvenCorners() const noexcept
  {
    return AreEssentiallyEqual(_topLeft, _topRight) && AreEssentiallyEqual(_topLeft, _bottomLeft)
           && AreEssentiallyEqual(_topLeft, _bottomRight);
  }

  bool FloatRoundedRect::Radii::IsUniformCornerRadius() const noexcept
  {
    return AreEssentiallyEqual(_topLeft.Width(), _topLeft.Height()) && HasEvenCorners();
  }

  void FloatRoundedRect::Radii::Scale(float factor) noexcept
  {
    Scale(factor, factor);
  }

  void FloatRoundedRect::Radii::Scale(float horizontalFactor, float verticalFactor) noexcept
  {
    if (horizontalFactor == 1 && verticalFactor == 1)
    {
      return;
    }

    // If either radius on a corner becomes zero, reset both radii on that corner.
    _topLeft.Scale(horizontalFactor, verticalFactor);
    if (!_topLeft.Width() || !_topLeft.Height())
    {
      _topLeft = FloatSize();
    }
    _topRight.Scale(horizontalFactor, verticalFactor);
    if (!_topRight.Width() || !_topRight.Height())
    {
      _topRight = FloatSize();
    }
    _bottomLeft.Scale(horizontalFactor, verticalFactor);
    if (!_bottomLeft.Width() || !_bottomLeft.Height())
    {
      _bottomLeft = FloatSize();
    }
    _bottomRight.Scale(horizontalFactor, verticalFactor);
    if (!_bottomRight.Width() || !_bottomRight.Height())
    {
      _bottomRight = FloatSize();
    }
  }

  void FloatRoundedRect::Radii::Expand(float topWidth, float bottomWidth, float leftWidth,
                                       float rightWidth) noexcept
  {
    if (_topLeft.Width() > 0 && _topLeft.Height() > 0)
    {
      _topLeft.SetWidth(std::max<float>(0, _topLeft.Width() + leftWidth));
      _topLeft.SetHeight(std::max<float>(0, _topLeft.Height() + topWidth));
    }
    if (_topRight.Width() > 0 && _topRight.Height() > 0)
    {
      _topRight.SetWidth(std::max<float>(0, _topRight.Width() + rightWidth));
      _topRight.SetHeight(std::max<float>(0, _topRight.Height() + topWidth));
    }
    if (_bottomLeft.Width() > 0 && _bottomLeft.Height() > 0)
    {
      _bottomLeft.SetWidth(std::max<float>(0, _bottomLeft.Width() + leftWidth));
      _bottomLeft.SetHeight(std::max<float>(0, _bottomLeft.Height() + bottomWidth));
    }
    if (_bottomRight.Width() > 0 && _bottomRight.Height() > 0)
    {
      _bottomRight.SetWidth(std::max<float>(0, _bottomRight.Width() + rightWidth));
      _bottomRight.SetHeight(std::max<float>(0, _bottomRight.Height() + bottomWidth));
    }
  }

  void FloatRoundedRect::Radii::ExpandEvenIfZero(float size) noexcept
  {
    auto expand = [&](auto &corner)
    {
      corner.SetWidth(std::max(0.f, corner.Width() + size));
      corner.SetHeight(std::max(0.f, corner.Height() + size));
    };

    expand(_topLeft);
    expand(_topRight);
    expand(_bottomLeft);
    expand(_bottomRight);
  }

  KRYS_NODISCARD static inline float CornerRectIntercept(float y, const FloatRect &cornerRect) noexcept
  {
    krys_debug_assert(cornerRect.Height() > 0);
    return cornerRect.Width() * sqrt(1 - (y * y) / (cornerRect.Height() * cornerRect.Height()));
  }

  bool FloatRoundedRect::XInterceptsAtY(float y, float &minXIntercept, float &maxXIntercept) const noexcept
  {
    if (y < Rect().Y() || y > Rect().MaxY())
    {
      return false;
    }

    if (!IsRounded())
    {
      minXIntercept = Rect().X();
      maxXIntercept = Rect().MaxX();
      return true;
    }

    const FloatRect &topLeftRect = TopLeftCorner();
    const FloatRect &bottomLeftRect = BottomLeftCorner();

    if (!topLeftRect.IsEmpty() && y >= topLeftRect.Y() && y < topLeftRect.MaxY())
    {
      minXIntercept = topLeftRect.MaxX() - CornerRectIntercept(topLeftRect.MaxY() - y, topLeftRect);
    }
    else if (!bottomLeftRect.IsEmpty() && y >= bottomLeftRect.Y() && y <= bottomLeftRect.MaxY())
    {
      minXIntercept = bottomLeftRect.MaxX() - CornerRectIntercept(y - bottomLeftRect.Y(), bottomLeftRect);
    }
    else
    {
      minXIntercept = _rect.X();
    }

    const FloatRect &topRightRect = TopRightCorner();
    const FloatRect &bottomRightRect = BottomRightCorner();

    if (!topRightRect.IsEmpty() && y >= topRightRect.Y() && y <= topRightRect.MaxY())
    {
      maxXIntercept = topRightRect.X() + CornerRectIntercept(topRightRect.MaxY() - y, topRightRect);
    }
    else if (!bottomRightRect.IsEmpty() && y >= bottomRightRect.Y() && y <= bottomRightRect.MaxY())
    {
      maxXIntercept = bottomRightRect.X() + CornerRectIntercept(y - bottomRightRect.Y(), bottomRightRect);
    }
    else
    {
      maxXIntercept = _rect.MaxX();
    }

    return true;
  }

  bool FloatRoundedRect::IsRenderable() const noexcept
  {
    return _radii.TopLeft().Width() >= 0 && _radii.TopLeft().Height() >= 0 && _radii.BottomLeft().Width() >= 0
           && _radii.BottomLeft().Height() >= 0 && _radii.TopRight().Width() >= 0
           && _radii.TopRight().Height() >= 0 && _radii.BottomRight().Width() >= 0
           && _radii.BottomRight().Height() >= 0
           && _radii.TopLeft().Width() + _radii.TopRight().Width() <= _rect.Width()
           && _radii.BottomLeft().Width() + _radii.BottomRight().Width() <= _rect.Width()
           && _radii.TopLeft().Height() + _radii.BottomLeft().Height() <= _rect.Height()
           && _radii.TopRight().Height() + _radii.BottomRight().Height() <= _rect.Height();
  }

  void FloatRoundedRect::InflateWithRadii(float size) noexcept
  {
    FloatRect old = _rect;

    _rect.Inflate(size);
    // Considering the inflation factor of shorter size to scale the radii seems appropriate here
    float factor;
    if (_rect.Width() < _rect.Height())
    {
      factor = old.Width() ? _rect.Width() / old.Width() : 0;
    }
    else
    {
      factor = old.Height() ? _rect.Height() / old.Height() : 0;
    }

    _radii.Scale(factor);
  }

  void FloatRoundedRect::AdjustRadii() noexcept
  {
    float maxRadiusWidth = std::max(_radii.TopLeft().Width() + _radii.TopRight().Width(),
                                    _radii.BottomLeft().Width() + _radii.BottomRight().Width());
    float maxRadiusHeight = std::max(_radii.TopLeft().Height() + _radii.BottomLeft().Height(),
                                     _radii.TopRight().Height() + _radii.BottomRight().Height());

    if (maxRadiusWidth <= 0 || maxRadiusHeight <= 0)
    {
      _radii.Scale(0.0f);
      return;
    }
    float widthRatio = _rect.Width() / maxRadiusWidth;
    float heightRatio = _rect.Height() / maxRadiusHeight;
    _radii.Scale(widthRatio < heightRatio ? widthRatio : heightRatio);
  }

  // This is conservative; it does not test intrusion into the corner rects.
  bool FloatRoundedRect::IntersectionIsRectangular(const FloatRect &rect) const noexcept
  {
    return !(rect.Intersects(TopLeftCorner()) || rect.Intersects(TopRightCorner())
             || rect.Intersects(BottomLeftCorner()) || rect.Intersects(BottomRightCorner()));
  }

  Path FloatRoundedRect::GetPath() const noexcept
  {
    Path path;
    path.AddRoundedRect(*this);
    return path;
  }

  Region ApproximateAsRegion(const FloatRoundedRect &roundedRect, unsigned stepLength) noexcept
  {
    Region region;

    if (roundedRect.IsEmpty())
    {
      return region;
    }

    auto rect = LayoutRect(roundedRect.Rect());
    region.Unite(EnclosingIntRect(rect));

    if (!roundedRect.IsRounded())
    {
      return region;
    }

    auto &radii = roundedRect.GetRadii();

    auto makeIntRect = [](LayoutPoint a, LayoutPoint b)
    {
      return EnclosingIntRect(LayoutRect {LayoutPoint {std::min(a.X(), b.X()), std::min(a.Y(), b.Y())},
                                          LayoutPoint {std::max(a.X(), b.X()), std::max(a.Y(), b.Y())}});
    };

    auto subtractCornerRects =
      [&](LayoutPoint corner, LayoutPoint ellipsisCenter, FloatSize axes, double fromAngle)
    {
      double toAngle = fromAngle + std::numbers::pi / 2;

      // Substract more rects for longer, more rounded arcs.
      auto arcLengthFactor = RoundToInt(std::min(axes.Width(), axes.Height()));
      auto count = (arcLengthFactor + (stepLength / 2)) / stepLength;

      constexpr auto maximumCount = 20u;
      count = std::min(maximumCount, count);

      for (decltype(count) i = 0; i < count; ++i)
      {
        auto angle = fromAngle + (i + 1) * (toAngle - fromAngle) / (count + 1);
        auto ellipsisPoint = LayoutPoint {axes.Width() * std::cos(angle), axes.Height() * std::sin(angle)};
        auto cornerRect = makeIntRect(corner, ellipsisCenter + ellipsisPoint);
        region.Subtract(cornerRect);
      }
    };

    {
      auto corner = rect.MaxXMaxYCorner();
      auto axes = radii.BottomRight();
      auto ellipsisCenter = LayoutPoint(corner.X() - axes.Width(), corner.Y() - axes.Height());
      subtractCornerRects(corner, ellipsisCenter, axes, 0);
    }

    {
      auto corner = rect.MinXMaxYCorner();
      auto axes = radii.BottomLeft();
      auto ellipsisCenter = LayoutPoint(corner.X() + axes.Width(), corner.Y() - axes.Height());
      subtractCornerRects(corner, ellipsisCenter, axes, std::numbers::pi / 2);
    }

    {
      auto corner = rect.MinXMinYCorner();
      auto axes = radii.TopLeft();
      auto ellipsisCenter = LayoutPoint(corner.X() + axes.Width(), corner.Y() + axes.Height());
      subtractCornerRects(corner, ellipsisCenter, axes, std::numbers::pi);
    }

    {
      auto corner = rect.MaxXMinYCorner();
      auto axes = radii.TopRight();
      auto ellipsisCenter = LayoutPoint(corner.X() - axes.Width(), corner.Y() + axes.Height());
      subtractCornerRects(corner, ellipsisCenter, axes, std::numbers::pi * 3 / 2);
    }

    return region;
  }

  TextStream &operator<<(TextStream &ts, const FloatRoundedRect &roundedRect) noexcept
  {
    ts << roundedRect.Rect();
    ts.DumpProperty(u8"top-left", roundedRect.GetRadii().TopLeft());
    ts.DumpProperty(u8"top-right", roundedRect.GetRadii().TopRight());
    ts.DumpProperty(u8"bottom-left", roundedRect.GetRadii().BottomLeft());
    ts.DumpProperty(u8"bottom-right", roundedRect.GetRadii().BottomRight());
    return ts;
  }
}
