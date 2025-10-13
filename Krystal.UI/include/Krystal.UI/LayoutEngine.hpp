#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Queue.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Element.hpp"
#include "Krystal.UI/ElementPool.hpp"

namespace Krys::UI
{
  // NOTE: shouldn't allow percentage for border
  // NOTE: margin and padding percentage is relative to the parent width, not height

  class LayoutEngine
  {
    NO_COPY_MOVE(LayoutEngine)

  private:
    ElementPool &_elementPool;

  public:
    LayoutEngine(ElementPool &elementPool) noexcept : _elementPool(elementPool)
    {
    }

    ~LayoutEngine() noexcept = default;

    void ComputeLayout(Body &body, const BoundingBox &viewport)
    {
      Measure(body, Unit(viewport.Width, Unit::Pixels), Unit(viewport.Height, Unit::Pixels));
    }

  private:
    constexpr Unit ConvertToPixels(const Unit &unit, float reference = 0.f) const noexcept
    {
      switch (unit.Type)
      {
        // TODO: Assumes 96 DPI for conversion
        case Unit::Points:     return Unit(unit.Value * (96.0f / 72.0f), Unit::Pixels);
        case Unit::Pixels:     return Unit(unit.Value, Unit::Pixels);
        case Unit::Percentage: return Unit((reference * unit.Value) / 100.f, Unit::Pixels);
        case Unit::Auto:
        default:               return Auto; // Auto cannot be directly converted
      }
    }

    void Measure(Element &element, Unit parentWidth, Unit parentHeight)
    {
      const Properties &props = element.GetProperties();
      ComputedLayout &layout = element.ComputedLayout();

      // TODO:
      // if (props.Display == DisplayType::None)
      //{
      //  layout.Width = Unit(0.f, Unit::Pixels);
      //  layout.Height = Unit(0.f, Unit::Pixels);
      //}

      if (props.Width.IsPercentage())
      {
        if (parentWidth.IsFixed())
        {
          layout.IntrinsicWidth = ConvertToPixels(props.Width, ConvertToPixels(parentWidth).Value);
        }
        else
        {
          layout.IntrinsicWidth = Auto; // Cannot resolve percentage, treat as Auto.
        }
      }
      else if (props.Width.IsFixed())
      {
        layout.IntrinsicWidth = ConvertToPixels(props.Width);
      }
      else if (props.Width.IsAuto())
      {
        layout.IntrinsicWidth = Auto; // Auto, will be calculated later.
      }

      if (props.Height.IsPercentage())
      {
        if (parentHeight.IsFixed())
        {
          layout.IntrinsicHeight = ConvertToPixels(props.Height, ConvertToPixels(parentHeight).Value);
        }
        else
        {
          layout.IntrinsicHeight = Auto; // Cannot resolve percentage, treat as Auto.
        }
      }
      else if (props.Height.IsFixed())
      {
        layout.IntrinsicHeight = ConvertToPixels(props.Height);
      }
      else if (props.Height.IsAuto())
      {
        layout.IntrinsicHeight = Auto; // Auto, will be calculated later.
      }

      // ... Calculate margin, padding, border, gap, font size, etc.

      for (const ElementHandle &childHandle : element.GetChildren())
      {
        Element *child = _elementPool.TryGet<Element>(childHandle);
        Measure(*child, layout.IntrinsicWidth, layout.IntrinsicHeight);
      }
    }
  };
}