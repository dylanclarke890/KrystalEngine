#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Core/Base.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo
{
  enum class LogicalBoxAxis : uint8
  {
    Inline,
    Block
  };

  enum class BoxAxis : uint8
  {
    Horizontal,
    Vertical,
  };

  enum class LogicalBoxSide : uint8
  {
    // Flow-Relative sides.
    BlockStart,
    InlineEnd,
    BlockEnd,
    InlineStart,

    // Coordinate-relative sides (per WebKit-internal logic).
    LogicalLeft,  // low-coord inline side
    LogicalRight, // high-coord inline side
    LogicalTop = BlockStart,
    LogicalBottom = BlockEnd,

    // Line-relative sides are currently unused.
    // Add LineOver/LineUnder/LineLeft/LineRight if needed.
  };

  enum class BoxSide : uint8
  {
    Top,
    Right,
    Bottom,
    Left,
  };

  enum class LogicalBoxCorner : uint8
  {
    // Follows BlockInline naming convention.
    StartStart = 0,
    StartEnd = 1,
    EndStart = 2,
    EndEnd = 3,
  };

  enum class BoxCorner : uint8
  {
    TopLeft = 0,
    TopRight = 1,
    BottomLeft = 2,
    BottomRight = 3,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::LogicalBoxAxis, 2uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::BoxAxis, 2uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::LogicalBoxSide, 6uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::BoxSide, 4uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::LogicalBoxCorner, 4uz);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::BoxCorner, 4uz);

namespace krys::boo
{
  constexpr Array<BoxSide, 4uz> AllBoxSides = {BoxSide::Top, BoxSide::Right, BoxSide::Bottom, BoxSide::Left};

  KRYS_NODISCARD constexpr BoxAxis MapAxisLogicalToPhysical(const WritingMode writingMode,
                                                            const LogicalBoxAxis logicalAxis) noexcept
  {
    bool isBlock = logicalAxis == LogicalBoxAxis::Block;
    bool isVertical = isBlock != writingMode.IsVertical();
    return isVertical ? BoxAxis::Vertical : BoxAxis::Horizontal;
  }

  KRYS_NODISCARD constexpr LogicalBoxAxis MapAxisPhysicalToLogical(const WritingMode writingMode,
                                                                   const BoxAxis axis) noexcept
  {
    bool isVertical = axis == BoxAxis::Vertical;
    bool isBlock = isVertical != writingMode.IsVertical();
    return isBlock ? LogicalBoxAxis::Block : LogicalBoxAxis::Inline;
  }

  KRYS_NODISCARD constexpr LogicalBoxAxis OppositeAxis(LogicalBoxAxis axis) noexcept
  {
    return axis == LogicalBoxAxis::Inline ? LogicalBoxAxis::Block : LogicalBoxAxis::Inline;
  }

  KRYS_NODISCARD constexpr BoxAxis OppositeAxis(BoxAxis axis) noexcept
  {
    return axis == BoxAxis::Horizontal ? BoxAxis::Vertical : BoxAxis::Horizontal;
  }

  KRYS_NODISCARD constexpr BoxSide MapSideLogicalToPhysical(const WritingMode writingMode,
                                                            const LogicalBoxSide logicalSide) noexcept
  {
    switch (logicalSide)
    {
      case LogicalBoxSide::BlockStart:
      {
        switch (writingMode.BlockDirection())
        {
          case FlowDirection::TopToBottom: return BoxSide::Top;
          case FlowDirection::RightToLeft: return BoxSide::Right;
          case FlowDirection::BottomToTop: return BoxSide::Bottom;
          case FlowDirection::LeftToRight: return BoxSide::Left;
        }

        krys_unreachable();
      }
      case LogicalBoxSide::BlockEnd:
      {
        switch (writingMode.BlockDirection())
        {
          case FlowDirection::TopToBottom: return BoxSide::Bottom;
          case FlowDirection::RightToLeft: return BoxSide::Left;
          case FlowDirection::BottomToTop: return BoxSide::Top;
          case FlowDirection::LeftToRight: return BoxSide::Right;
        }

        krys_unreachable();
      }
      case LogicalBoxSide::InlineStart:
      {
        if (writingMode.IsHorizontal())
        {
          return writingMode.IsInlineLeftToRight() ? BoxSide::Left : BoxSide::Right;
        }

        return writingMode.IsInlineTopToBottom() ? BoxSide::Top : BoxSide::Bottom;
      }
      case LogicalBoxSide::InlineEnd:
      {
        if (writingMode.IsHorizontal())
        {
          return writingMode.IsInlineLeftToRight() ? BoxSide::Right : BoxSide::Left;
        }

        return writingMode.IsInlineTopToBottom() ? BoxSide::Bottom : BoxSide::Top;
      }
      case LogicalBoxSide::LogicalLeft:
      {
        if (writingMode.IsHorizontal())
        {
          return BoxSide::Left;
        }

        return BoxSide::Top;
      }
      case LogicalBoxSide::LogicalRight:
      {
        if (writingMode.IsHorizontal())
        {
          return BoxSide::Right;
        }

        return BoxSide::Bottom;
      }
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr LogicalBoxSide MapSidePhysicalToLogical(const WritingMode writingMode,
                                                                   const BoxSide side) noexcept
  {
    switch (side)
    {
      case BoxSide::Top:
      {
        switch (writingMode.BlockDirection())
        {
          case FlowDirection::TopToBottom: return LogicalBoxSide::BlockStart;
          case FlowDirection::RightToLeft:
          case FlowDirection::LeftToRight:
            return writingMode.IsInlineTopToBottom() ? LogicalBoxSide::InlineStart
                                                     : LogicalBoxSide::InlineEnd;
          case FlowDirection::BottomToTop: return LogicalBoxSide::BlockEnd;
        }

        krys_unreachable();
      }
      case BoxSide::Left:
      {
        switch (writingMode.BlockDirection())
        {
          case FlowDirection::TopToBottom:
          case FlowDirection::BottomToTop:
            return writingMode.IsInlineLeftToRight() ? LogicalBoxSide::InlineStart
                                                     : LogicalBoxSide::InlineEnd;
          case FlowDirection::LeftToRight: return LogicalBoxSide::BlockStart;
          case FlowDirection::RightToLeft: return LogicalBoxSide::BlockEnd;
        }

        krys_unreachable();
      }
      case BoxSide::Bottom:
      {
        switch (writingMode.BlockDirection())
        {
          case FlowDirection::TopToBottom: return LogicalBoxSide::BlockEnd;
          case FlowDirection::RightToLeft:
          case FlowDirection::LeftToRight:
            return writingMode.IsInlineTopToBottom() ? LogicalBoxSide::InlineEnd
                                                     : LogicalBoxSide::InlineStart;
          case FlowDirection::BottomToTop: return LogicalBoxSide::BlockStart;
        }

        krys_unreachable();
      }
      case BoxSide::Right:
      {
        switch (writingMode.BlockDirection())
        {
          case FlowDirection::TopToBottom:
          case FlowDirection::BottomToTop:
            return writingMode.IsInlineLeftToRight() ? LogicalBoxSide::InlineEnd
                                                     : LogicalBoxSide::InlineStart;
          case FlowDirection::LeftToRight: return LogicalBoxSide::BlockEnd;
          case FlowDirection::RightToLeft: return LogicalBoxSide::BlockStart;
        }

        krys_unreachable();
      }
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr BoxAxis BoxAxisForSide(BoxSide side) noexcept
  {
    switch (side)
    {
      case BoxSide::Top:
      case BoxSide::Bottom: return BoxAxis::Vertical;
      case BoxSide::Left:
      case BoxSide::Right:  return BoxAxis::Horizontal;
    }

    krys_unreachable();
  }

  // Numeric values used below for bit-hacking; don't change without adjusting mapping methods.

  KRYS_NODISCARD constexpr BoxCorner
    MapCornerLogicalToPhysical(const WritingMode writingMode,
                               const LogicalBoxCorner logicalBoxCorner) noexcept
  {
    bool isBlockStart = !(static_cast<uint8_t>(logicalBoxCorner) & 2);
    bool isInlineStart = !(static_cast<uint8_t>(logicalBoxCorner) & 1);

    bool isTop, isLeft;
    if (writingMode.IsHorizontal())
    {
      isTop = isBlockStart != writingMode.IsBlockFlipped();
      isLeft = isInlineStart == writingMode.IsInlineLeftToRight();
    }
    else
    {
      isTop = isInlineStart == writingMode.IsInlineTopToBottom();
      isLeft = isBlockStart != writingMode.IsBlockFlipped();
    }

    if (isTop)
      return isLeft ? BoxCorner::TopLeft : BoxCorner::TopRight;
    return isLeft ? BoxCorner::BottomLeft : BoxCorner::BottomRight;
  }
  KRYS_NODISCARD constexpr LogicalBoxCorner MapCornerPhysicalToLogical(const WritingMode writingMode,
                                                                       const BoxCorner boxCorner) noexcept
  {
    bool isTop = !(static_cast<uint8_t>(boxCorner) & 2);
    bool isLeft = !(static_cast<uint8_t>(boxCorner) & 1);

    bool isBlockStart, isInlineStart;
    if (writingMode.IsHorizontal())
    {
      isBlockStart = isTop != writingMode.IsBlockFlipped();
      isInlineStart = isLeft == writingMode.IsInlineLeftToRight();
    }
    else
    {
      isBlockStart = isLeft != writingMode.IsBlockFlipped();
      isInlineStart = isTop == writingMode.IsInlineTopToBottom();
    }

    if (isBlockStart)
      return isInlineStart ? LogicalBoxCorner::StartStart : LogicalBoxCorner::StartEnd;
    return isInlineStart ? LogicalBoxCorner::EndStart : LogicalBoxCorner::EndEnd;
  }

#pragma region WritingMode Mapping Functions

  constexpr BoxAxis WritingMode::InlineAxis() const noexcept
  {
    return IsHorizontal() ? BoxAxis::Horizontal : BoxAxis::Vertical;
  }

  constexpr BoxAxis WritingMode::BlockAxis() const noexcept
  {
    return IsHorizontal() ? BoxAxis::Vertical : BoxAxis::Horizontal;
  }

  constexpr LogicalBoxAxis WritingMode::HorizontalAxis() const noexcept
  {
    return IsHorizontal() ? LogicalBoxAxis::Inline : LogicalBoxAxis::Block;
  }

  constexpr LogicalBoxAxis WritingMode::VerticalAxis() const noexcept
  {
    return IsHorizontal() ? LogicalBoxAxis::Block : LogicalBoxAxis::Inline;
  }

#pragma endregion
}
