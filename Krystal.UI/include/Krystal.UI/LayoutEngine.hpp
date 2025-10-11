#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Element.hpp"

namespace Krys::UI
{
  struct LayoutContext
  {
    BoundingBox Viewport {};
    Maths::Vec2 ParentSize {0.f, 0.f};
  };

  class LayoutEngine
  {
    NO_COPY_MOVE(LayoutEngine)

  public:
    LayoutEngine() noexcept = default;

    ~LayoutEngine() noexcept = default;

    void ComputeLayout(Body &body, const BoundingBox &viewport) const
    {
      ComputeElementLayout(body, viewport);
    }

    float ResolveSizeUnit(const SizeUnit &sizeUnit, float parentSize) const noexcept
    {
      switch (sizeUnit.Type)
      {
        case SizeUnitType::Points:
          // Assuming 96 DPI TODO: get from window events
          return sizeUnit.Value * (96.f / 72.f);
        case SizeUnitType::Percentage: return (sizeUnit.Value / 100.f) * parentSize;
        case SizeUnitType::Pixels:
        default:                       return sizeUnit.Value;
      }
    }

  private:
    void ComputeElementLayout(Element &element, const BoundingBox &parentContentBounds) const
    {
      const float parentWidth = parentContentBounds.GetWidth();
      const float parentHeight = parentContentBounds.GetHeight();

      const auto &props = element._properties;
      auto &layout = element._layout;

      layout.Padding.Left = ResolveSizeUnit(props.Padding.Left, parentWidth);
      layout.Padding.Right = ResolveSizeUnit(props.Padding.Right, parentWidth);
      layout.Padding.Top = ResolveSizeUnit(props.Padding.Top, parentHeight);
      layout.Padding.Bottom = ResolveSizeUnit(props.Padding.Bottom, parentHeight);

      layout.BorderWidth.Left = ResolveSizeUnit(props.Border.Left.Width, parentWidth);
      layout.BorderWidth.Right = ResolveSizeUnit(props.Border.Right.Width, parentWidth);
      layout.BorderWidth.Top = ResolveSizeUnit(props.Border.Top.Width, parentHeight);
      layout.BorderWidth.Bottom = ResolveSizeUnit(props.Border.Bottom.Width, parentHeight);

      layout.Margin.Left = ResolveSizeUnit(props.Margin.Left, parentWidth);
      layout.Margin.Right = ResolveSizeUnit(props.Margin.Right, parentWidth);
      layout.Margin.Top = ResolveSizeUnit(props.Margin.Top, parentHeight);
      layout.Margin.Bottom = ResolveSizeUnit(props.Margin.Bottom, parentHeight);

      // width and height specify the border-box size, includes border and padding.
      const float borderBoxWidth = ResolveSizeUnit(props.Width, parentWidth);
      const float borderBoxHeight = ResolveSizeUnit(props.Height, parentHeight);

      // get the total bounds by adding margin
      const float boundsWidth = borderBoxWidth + layout.Margin.Horizontal();
      const float boundsHeight = borderBoxHeight + layout.Margin.Vertical();

      // get the content size by removing padding and border
      const float contentWidth =
        borderBoxWidth - layout.Padding.Horizontal() - layout.BorderWidth.Horizontal();
      const float contentHeight = borderBoxHeight - layout.Padding.Vertical() - layout.BorderWidth.Vertical();

      const float relativeX = ResolveSizeUnit(props.RelativeX, parentWidth);
      const float relativeY = ResolveSizeUnit(props.RelativeY, parentHeight);

      // TODO: anchor point, assume top-left for now
      const float x = parentContentBounds.X + layout.Margin.Left + relativeX;
      const float y = parentContentBounds.Y + layout.Margin.Top + relativeY;

      layout.Bounds = {x, y, x + boundsWidth, y + boundsHeight};
      layout.FontSizePx = ResolveSizeUnit(props.Text.FontSize, parentHeight);
    }
  };
}