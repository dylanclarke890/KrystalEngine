#include "Krystal.Booey/Style/Values/AnchorPosition/PositionArea.hpp"

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"

namespace krys::boo::style
{
  [[maybe_unused]] static bool AxisIsBlockOrX(PositionAreaAxis axis)
  {
    switch (axis)
    {
      case PositionAreaAxis::Horizontal:
      case PositionAreaAxis::X:
      case PositionAreaAxis::Block:      return true;

      default:                           return false;
    }
  }

  [[maybe_unused]] static bool AxisIsInlineOrY(PositionAreaAxis axis)
  {
    switch (axis)
    {
      case PositionAreaAxis::Vertical:
      case PositionAreaAxis::Y:
      case PositionAreaAxis::Inline:   return true;

      default:                         return false;
    }
  }

  PositionAreaValue::PositionAreaValue(PositionAreaSpan blockOrXAxis, PositionAreaSpan inlineOrYAxis) noexcept
      : _blockOrXAxis(blockOrXAxis), _inlineOrYAxis(inlineOrYAxis)
  {
    krys_debug_assert(AxisIsBlockOrX(_blockOrXAxis.Axis()));
    krys_debug_assert(AxisIsInlineOrY(_inlineOrYAxis.Axis()));
  }

  PositionAreaSpan PositionAreaValue::SpanForAxis(BoxAxis physicalAxis, WritingMode containerWritingMode,
                                                  WritingMode selfWritingMode) const noexcept
  {
    bool useSelfWritingMode = _blockOrXAxis.Self() == PositionAreaSelf::Yes;
    auto writingMode = useSelfWritingMode ? selfWritingMode : containerWritingMode;
    return physicalAxis == MapPositionAreaAxisToPhysicalAxis(_blockOrXAxis.Axis(), writingMode)
             ? _blockOrXAxis
             : _inlineOrYAxis;
  }

  PositionAreaSpan PositionAreaValue::SpanForAxis(LogicalBoxAxis logicalAxis,
                                                  WritingMode containerWritingMode,
                                                  WritingMode selfWritingMode) const noexcept
  {
    bool useSelfWritingMode = _blockOrXAxis.Self() == PositionAreaSelf::Yes;
    auto writingMode = useSelfWritingMode ? selfWritingMode : containerWritingMode;
    return logicalAxis == MapPositionAreaAxisToLogicalAxis(_blockOrXAxis.Axis(), writingMode)
             ? _blockOrXAxis
             : _inlineOrYAxis;
  }

  PositionAreaTrack PositionAreaValue::CoordMatchedTrackForAxis(BoxAxis physicalAxis,
                                                                WritingMode containerWritingMode,
                                                                WritingMode selfWritingMode) const noexcept
  {
    auto relevantSpan = SpanForAxis(physicalAxis, containerWritingMode, selfWritingMode);
    auto positionAxis = relevantSpan.Axis();
    auto track = relevantSpan.Track();

    bool shouldFlip = false;
    if (LogicalBoxAxis::Inline == MapAxisPhysicalToLogical(containerWritingMode, physicalAxis))
    {
      if (IsPositionAreaDirectionLogical(positionAxis))
      {
        shouldFlip = containerWritingMode.IsInlineFlipped();
        if (relevantSpan.Self() == PositionAreaSelf::Yes
            && !containerWritingMode.IsInlineMatchingAny(selfWritingMode))
        {
          shouldFlip = !shouldFlip;
        }
      }
    }
    else
    {
      shouldFlip = !IsPositionAreaDirectionLogical(positionAxis) && containerWritingMode.IsBlockFlipped();
      if (relevantSpan.Self() == PositionAreaSelf::Yes
          && !containerWritingMode.IsBlockMatchingAny(selfWritingMode))
      {
        shouldFlip = !shouldFlip;
      }
    }

    return shouldFlip ? FlipPositionAreaTrack(track) : track;
  }

  KRYS_NODISCARD static ItemPosition Flip(ItemPosition alignment) noexcept
  {
    return ItemPosition::Start == alignment ? ItemPosition::End : ItemPosition::Start;
  };

  ItemPosition PositionAreaValue::DefaultAlignmentForAxis(BoxAxis physicalAxis,
                                                          WritingMode containerWritingMode,
                                                          WritingMode selfWritingMode) const noexcept
  {
    auto relevantSpan = SpanForAxis(physicalAxis, containerWritingMode, selfWritingMode);

    ItemPosition alignment {ItemPosition::Start};
    switch (relevantSpan.Track())
    {
      case PositionAreaTrack::Start:
      case PositionAreaTrack::SpanStart: alignment = ItemPosition::End; break;
      case PositionAreaTrack::End:
      case PositionAreaTrack::SpanEnd:   alignment = ItemPosition::Start; break;
      case PositionAreaTrack::Center:    return ItemPosition::Center;
      case PositionAreaTrack::SpanAll:   return ItemPosition::AnchorCenter;
    }

    // Remap for self alignment.
    auto axis = relevantSpan.Axis();
    bool shouldFlip = false;
    if (relevantSpan.Self() == PositionAreaSelf::Yes && containerWritingMode != selfWritingMode)
    {
      auto logicalAxis = MapPositionAreaAxisToLogicalAxis(axis, selfWritingMode);
      if (containerWritingMode.IsOrthogonal(selfWritingMode))
      {
        if (LogicalBoxAxis::Inline == logicalAxis)
        {
          shouldFlip = !selfWritingMode.IsInlineMatchingAny(containerWritingMode);
        }
        else
        {
          shouldFlip = !selfWritingMode.IsBlockMatchingAny(containerWritingMode);
        }
      }
      else if (LogicalBoxAxis::Inline == logicalAxis)
      {
        shouldFlip = selfWritingMode.IsInlineOpposing(containerWritingMode);
      }
      else
      {
        shouldFlip = selfWritingMode.IsBlockOpposing(containerWritingMode);
      }
    }

    if (IsPositionAreaDirectionLogical(axis))
    {
      return shouldFlip ? Flip(alignment) : alignment;
    }

    krys_debug_assert(PositionAreaAxis::Horizontal == axis || PositionAreaAxis::Vertical == axis);

    if ((PositionAreaAxis::Horizontal == axis) == containerWritingMode.IsHorizontal())
    {
      return containerWritingMode.IsInlineFlipped() ? Flip(alignment) : alignment;
    }
    return containerWritingMode.IsBlockFlipped() ? Flip(alignment) : alignment;
  }

  // MARK: - Conversion

  KRYS_NODISCARD static Maybe<PositionAreaAxis> PositionAreaKeywordToAxis(css::ValueId keyword) noexcept
  {
    switch (keyword)
    {
      case css::ValueId::Left:
      case css::ValueId::SpanLeft:
      case css::ValueId::Right:
      case css::ValueId::SpanRight:           return PositionAreaAxis::Horizontal;

      case css::ValueId::Top:
      case css::ValueId::SpanTop:
      case css::ValueId::Bottom:
      case css::ValueId::SpanBottom:          return PositionAreaAxis::Vertical;

      case css::ValueId::XStart:
      case css::ValueId::SpanXStart:
      case css::ValueId::SelfXStart:
      case css::ValueId::SpanSelfXStart:
      case css::ValueId::XEnd:
      case css::ValueId::SpanXEnd:
      case css::ValueId::SelfXEnd:
      case css::ValueId::SpanSelfXEnd:        return PositionAreaAxis::X;

      case css::ValueId::YStart:
      case css::ValueId::SpanYStart:
      case css::ValueId::SelfYStart:
      case css::ValueId::SpanSelfYStart:
      case css::ValueId::YEnd:
      case css::ValueId::SpanYEnd:
      case css::ValueId::SelfYEnd:
      case css::ValueId::SpanSelfYEnd:        return PositionAreaAxis::Y;

      case css::ValueId::BlockStart:
      case css::ValueId::SpanBlockStart:
      case css::ValueId::SelfBlockStart:
      case css::ValueId::SpanSelfBlockStart:
      case css::ValueId::BlockEnd:
      case css::ValueId::SpanBlockEnd:
      case css::ValueId::SelfBlockEnd:
      case css::ValueId::SpanSelfBlockEnd:    return PositionAreaAxis::Block;

      case css::ValueId::InlineStart:
      case css::ValueId::SpanInlineStart:
      case css::ValueId::SelfInlineStart:
      case css::ValueId::SpanSelfInlineStart:
      case css::ValueId::InlineEnd:
      case css::ValueId::SpanInlineEnd:
      case css::ValueId::SelfInlineEnd:
      case css::ValueId::SpanSelfInlineEnd:   return PositionAreaAxis::Inline;

      case css::ValueId::Start:
      case css::ValueId::SpanStart:
      case css::ValueId::SelfStart:
      case css::ValueId::SpanSelfStart:
      case css::ValueId::End:
      case css::ValueId::SpanEnd:
      case css::ValueId::SelfEnd:
      case css::ValueId::SpanSelfEnd:
      case css::ValueId::Center:
      case css::ValueId::SpanAll:             return {};

      default:                                krys_debug_assert_msg(false, "Unreachable"); return {};
    }
  }

  KRYS_NODISCARD static PositionAreaTrack PositionAreaKeywordToTrack(css::ValueId keyword) noexcept
  {
    switch (keyword)
    {
      case css::ValueId::Left:
      case css::ValueId::Top:
      case css::ValueId::XStart:
      case css::ValueId::SelfXStart:
      case css::ValueId::YStart:
      case css::ValueId::SelfYStart:
      case css::ValueId::BlockStart:
      case css::ValueId::SelfBlockStart:
      case css::ValueId::InlineStart:
      case css::ValueId::SelfInlineStart:
      case css::ValueId::Start:
      case css::ValueId::SelfStart:           return PositionAreaTrack::Start;

      case css::ValueId::SpanLeft:
      case css::ValueId::SpanTop:
      case css::ValueId::SpanXStart:
      case css::ValueId::SpanSelfXStart:
      case css::ValueId::SpanYStart:
      case css::ValueId::SpanSelfYStart:
      case css::ValueId::SpanBlockStart:
      case css::ValueId::SpanSelfBlockStart:
      case css::ValueId::SpanInlineStart:
      case css::ValueId::SpanSelfInlineStart:
      case css::ValueId::SpanStart:
      case css::ValueId::SpanSelfStart:       return PositionAreaTrack::SpanStart;

      case css::ValueId::Right:
      case css::ValueId::Bottom:
      case css::ValueId::XEnd:
      case css::ValueId::SelfXEnd:
      case css::ValueId::YEnd:
      case css::ValueId::SelfYEnd:
      case css::ValueId::BlockEnd:
      case css::ValueId::SelfBlockEnd:
      case css::ValueId::InlineEnd:
      case css::ValueId::SelfInlineEnd:
      case css::ValueId::End:
      case css::ValueId::SelfEnd:             return PositionAreaTrack::End;

      case css::ValueId::SpanRight:
      case css::ValueId::SpanBottom:
      case css::ValueId::SpanXEnd:
      case css::ValueId::SpanSelfXEnd:
      case css::ValueId::SpanYEnd:
      case css::ValueId::SpanSelfYEnd:
      case css::ValueId::SpanBlockEnd:
      case css::ValueId::SpanSelfBlockEnd:
      case css::ValueId::SpanInlineEnd:
      case css::ValueId::SpanSelfInlineEnd:
      case css::ValueId::SpanEnd:
      case css::ValueId::SpanSelfEnd:         return PositionAreaTrack::SpanEnd;

      case css::ValueId::Center:              return PositionAreaTrack::Center;
      case css::ValueId::SpanAll:             return PositionAreaTrack::SpanAll;

      default:                                krys_debug_assert_msg(false, "Unreachable"); return PositionAreaTrack::Start;
    }
  }

  KRYS_NODISCARD static PositionAreaSelf PositionAreaKeywordToSelf(css::ValueId keyword) noexcept
  {
    switch (keyword)
    {
      case css::ValueId::Left:
      case css::ValueId::SpanLeft:
      case css::ValueId::Right:
      case css::ValueId::SpanRight:
      case css::ValueId::Top:
      case css::ValueId::SpanTop:
      case css::ValueId::Bottom:
      case css::ValueId::SpanBottom:
      case css::ValueId::XStart:
      case css::ValueId::SpanXStart:
      case css::ValueId::XEnd:
      case css::ValueId::SpanXEnd:
      case css::ValueId::YStart:
      case css::ValueId::SpanYStart:
      case css::ValueId::YEnd:
      case css::ValueId::SpanYEnd:
      case css::ValueId::BlockStart:
      case css::ValueId::SpanBlockStart:
      case css::ValueId::BlockEnd:
      case css::ValueId::SpanBlockEnd:
      case css::ValueId::InlineStart:
      case css::ValueId::SpanInlineStart:
      case css::ValueId::InlineEnd:
      case css::ValueId::SpanInlineEnd:
      case css::ValueId::Start:
      case css::ValueId::SpanStart:
      case css::ValueId::End:
      case css::ValueId::SpanEnd:
      case css::ValueId::Center:
      case css::ValueId::SpanAll:             return PositionAreaSelf::No;

      case css::ValueId::SelfXStart:
      case css::ValueId::SpanSelfXStart:
      case css::ValueId::SelfXEnd:
      case css::ValueId::SpanSelfXEnd:
      case css::ValueId::SelfYStart:
      case css::ValueId::SpanSelfYStart:
      case css::ValueId::SelfYEnd:
      case css::ValueId::SpanSelfYEnd:
      case css::ValueId::SelfBlockStart:
      case css::ValueId::SpanSelfBlockStart:
      case css::ValueId::SelfBlockEnd:
      case css::ValueId::SpanSelfBlockEnd:
      case css::ValueId::SelfInlineStart:
      case css::ValueId::SpanSelfInlineStart:
      case css::ValueId::SelfInlineEnd:
      case css::ValueId::SpanSelfInlineEnd:
      case css::ValueId::SelfStart:
      case css::ValueId::SpanSelfStart:
      case css::ValueId::SelfEnd:
      case css::ValueId::SpanSelfEnd:         return PositionAreaSelf::Yes;

      default:                                krys_debug_assert_msg(false, "Unreachable"); return PositionAreaSelf::No;
    }
  }

  // Expand a one keyword position-area to the equivalent keyword pair value.
  KRYS_NODISCARD static std::pair<css::ValueId, css::ValueId>
    PositionAreaExpandKeyword(css::ValueId dim) noexcept
  {
    auto maybeAxis = PositionAreaKeywordToAxis(dim);
    if (maybeAxis)
    {
      // Keyword is axis unambiguous, second keyword is span-all.

      // Y/inline axis keyword goes after in the pair.
      auto axis = *maybeAxis;
      if (axis == PositionAreaAxis::Vertical || axis == PositionAreaAxis::Y
          || axis == PositionAreaAxis::Inline)
      {
        return {css::ValueId::SpanAll, dim};
      }

      return {dim, css::ValueId::SpanAll};
    }

    // Keyword is axis ambiguous, it's repeated.
    return {dim, dim};
  }

  // Flip a PositionAreaValue across a logical axis (block or inline), given the current writing mode.
  KRYS_NODISCARD static PositionAreaValue FlipPositionAreaByLogicalAxis(LogicalBoxAxis flipAxis,
                                                                        PositionAreaValue area,
                                                                        WritingMode writingMode) noexcept
  {
    auto blockOrXSpan = area.BlockOrXAxis();
    auto inlineOrYSpan = area.InlineOrYAxis();

    // blockOrXSpan is on the flip axis, so flip its track and keep inlineOrYSpan intact.
    if (MapPositionAreaAxisToLogicalAxis(blockOrXSpan.Axis(), writingMode) == flipAxis)
    {
      return {{blockOrXSpan.Axis(), FlipPositionAreaTrack(blockOrXSpan.Track()), blockOrXSpan.Self()},
              inlineOrYSpan};
    }

    // The two spans are orthogonal in axis, so if blockOrXSpan isn't on the flip axis,
    // inlineOrYSpan must be. In this case, flip the track of inlineOrYSpan, and
    // keep blockOrXSpan intact.
    return {blockOrXSpan,
            {inlineOrYSpan.Axis(), FlipPositionAreaTrack(inlineOrYSpan.Track()), inlineOrYSpan.Self()}};
  }

  // Flip a PositionAreaValue across a physical axis (x or y), given the current writing mode.
  KRYS_NODISCARD static PositionAreaValue
    FlipPositionAreaByPhysicalAxis(BoxAxis flipAxis, PositionAreaValue area, WritingMode writingMode) noexcept
  {
    auto blockOrXSpan = area.BlockOrXAxis();
    auto inlineOrYSpan = area.InlineOrYAxis();

    // blockOrXSpan is on the flip axis, so flip its track and keep inlineOrYSpan intact.
    if (MapPositionAreaAxisToPhysicalAxis(blockOrXSpan.Axis(), writingMode) == flipAxis)
    {
      return {{blockOrXSpan.Axis(), FlipPositionAreaTrack(blockOrXSpan.Track()), blockOrXSpan.Self()},
              inlineOrYSpan};
    }

    // The two spans are orthogonal in axis, so if blockOrXSpan isn't on the flip axis,
    // inlineOrYSpan must be. In this case, flip the track of inlineOrYSpan, and
    // keep blockOrXSpan intact.
    return {blockOrXSpan,
            {inlineOrYSpan.Axis(), FlipPositionAreaTrack(inlineOrYSpan.Track()), inlineOrYSpan.Self()}};
  }

  // Flip a PositionAreaValue as specified by flip-start tactic.
  // Intuitively, this mirrors the PositionAreaValue across a diagonal line drawn from the
  // block-start/inline-start corner to the block-end/inline-end corner. This is done
  // by flipping the axes of the spans in the PositionAreaValue, while keeping their track
  // and self properties intact. Because this turns a block/X span into an inline/Y
  // span and vice versa, this function also swaps the order of the spans, so
  // that the block/X span goes before the inline/Y span.
  KRYS_NODISCARD static PositionAreaValue MirrorPositionAreaAcrossDiagonal(PositionAreaValue area) noexcept
  {
    auto blockOrXSpan = area.BlockOrXAxis();
    auto inlineOrYSpan = area.InlineOrYAxis();

    return {{OppositePositionAreaAxis(inlineOrYSpan.Axis()), inlineOrYSpan.Track(), inlineOrYSpan.Self()},
            {OppositePositionAreaAxis(blockOrXSpan.Axis()), blockOrXSpan.Track(), blockOrXSpan.Self()}};
  }

  PositionArea CSSValueConversion<PositionArea>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    std::pair<css::ValueId, css::ValueId> dimPair;

    if (value.IsValueId())
    {
      if (value.ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      dimPair = PositionAreaExpandKeyword(value.ValueId());
    }
    else if (const auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      const auto &first = pair->First();
      const auto &second = pair->Second();

      if (!first.IsValueId() || !second.IsValueId())
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::None {};
      }

      // The parsing logic guarantees the keyword pair is in the correct order
      // (horizontal/x/block axis before vertical/Y/inline axis)

      dimPair = Pair<css::ValueId, css::ValueId> {first.ValueId(), second.ValueId()};
    }
    else
    {
      // value MUST be a single ValueID or a pair of ValueIDs, as returned by the parsing logic.
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    auto dim1Axis = PositionAreaKeywordToAxis(dimPair.first);
    auto dim2Axis = PositionAreaKeywordToAxis(dimPair.second);

    // If both keyword axes are ambiguous, the first one is block axis and second one
    // is inline axis. If only one keyword axis is ambiguous, its axis is the opposite
    // of the other keyword's axis.
    if (!dim1Axis && !dim2Axis)
    {
      dim1Axis = PositionAreaAxis::Block;
      dim2Axis = PositionAreaAxis::Inline;
    }
    else if (!dim1Axis)
    {
      dim1Axis = OppositePositionAreaAxis(*dim2Axis);
    }
    else if (!dim2Axis)
    {
      dim2Axis = OppositePositionAreaAxis(*dim1Axis);
    }

    PositionAreaValue area {
      {*dim1Axis, PositionAreaKeywordToTrack(dimPair.first), PositionAreaKeywordToSelf(dimPair.first)},
      {*dim2Axis, PositionAreaKeywordToTrack(dimPair.second), PositionAreaKeywordToSelf(dimPair.second)}};

    // Flip according to `position-try-fallbacks`, if specified.
    if (const auto &positionTryFallback = state.GetPositionTryFallback())
    {
      auto writingMode = state.Style().GetWritingMode();
      for (auto tactic : positionTryFallback->Tactics)
      {
        switch (tactic)
        {
          case PositionTryFallbackTactic::FlipBlock:
            area = FlipPositionAreaByLogicalAxis(LogicalBoxAxis::Block, area, writingMode);
            break;
          case PositionTryFallbackTactic::FlipInline:
            area = FlipPositionAreaByLogicalAxis(LogicalBoxAxis::Inline, area, writingMode);
            break;
          case PositionTryFallbackTactic::FlipX:
            area = FlipPositionAreaByPhysicalAxis(BoxAxis::Horizontal, area, writingMode);
            break;
          case PositionTryFallbackTactic::FlipY:
            area = FlipPositionAreaByPhysicalAxis(BoxAxis::Vertical, area, writingMode);
            break;
          case PositionTryFallbackTactic::FlipStart: area = MirrorPositionAreaAcrossDiagonal(area); break;
        }
      }
    }

    return area;
  }

  KRYS_NODISCARD static css::ValueId KeywordForPositionAreaSpan(PositionAreaSpan span) noexcept
  {
    auto axis = span.Axis();
    auto track = span.Track();
    auto self = span.Self();

    switch (axis)
    {
      case PositionAreaAxis::Horizontal:
        krys_debug_assert(self == PositionAreaSelf::No);
        switch (track)
        {
          case PositionAreaTrack::Start:     return css::ValueId::Left;
          case PositionAreaTrack::SpanStart: return css::ValueId::SpanLeft;
          case PositionAreaTrack::End:       return css::ValueId::Right;
          case PositionAreaTrack::SpanEnd:   return css::ValueId::SpanRight;
          case PositionAreaTrack::Center:    return css::ValueId::Center;
          case PositionAreaTrack::SpanAll:   return css::ValueId::SpanAll;
          default:                           krys_debug_assert_msg(false, "Unreachable"); return css::ValueId::Left;
        }

      case PositionAreaAxis::Vertical:
        krys_debug_assert(self == PositionAreaSelf::No);
        switch (track)
        {
          case PositionAreaTrack::Start:     return css::ValueId::Top;
          case PositionAreaTrack::SpanStart: return css::ValueId::SpanTop;
          case PositionAreaTrack::End:       return css::ValueId::Bottom;
          case PositionAreaTrack::SpanEnd:   return css::ValueId::SpanBottom;
          case PositionAreaTrack::Center:    return css::ValueId::Center;
          case PositionAreaTrack::SpanAll:   return css::ValueId::SpanAll;
          default:                           krys_debug_assert_msg(false, "Unreachable"); return css::ValueId::Top;
        }

      case PositionAreaAxis::X:
        switch (track)
        {
          case PositionAreaTrack::Start:
            return self == PositionAreaSelf::No ? css::ValueId::XStart : css::ValueId::SelfXStart;
          case PositionAreaTrack::SpanStart:
            return self == PositionAreaSelf::No ? css::ValueId::SpanXStart : css::ValueId::SpanSelfXStart;
          case PositionAreaTrack::End:
            return self == PositionAreaSelf::No ? css::ValueId::XEnd : css::ValueId::SelfXEnd;
          case PositionAreaTrack::SpanEnd:
            return self == PositionAreaSelf::No ? css::ValueId::SpanXEnd : css::ValueId::SpanSelfXEnd;
          case PositionAreaTrack::Center:  return css::ValueId::Center;
          case PositionAreaTrack::SpanAll: return css::ValueId::SpanAll;
          default:                         krys_debug_assert_msg(false, "Unreachable"); return css::ValueId::XStart;
        }

      case PositionAreaAxis::Y:
        switch (track)
        {
          case PositionAreaTrack::Start:
            return self == PositionAreaSelf::No ? css::ValueId::YStart : css::ValueId::SelfYStart;
          case PositionAreaTrack::SpanStart:
            return self == PositionAreaSelf::No ? css::ValueId::SpanYStart : css::ValueId::SpanSelfYStart;
          case PositionAreaTrack::End:
            return self == PositionAreaSelf::No ? css::ValueId::YEnd : css::ValueId::SelfYEnd;
          case PositionAreaTrack::SpanEnd:
            return self == PositionAreaSelf::No ? css::ValueId::SpanYEnd : css::ValueId::SpanSelfYEnd;
          case PositionAreaTrack::Center:  return css::ValueId::Center;
          case PositionAreaTrack::SpanAll: return css::ValueId::SpanAll;
          default:                         krys_debug_assert_msg(false, "Unreachable"); return css::ValueId::YStart;
        }

      case PositionAreaAxis::Block:
        switch (track)
        {
          case PositionAreaTrack::Start:
            return self == PositionAreaSelf::No ? css::ValueId::BlockStart : css::ValueId::SelfBlockStart;
          case PositionAreaTrack::SpanStart:
            return self == PositionAreaSelf::No ? css::ValueId::SpanBlockStart
                                                : css::ValueId::SpanSelfBlockStart;
          case PositionAreaTrack::End:
            return self == PositionAreaSelf::No ? css::ValueId::BlockEnd : css::ValueId::SelfBlockEnd;
          case PositionAreaTrack::SpanEnd:
            return self == PositionAreaSelf::No ? css::ValueId::SpanBlockEnd : css::ValueId::SpanSelfBlockEnd;
          case PositionAreaTrack::Center:  return css::ValueId::Center;
          case PositionAreaTrack::SpanAll: return css::ValueId::SpanAll;
          default:                         krys_debug_assert_msg(false, "Unreachable"); return css::ValueId::BlockStart;
        }

      case PositionAreaAxis::Inline:
        switch (track)
        {
          case PositionAreaTrack::Start:
            return self == PositionAreaSelf::No ? css::ValueId::InlineStart : css::ValueId::SelfInlineStart;
          case PositionAreaTrack::SpanStart:
            return self == PositionAreaSelf::No ? css::ValueId::SpanInlineStart
                                                : css::ValueId::SpanSelfInlineStart;
          case PositionAreaTrack::End:
            return self == PositionAreaSelf::No ? css::ValueId::InlineEnd : css::ValueId::SelfInlineEnd;
          case PositionAreaTrack::SpanEnd:
            return self == PositionAreaSelf::No ? css::ValueId::SpanInlineEnd
                                                : css::ValueId::SpanSelfInlineEnd;
          case PositionAreaTrack::Center:  return css::ValueId::Center;
          case PositionAreaTrack::SpanAll: return css::ValueId::SpanAll;
          default:                         krys_debug_assert_msg(false, "Unreachable"); return css::ValueId::InlineStart;
        }
    }

    krys_debug_assert_msg(false, "Unreachable");
    return css::ValueId::Left;
  }

  Ref<css::Value> CSSValueCreation<PositionAreaValue>::operator()(css::ValuePool &, const RenderStyle &,
                                                                  const PositionAreaValue &value) noexcept
  {
    auto blockOrXAxisKeyword = KeywordForPositionAreaSpan(value.BlockOrXAxis());
    auto inlineOrYAxisKeyword = KeywordForPositionAreaSpan(value.InlineOrYAxis());

    return Ref<css::Value> {css::PropertyParserHelpers::ValueForPositionArea(
      blockOrXAxisKeyword, inlineOrYAxisKeyword, css::PropertyParserHelpers::ValueType::Computed)};
  }

  // MARK: - Serialization

  void Serialiser<PositionAreaValue>::operator()(css::CSSOMString &builder,
                                                 const css::SerialisationContext &context,
                                                 const RenderStyle &, const PositionAreaValue &value) noexcept
  {
    auto blockOrXAxisKeyword = KeywordForPositionAreaSpan(value.BlockOrXAxis());
    auto inlineOrYAxisKeyword = KeywordForPositionAreaSpan(value.InlineOrYAxis());

    // FIXME: Do this more efficiently without creating and destroying a css::Value object.
    builder.append(
      css::PropertyParserHelpers::ValueForPositionArea(blockOrXAxisKeyword, inlineOrYAxisKeyword,
                                                       css::PropertyParserHelpers::ValueType::Computed)
        ->CssText(context));
  }
}
