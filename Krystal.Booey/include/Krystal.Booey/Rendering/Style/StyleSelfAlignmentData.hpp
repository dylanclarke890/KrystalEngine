#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  class LayoutUnit;
  class WritingMode;

  enum class LogicalBoxAxis : uint8;

  class StyleSelfAlignmentData
  {
  public:
    constexpr StyleSelfAlignmentData() noexcept = default;

    // Style data for Self-Alignment and Default-Alignment properties: align-{self, items}, justify-{self,
    // items}. [ <self-position> && <overflow-position>? ] | [ legacy && [ left | right | center ] ]
    constexpr StyleSelfAlignmentData(ItemPosition position,
                                     OverflowAlignment overflow = OverflowAlignment::Default,
                                     ItemPositionType positionType = ItemPositionType::NonLegacy) noexcept
        : _position(ToUnderlying(position)), _positionType(ToUnderlying(positionType)),
          _overflow(ToUnderlying(overflow))
    {
    }

    void SetPosition(ItemPosition position) noexcept
    {
      _position = ToUnderlying(position);
    }
    void SetPositionType(ItemPositionType positionType) noexcept
    {
      _positionType = ToUnderlying(positionType);
    }
    void SetOverflow(OverflowAlignment overflow) noexcept
    {
      _overflow = ToUnderlying(overflow);
    }

    KRYS_NODISCARD ItemPosition Position() const noexcept
    {
      return static_cast<ItemPosition>(_position);
    }
    KRYS_NODISCARD ItemPositionType PositionType() const noexcept
    {
      return static_cast<ItemPositionType>(_positionType);
    }
    KRYS_NODISCARD OverflowAlignment Overflow() const noexcept
    {
      return static_cast<OverflowAlignment>(_overflow);
    }

    KRYS_NODISCARD bool IsNormal(ItemPosition autoAlignment = ItemPosition::Normal) const noexcept
    {
      if (Position() == ItemPosition::Auto)
      {
        return autoAlignment == ItemPosition::Normal;
      }
      return Position() == ItemPosition::Normal;
    }

    // Must resolve Auto before calling. Normal treated as Start.
    // Returns position adjustment from container's start edge.
    KRYS_NODISCARD static LayoutUnit AdjustmentFromStartEdge(LayoutUnit extraSpace,
                                                             ItemPosition alignmentPosition,
                                                             LogicalBoxAxis containerAxis,
                                                             WritingMode containerWritingMode,
                                                             WritingMode selfWritingMode) noexcept;

    KRYS_NODISCARD bool operator==(const StyleSelfAlignmentData &) const noexcept = default;

  private:
    uint8 _position : 4 {0};
    uint8 _positionType : 1 {0}; // Whether or not alignment uses the 'legacy' keyword.
    uint8 _overflow : 2 {0};
  };
}
