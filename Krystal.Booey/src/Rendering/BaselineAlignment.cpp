#include "Krystal.Booey/Rendering/BaselineAlignment.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  BaselineGroup::BaselineGroup(FlowDirection blockFlow, ItemPosition alignmentSubjectPreference) noexcept
      : _blockFlow(blockFlow), _preference(alignmentSubjectPreference), _maxAscent(0)
  {
  }

  void BaselineGroup::Update(const RenderBox &alignmentSubject, LayoutUnit ascent) noexcept
  {
    if (_alignmentSubjects.insert(&alignmentSubject).second)
    {
      _maxAscent = std::max(_maxAscent, ascent);
    }
  }

  bool BaselineGroup::IsOppositeBlockFlow(FlowDirection blockFlow) const noexcept
  {
    switch (blockFlow)
    {
      case FlowDirection::TopToBottom: return false;
      case FlowDirection::LeftToRight: return _blockFlow == FlowDirection::RightToLeft;
      case FlowDirection::RightToLeft: return _blockFlow == FlowDirection::LeftToRight;
      default:                         krys_debug_assert(false); return false;
    }
  }

  bool BaselineGroup::IsOrthogonalBlockFlow(FlowDirection blockFlow) const noexcept
  {
    switch (blockFlow)
    {
      case FlowDirection::TopToBottom: return _blockFlow != FlowDirection::TopToBottom;
      case FlowDirection::LeftToRight:
      case FlowDirection::RightToLeft: return _blockFlow == FlowDirection::TopToBottom;
      default:                         krys_debug_assert(false); return false;
    }
  }

  bool BaselineGroup::IsCompatible(FlowDirection alignmentSubjectBlockFlow,
                                   ItemPosition alignmentSubjectPreference) const noexcept
  {
    krys_debug_assert(IsBaselinePosition(alignmentSubjectPreference));
    krys_debug_assert(ComputeSize() > 0);
    return ((_blockFlow == alignmentSubjectBlockFlow || IsOrthogonalBlockFlow(alignmentSubjectBlockFlow))
            && _preference == alignmentSubjectPreference)
           || (IsOppositeBlockFlow(alignmentSubjectBlockFlow) && _preference != alignmentSubjectPreference);
  }

  BaselineAlignmentState::BaselineAlignmentState(const RenderBox &alignmentSubject, ItemPosition preference,
                                                 LayoutUnit ascent, LogicalBoxAxis alignmentContextAxis,
                                                 WritingMode alignmentContainerWritingMode) noexcept
      : _alignmentContainerWritingMode(alignmentContainerWritingMode),
        _alignmentContextAxis(alignmentContextAxis)
  {
    krys_debug_assert(IsBaselinePosition(preference));
    UpdateSharedGroup(alignmentSubject, preference, ascent);
  }

  const BaselineGroup &BaselineAlignmentState::SharedGroup(const RenderBox &alignmentSubject,
                                                           ItemPosition preference) const noexcept
  {
    krys_debug_assert(IsBaselinePosition(preference));
    return const_cast<BaselineAlignmentState *>(this)->FindCompatibleSharedGroup(alignmentSubject,
                                                                                 preference);
  }

  void BaselineAlignmentState::UpdateSharedGroup(const RenderBox &alignmentSubject, ItemPosition preference,
                                                 LayoutUnit ascent) noexcept
  {
    krys_debug_assert(IsBaselinePosition(preference));
    auto &group = FindCompatibleSharedGroup(alignmentSubject, preference);
    group.Update(alignmentSubject, ascent);
  }

  FontBaseline BaselineAlignmentState::DominantBaseline(WritingMode writingMode) noexcept
  {
    // https://drafts.csswg.org/css-inline-3/#alignment-baseline-property
    // https://drafts.csswg.org/css-inline-3/#dominant-baseline-property
    return writingMode.PrefersCentralBaseline() ? FontBaseline::Central : FontBaseline::Alphabetic;
  }

  LayoutUnit BaselineAlignmentState::SynthesizedBaseline(const RenderBox &box, FontBaseline baselineType,
                                                         WritingMode writingModeForSynthesis,
                                                         LineDirection lineDirection,
                                                         BaselineSynthesisEdge edge) noexcept
  {
    auto boxSize = lineDirection == LineDirection::Horizontal ? box.Height() : box.Width();
    if (edge == BaselineSynthesisEdge::ContentBox)
    {
      boxSize -= lineDirection == LineDirection::Horizontal ? box.VerticalBorderAndPaddingExtent()
                                                            : box.HorizontalBorderAndPaddingExtent();
    }
    else if (edge == BaselineSynthesisEdge::MarginBox)
    {
      boxSize += lineDirection == LineDirection::Horizontal ? box.VerticalMarginExtent()
                                                            : box.HorizontalMarginExtent();
    }

    if (baselineType == FontBaseline::Alphabetic)
    {
      // When synthesizing the alphabetic baseline for a box we are determining the distance
      // to the line-under edge. For a box with vertical-lr writing mode the location
      // of the line-under edge should be the same as the box's block-start edge. For
      // vertical-rl writing mode we need the box's size since they are on opposiate sides.
      auto shouldTreatAsHorizontal =
        lineDirection == LineDirection::Horizontal
        || writingModeForSynthesis.ComputedWritingMode() == StyleWritingMode::VerticalRl;
      return shouldTreatAsHorizontal ? boxSize : LayoutUnit();
    }
    return boxSize / 2;
  }

  WritingMode BaselineAlignmentState::UsedWritingModeForBaselineAlignment(
    LogicalBoxAxis alignmentContextAxis, WritingMode alignmentContainerWritingMode,
    WritingMode aligmentSubjectWritingMode) noexcept
  {
    auto isAlignmentSubjectBlockFlowParallelToAlignmentContextAxis = [&]
    {
      if (alignmentContextAxis == LogicalBoxAxis::Block)
      {
        return !alignmentContainerWritingMode.IsOrthogonal(aligmentSubjectWritingMode);
      }
      return alignmentContainerWritingMode.IsOrthogonal(aligmentSubjectWritingMode);
    };

    // css-align-3: 9.1. Determining the Baselines of a Box
    // In general, the writing mode of the box, shape, or other object being aligned is used to determine
    // the line-under and line-over edges for synthesis...
    if (!isAlignmentSubjectBlockFlowParallelToAlignmentContextAxis())
    {
      return aligmentSubjectWritingMode;
    }

    // ... However, when that writing mode's block flow direction
    // is parallel to the axis of the alignment context, an axis-compatible writing mode must be assumed:
    //

    // If the box establishing the alignment context has a block flow direction that is orthogonal to the
    // axis of the alignment context, use its writing mode.
    if (alignmentContextAxis == LogicalBoxAxis::Inline)
    {
      return alignmentContainerWritingMode;
    }

    // Otherwise:
    // If the box's own writing mode is vertical, assume horizontal-tb.
    // If the box's own writing mode is horizontal, assume vertical-lr if
    // direction is ltr and vertical-rl if direction is rtl.
    if (!aligmentSubjectWritingMode.IsHorizontal())
    {
      return {};
    }
    auto styleWritingMode =
      alignmentContainerWritingMode.IsBidiLTR() ? StyleWritingMode::VerticalLr : StyleWritingMode::VerticalRl;
    return {styleWritingMode, TextDirection::LTR, TextOrientation::Mixed};
  }

  BaselineGroup &BaselineAlignmentState::FindCompatibleSharedGroup(const RenderBox &alignmentSubject,
                                                                   ItemPosition preference) noexcept
  {
    auto usedWritingModeForBaselineAlignment = UsedWritingModeForBaselineAlignment(
      _alignmentContextAxis, _alignmentContainerWritingMode, alignmentSubject.GetWritingMode());
    auto blockFlowDirection = usedWritingModeForBaselineAlignment.BlockDirection();
    for (auto &group : _sharedGroups)
    {
      if (group.IsCompatible(blockFlowDirection, preference))
      {
        return group;
      }
    }
    _sharedGroups.insert(_sharedGroups.begin(), BaselineGroup(blockFlowDirection, preference));
    return _sharedGroups[0];
  }
}
