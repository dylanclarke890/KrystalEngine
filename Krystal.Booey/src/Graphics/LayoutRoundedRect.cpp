#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>

namespace krys::boo
{
  bool LayoutRoundedRect::Radii::IsZero() const noexcept
  {
    return _topLeft.IsZero() && _topRight.IsZero() && _bottomLeft.IsZero() && _bottomRight.IsZero();
  }

  void LayoutRoundedRect::Radii::Scale(float factor) noexcept
  {
    if (factor == 1)
    {
      return;
    }

    // If either radius on a corner becomes zero, reset both radii on that corner.
    _topLeft.Scale(factor);
    if (!_topLeft.Width() || !_topLeft.Height())
    {
      _topLeft = LayoutSize();
    }
    _topRight.Scale(factor);
    if (!_topRight.Width() || !_topRight.Height())
    {
      _topRight = LayoutSize();
    }
    _bottomLeft.Scale(factor);
    if (!_bottomLeft.Width() || !_bottomLeft.Height())
    {
      _bottomLeft = LayoutSize();
    }
    _bottomRight.Scale(factor);
    if (!_bottomRight.Width() || !_bottomRight.Height())
    {
      _bottomRight = LayoutSize();
    }
  }

  void LayoutRoundedRect::Radii::Expand(LayoutUnit topWidth, LayoutUnit bottomWidth, LayoutUnit leftWidth,
                                        LayoutUnit rightWidth) noexcept
  {
    if (_topLeft.Width() > 0 && _topLeft.Height() > 0)
    {
      _topLeft.SetWidth(std::max<LayoutUnit>(0, _topLeft.Width() + leftWidth));
      _topLeft.SetHeight(std::max<LayoutUnit>(0, _topLeft.Height() + topWidth));
    }
    if (_topRight.Width() > 0 && _topRight.Height() > 0)
    {
      _topRight.SetWidth(std::max<LayoutUnit>(0, _topRight.Width() + rightWidth));
      _topRight.SetHeight(std::max<LayoutUnit>(0, _topRight.Height() + topWidth));
    }
    if (_bottomLeft.Width() > 0 && _bottomLeft.Height() > 0)
    {
      _bottomLeft.SetWidth(std::max<LayoutUnit>(0, _bottomLeft.Width() + leftWidth));
      _bottomLeft.SetHeight(std::max<LayoutUnit>(0, _bottomLeft.Height() + bottomWidth));
    }
    if (_bottomRight.Width() > 0 && _bottomRight.Height() > 0)
    {
      _bottomRight.SetWidth(std::max<LayoutUnit>(0, _bottomRight.Width() + rightWidth));
      _bottomRight.SetHeight(std::max<LayoutUnit>(0, _bottomRight.Height() + bottomWidth));
    }
  }

  void LayoutRoundedRect::InflateWithRadii(LayoutUnit amount) noexcept
  {
    LayoutRect old = _rect;

    if (amount < 0)
    {
      _rect.InflateX(std::max(-_rect.Width() / 2, amount));
      _rect.InflateY(std::max(-_rect.Height() / 2, amount));
    }
    else
    {
      _rect.Inflate(amount);
    }

    // Considering the inflation factor of shorter size to scale the radii seems appropriate here
    float factor;
    if (_rect.Width() < _rect.Height())
    {
      factor = old.Width() ? static_cast<float>(_rect.Width()) / old.Width() : 0.0f;
    }
    else
    {
      factor = old.Height() ? static_cast<float>(_rect.Height()) / old.Height() : 0.0f;
    }

    _radii.Scale(factor);
  }

  void LayoutRoundedRect::Radii::SetRadiiForEdges(const LayoutRoundedRect::Radii &radii,
                                                  RectEdges<bool> includeEdges) noexcept
  {
    if (includeEdges.Top())
    {
      if (includeEdges.Left())
      {
        _topLeft = radii.TopLeft();
      }
      if (includeEdges.Right())
      {
        _topRight = radii.TopRight();
      }
    }
    if (includeEdges.Bottom())
    {
      if (includeEdges.Left())
      {
        _bottomLeft = radii.BottomLeft();
      }
      if (includeEdges.Right())
      {
        _bottomRight = radii.BottomRight();
      }
    }
  }

  bool LayoutRoundedRect::Radii::AreRenderableInRect(const LayoutRect &rect) const noexcept
  {
    return TopLeft().Width() >= 0 && TopLeft().Height() >= 0 && BottomLeft().Width() >= 0
           && BottomLeft().Height() >= 0 && TopRight().Width() >= 0 && TopRight().Height() >= 0
           && BottomRight().Width() >= 0 && BottomRight().Height() >= 0
           && TopLeft().Width() + TopRight().Width() <= rect.Width()
           && BottomLeft().Width() + BottomRight().Width() <= rect.Width()
           && TopLeft().Height() + BottomLeft().Height() <= rect.Height()
           && TopRight().Height() + BottomRight().Height() <= rect.Height();
  }

  void LayoutRoundedRect::Radii::MakeRenderableInRect(const LayoutRect &rect) noexcept
  {
    auto maxRadiusWidth =
      std::max(TopLeft().Width() + TopRight().Width(), BottomLeft().Width() + BottomRight().Width());
    auto maxRadiusHeight =
      std::max(TopLeft().Height() + BottomLeft().Height(), TopRight().Height() + BottomRight().Height());

    if (maxRadiusWidth <= 0 || maxRadiusHeight <= 0)
    {
      Scale(0);
      return;
    }

    float widthRatio = static_cast<float>(rect.Width()) / maxRadiusWidth;
    float heightRatio = static_cast<float>(rect.Height()) / maxRadiusHeight;
    Scale(widthRatio < heightRatio ? widthRatio : heightRatio);

    if (!AreRenderableInRect(rect))
    {
      maxRadiusWidth =
        std::max(TopLeft().Width() + TopRight().Width(), BottomLeft().Width() + BottomRight().Width());
      maxRadiusHeight =
        std::max(TopLeft().Height() + BottomLeft().Height(), TopRight().Height() + BottomRight().Height());

      widthRatio = static_cast<float>(rect.Width()) / maxRadiusWidth;
      heightRatio = static_cast<float>(rect.Height()) / maxRadiusHeight;
      Scale(widthRatio < heightRatio ? widthRatio : heightRatio);
    }
  }

  LayoutRoundedRect::LayoutRoundedRect(LayoutUnit x, LayoutUnit y, LayoutUnit width,
                                       LayoutUnit height) noexcept
      : _rect(x, y, width, height)
  {
  }

  LayoutRoundedRect::LayoutRoundedRect(const LayoutRect &rect, const Radii &radii) noexcept
      : _rect(rect), _radii(radii)
  {
  }

  LayoutRoundedRect::LayoutRoundedRect(const LayoutRect &rect, const LayoutSize &topLeft,
                                       const LayoutSize &topRight, const LayoutSize &bottomLeft,
                                       const LayoutSize &bottomRight) noexcept
      : _rect(rect), _radii(topLeft, topRight, bottomLeft, bottomRight)
  {
  }

  bool LayoutRoundedRect::IsRenderable() const noexcept
  {
    return _radii.AreRenderableInRect(_rect);
  }

  void LayoutRoundedRect::AdjustRadii() noexcept
  {
    _radii.MakeRenderableInRect(_rect);
  }

  bool LayoutRoundedRect::IntersectsQuad(const FloatQuad &quad) const noexcept
  {
    FloatRect rect(_rect);
    if (!quad.IntersectsRect(rect))
    {
      return false;
    }

    const LayoutSize &topLeft = _radii.TopLeft();
    if (!topLeft.IsEmpty())
    {
      FloatRect cornerRect(_rect.X(), _rect.Y(), topLeft.Width(), topLeft.Height());
      if (quad.IntersectsRect(cornerRect))
      {
        FloatPoint center(_rect.X() + topLeft.Width(), _rect.Y() + topLeft.Height());
        FloatSize size(topLeft.Width(), topLeft.Height());
        if (!quad.IntersectsEllipse(center, size))
        {
          return false;
        }
      }
    }

    const LayoutSize &topRight = _radii.TopRight();
    if (!topRight.IsEmpty())
    {
      FloatRect cornerRect(_rect.MaxX() - topRight.Width(), _rect.Y(), topRight.Width(), topRight.Height());
      if (quad.IntersectsRect(cornerRect))
      {
        FloatPoint center(_rect.MaxX() - topRight.Width(), _rect.Y() + topRight.Height());
        FloatSize size(topRight.Width(), topRight.Height());
        if (!quad.IntersectsEllipse(center, size))
        {
          return false;
        }
      }
    }

    const LayoutSize &bottomLeft = _radii.BottomLeft();
    if (!bottomLeft.IsEmpty())
    {
      FloatRect cornerRect(_rect.X(), _rect.MaxY() - bottomLeft.Height(), bottomLeft.Width(),
                           bottomLeft.Height());
      if (quad.IntersectsRect(cornerRect))
      {
        FloatPoint center(_rect.X() + bottomLeft.Width(), _rect.MaxY() - bottomLeft.Height());
        FloatSize size(bottomLeft.Width(), bottomLeft.Height());
        if (!quad.IntersectsEllipse(center, size))
        {
          return false;
        }
      }
    }

    const LayoutSize &bottomRight = _radii.BottomRight();
    if (!bottomRight.IsEmpty())
    {
      FloatRect cornerRect(_rect.MaxX() - bottomRight.Width(), _rect.MaxY() - bottomRight.Height(),
                           bottomRight.Width(), bottomRight.Height());
      if (quad.IntersectsRect(cornerRect))
      {
        FloatPoint center(_rect.MaxX() - bottomRight.Width(), _rect.MaxY() - bottomRight.Height());
        FloatSize size(bottomRight.Width(), bottomRight.Height());
        if (!quad.IntersectsEllipse(center, size))
        {
          return false;
        }
      }
    }

    return true;
  }

  bool LayoutRoundedRect::Contains(const LayoutRect &otherRect) const noexcept
  {
    if (!Rect().Contains(otherRect) || !IsRenderable())
    {
      return false;
    }

    const LayoutSize &topLeft = _radii.TopLeft();
    if (!topLeft.IsEmpty())
    {
      FloatPoint center = {_rect.X() + topLeft.Width(), _rect.Y() + topLeft.Height()};
      if (otherRect.X() <= center.X() && otherRect.Y() <= center.Y())
      {
        if (!EllipseContainsPoint(center, topLeft, otherRect.MinXMinYCorner()))
        {
          return false;
        }
      }
    }

    const LayoutSize &topRight = _radii.TopRight();
    if (!topRight.IsEmpty())
    {
      FloatPoint center = {_rect.MaxX() - topRight.Width(), _rect.Y() + topRight.Height()};
      if (otherRect.MaxX() >= center.X() && otherRect.Y() <= center.Y())
      {
        if (!EllipseContainsPoint(center, topRight, otherRect.MaxXMinYCorner()))
        {
          return false;
        }
      }
    }

    const LayoutSize &bottomLeft = _radii.BottomLeft();
    if (!bottomLeft.IsEmpty())
    {
      FloatPoint center = {_rect.X() + bottomLeft.Width(), _rect.MaxY() - bottomLeft.Height()};
      if (otherRect.X() <= center.X() && otherRect.MaxY() >= center.Y())
      {
        if (!EllipseContainsPoint(center, bottomLeft, otherRect.MinXMaxYCorner()))
        {
          return false;
        }
      }
    }

    const LayoutSize &bottomRight = _radii.BottomRight();
    if (!bottomRight.IsEmpty())
    {
      FloatPoint center = {_rect.MaxX() - bottomRight.Width(), _rect.MaxY() - bottomRight.Height()};
      if (otherRect.MaxX() >= center.X() && otherRect.MaxY() >= center.Y())
      {
        if (!EllipseContainsPoint(center, bottomRight, otherRect.MaxXMaxYCorner()))
        {
          return false;
        }
      }
    }

    return true;
  }

  FloatRoundedRect
    LayoutRoundedRect::PixelSnappedRoundedRectForPainting(float deviceScaleFactor) const noexcept
  {
    LayoutRect originalRect = Rect();
    if (originalRect.IsEmpty())
    {
      return FloatRoundedRect(originalRect, GetRadii());
    }

    FloatRect pixelSnappedRect = SnapRectToDevicePixels(originalRect, deviceScaleFactor);

    if (!IsRenderable())
    {
      return FloatRoundedRect(pixelSnappedRect, GetRadii());
    }

    // Snapping usually does not alter size, but when it does, we need to make sure that the final rect is
    // still renderable by distributing the size delta proportionally.
    FloatRoundedRect::Radii adjustedRadii = GetRadii();
    adjustedRadii.Scale(pixelSnappedRect.Width() / originalRect.Width().ToFloat(),
                        pixelSnappedRect.Height() / originalRect.Height().ToFloat());
    FloatRoundedRect snappedRoundedRect = FloatRoundedRect(pixelSnappedRect, adjustedRadii);
    if (!snappedRoundedRect.IsRenderable())
    {
      // Floating point mantissa overflow can produce a non-renderable rounded rect.
      adjustedRadii.Shrink(1 / deviceScaleFactor);
      snappedRoundedRect.SetRadii(adjustedRadii);
    }
    krys_debug_assert(snappedRoundedRect.IsRenderable());
    return snappedRoundedRect;
  }

  TextStream &operator<<(TextStream &ts, const LayoutRoundedRect &roundedRect) noexcept
  {
    ts << roundedRect.Rect();
    ts.DumpProperty(u8"top-left", roundedRect.GetRadii().TopLeft());
    ts.DumpProperty(u8"top-right", roundedRect.GetRadii().TopRight());
    ts.DumpProperty(u8"bottom-left", roundedRect.GetRadii().BottomLeft());
    ts.DumpProperty(u8"bottom-right", roundedRect.GetRadii().BottomRight());
    return ts;
  }
}
