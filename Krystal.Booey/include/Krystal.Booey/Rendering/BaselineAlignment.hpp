#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  enum class LineDirection : bool;
  class RenderBox;

  KRYS_NODISCARD inline bool IsBaselinePosition(ItemPosition position) noexcept
  {
    return position == ItemPosition::Baseline || position == ItemPosition::LastBaseline;
  }

  KRYS_NODISCARD inline bool IsFirstBaselinePosition(ItemPosition position) noexcept
  {
    return position == ItemPosition::Baseline;
  }

  // These classes are used to implement the Baseline Alignment logic, as described in the CSS Box Alignment
  // specification.
  // https://drafts.csswg.org/css-align/#baseline-terms
  //
  // A baseline-sharing group is composed of boxes that participate in baseline alignment together. This is
  // possible only if they:
  //
  //   * Share an alignment context along an axis perpendicular to their baseline alignment axis.
  //   * Have compatible baseline alignment preferences (i.e., the baselines that want to align are on the
  //   same
  //     side of the alignment context).
  //
  // Once the BaselineGroup is instantiated, defined by a 'block flow direction' and a 'baseline-preference'
  // (first/last baseline), it's ready to collect the alignment subjects that will participate in the Baseline
  // Alignment logic.
  //
  /// @brief A baseline-sharing group: the alignment subjects that align their baselines together, and their
  /// largest ascent.
  class BaselineGroup
  {
    friend class BaselineAlignmentState;

  private:
    FlowDirection _blockFlow;
    ItemPosition _preference;
    LayoutUnit _maxAscent;
    // Keyed by address: the subjects outlive the group (it's built during their container's layout).
    HashSet<const RenderBox *> _alignmentSubjects;

  public:
    // It stores an alignment subject(if not already present) and update the max_ascent
    // associated to this baseline-sharing group.
    void Update(const RenderBox &alignmentSubject, LayoutUnit ascent) noexcept;
    KRYS_NODISCARD LayoutUnit MaxAscent() const noexcept
    {
      return _maxAscent;
    }
    KRYS_NODISCARD size_t ComputeSize() const noexcept
    {
      return _alignmentSubjects.size();
    }
    KRYS_NODISCARD auto begin() const noexcept
    {
      return _alignmentSubjects.begin();
    }
    KRYS_NODISCARD auto end() const noexcept
    {
      return _alignmentSubjects.end();
    }

  private:
    BaselineGroup(FlowDirection blockFlow, ItemPosition alignmentSubjectPreference) noexcept;

    // Determines whether a baseline-sharing group is compatible with an alignment subject,
    // based on its 'block-flow' and 'baseline-preference'
    KRYS_NODISCARD bool IsCompatible(FlowDirection alignmentSubjectBlockFlow,
                                     ItemPosition alignmentSubjectPreference) const noexcept;

    // Determines whether the baseline-sharing group's associated block-flow is opposite (LR vs RL) to
    // particular alignment subject's writing-mode.
    KRYS_NODISCARD bool IsOppositeBlockFlow(FlowDirection blockFlow) const noexcept;

    // Determines whether the baseline-sharing group's associated block-flow is orthogonal (vertical vs
    // horizontal) to particular alignment subject's writing-mode.
    KRYS_NODISCARD bool IsOrthogonalBlockFlow(FlowDirection blockFlow) const noexcept;
  };

  //
  // BaselineAlignmentState provides an API to interact with baseline sharing groups in various
  // ways such as adding alignment subjects to appropriate ones and querying the baseline
  // sharing group for an alignment subject. A BaselineAlignmentState should be created by a formatting
  // context to use for each of its baseline alignment contexts.
  //
  // https://drafts.csswg.org/css-align-3/#baseline-sharing-group
  // A Baseline alignment-context may handle several baseline-sharing groups. In order to create an instance,
  // we need to pass the required data to define the first baseline-sharing group; a BaselineAlignmentState
  // must have at least one baseline-sharing group.
  //
  // By adding new alignment subjects to a BaselineAlignmentState, the baseline-sharing
  // groups it handles are automatically updated, if there is one that is compatible with
  // such alignment subject. Otherwise, a new baseline-sharing group is created, compatible with the new
  // alignment subject.
  /// @brief A baseline alignment context: its baseline-sharing groups.
  class BaselineAlignmentState
  {
  private:
    List<BaselineGroup> _sharedGroups;
    WritingMode _alignmentContainerWritingMode;
    LogicalBoxAxis _alignmentContextAxis;

  public:
    BaselineAlignmentState(const RenderBox &alignmentSubject, ItemPosition preference, LayoutUnit ascent,
                           LogicalBoxAxis alignmentContextAxis,
                           WritingMode alignmentContainerWritingMode) noexcept;
    KRYS_NODISCARD const BaselineGroup &SharedGroup(const RenderBox &alignmentSubject,
                                                    ItemPosition preference) const noexcept;

    void UpdateSharedGroup(const RenderBox &alignmentSubject, ItemPosition preference,
                           LayoutUnit ascent) noexcept;
    KRYS_NODISCARD List<BaselineGroup> &SharedGroups() noexcept
    {
      return _sharedGroups;
    }

    KRYS_NODISCARD static FontBaseline DominantBaseline(WritingMode writingMode) noexcept;
    KRYS_NODISCARD static WritingMode
      UsedWritingModeForBaselineAlignment(LogicalBoxAxis alignmentContextAxis,
                                          WritingMode alignmentContainerWritingMode,
                                          WritingMode alignmentSubjectWritingMode) noexcept;
    KRYS_NODISCARD static LayoutUnit SynthesizedBaseline(const RenderBox &box, FontBaseline baselineType,
                                                         WritingMode writingModeForSynthesis,
                                                         LineDirection lineDirection,
                                                         BaselineSynthesisEdge edge) noexcept;

  private:
    KRYS_NODISCARD BaselineGroup &FindCompatibleSharedGroup(const RenderBox &alignmentSubject,
                                                            ItemPosition preference) noexcept;
  };
}
