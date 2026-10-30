#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief The flex container's constraints along its main and cross axes.
  class ConstraintsForFlexContent
  {
  public:
    struct AxisGeometry
    {
      Maybe<LayoutUnit> MinimumSize;
      Maybe<LayoutUnit> MaximumSize;
      Maybe<LayoutUnit> AvailableSize;
      LayoutUnit StartPosition;
    };

  private:
    AxisGeometry _mainAxisGeometry;
    AxisGeometry _crossAxisGeometry;
    bool _isSizedUnderMinMax {false};

  public:
    ConstraintsForFlexContent(const AxisGeometry &mainAxis, const AxisGeometry &crossAxis,
                              bool isSizedUnderMinMax) noexcept
        : _mainAxisGeometry(mainAxis), _crossAxisGeometry(crossAxis), _isSizedUnderMinMax(isSizedUnderMinMax)
    {
    }

    KRYS_NODISCARD const AxisGeometry &MainAxis() const noexcept
    {
      return _mainAxisGeometry;
    }
    KRYS_NODISCARD const AxisGeometry &CrossAxis() const noexcept
    {
      return _crossAxisGeometry;
    }
    KRYS_NODISCARD bool IsSizedUnderMinMax() const noexcept
    {
      return _isSizedUnderMinMax;
    }
  };
}
