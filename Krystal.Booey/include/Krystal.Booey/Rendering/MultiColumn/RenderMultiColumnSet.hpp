/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentContainerSet.hpp"
#include "Krystal.Booey/Rendering/Layers/LayerFragment.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  struct PaintInfo;

  /// @brief A run of same-sized columns, one renderer standing for them all. A multi-column container has one
  /// set, plus one more after each spanner, since a spanner sits between the columns before and after it.
  class RenderMultiColumnSet final : public RenderFragmentContainerSet
  {
    KRYS_NON_COPY_MOVABLE(RenderMultiColumnSet);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMultiColumnSet);

  public:
    enum class ColumnHitTestTranslationMode : bool
    {
      ClampHitTestTranslationToColumns,
      DoNotClampHitTestTranslationToColumns
    };

  private:
    enum class ColumnIndexCalculationMode : bool
    {
      ClampToExistingColumns, // Stay within the range of already existing columns.
      AssumeNewColumns        // Allow column indices outside the range of already existing columns.
    };

    // A run of content without explicit (forced) breaks; i.e. a flow thread portion between two
    // explicit breaks, between flow thread start and an explicit break, between an explicit break
    // and flow thread end, or, in cases when there are no explicit breaks at all: between flow
    // thread start and flow thread end. We need to know where the explicit breaks are, in order to
    // figure out where the implicit breaks will end up, so that we get the columns properly
    // balanced. A content run starts out as representing one single column, and will represent one
    // additional column for each implicit break "inserted" there.
    class ContentRun
    {
    private:
      LayoutUnit _breakOffset;           // Flow thread offset where this run ends.
      uint32 _assumedImplicitBreaks {0}; // Number of implicit breaks in this run assumed so far.

    public:
      explicit ContentRun(LayoutUnit breakOffset) noexcept : _breakOffset(breakOffset)
      {
      }

      KRYS_NODISCARD uint32 AssumedImplicitBreaks() const noexcept
      {
        return _assumedImplicitBreaks;
      }
      void AssumeAnotherImplicitBreak() noexcept
      {
        ++_assumedImplicitBreaks;
      }
      KRYS_NODISCARD LayoutUnit BreakOffset() const noexcept
      {
        return _breakOffset;
      }

      /// @brief The column height that this content run would require, considering the implicit breaks
      /// assumed so far.
      KRYS_NODISCARD LayoutUnit ColumnLogicalHeight(LayoutUnit startOffset) const noexcept;
    };

    uint32 _computedColumnCount {
      1}; // Used column count (the resulting 'N' from the multicol spec's pseudo-algorithm)
    LayoutUnit
      _computedColumnWidth; // Used column width (the resulting 'W' from the multicol spec's pseudo-algorithm)
    LayoutUnit _computedColumnHeight;
    LayoutUnit _availableColumnHeight;
    bool _columnHeightComputed {false};

    // The following variables are used when balancing the column set.
    LayoutUnit _maxColumnHeight;  // Maximum column height allowed.
    LayoutUnit _minSpaceShortage; // The smallest amount of space shortage that caused a column break.
    LayoutUnit _minimumColumnHeight;
    LayoutUnit _spaceShortageForSizeContainment; // The shortage space that keeps size containment monolithic.

    List<ContentRun> _contentRuns;

  public:
    RenderMultiColumnSet(RenderFragmentedFlow &fragmentedFlow, RenderStyle &&style) noexcept;
    ~RenderMultiColumnSet() noexcept override;

    /// @brief The multi-column container.
    KRYS_NODISCARD RenderBlockFlow *MultiColumnBlockFlow() const noexcept;
    KRYS_NODISCARD RenderMultiColumnFlow *MultiColumnFlow() const noexcept;

    KRYS_NODISCARD RenderMultiColumnSet *NextSiblingMultiColumnSet() const noexcept;
    KRYS_NODISCARD RenderMultiColumnSet *PreviousSiblingMultiColumnSet() const noexcept;

    /// @brief The first object in the flow that's rendered inside this set.
    KRYS_NODISCARD RenderObject *FirstRendererInFragmentedFlow() const noexcept;
    /// @brief The last object in the flow that's rendered inside this set.
    KRYS_NODISCARD RenderObject *LastRendererInFragmentedFlow() const noexcept;

    /// @brief Whether @p renderer (a descendant of the flow) is inside this column set.
    KRYS_NODISCARD bool ContainsRendererInFragmentedFlow(const RenderObject &renderer) const noexcept;

    void SetLogicalTopInFragmentedFlow(LayoutUnit logicalTop) noexcept;
    KRYS_NODISCARD LayoutUnit LogicalTopInFragmentedFlow() const noexcept
    {
      return IsHorizontalWritingMode() ? FragmentedFlowPortionRect().Y() : FragmentedFlowPortionRect().X();
    }
    void SetLogicalBottomInFragmentedFlow(LayoutUnit logicalBottom) noexcept;
    KRYS_NODISCARD LayoutUnit LogicalBottomInFragmentedFlow() const noexcept
    {
      return IsHorizontalWritingMode() ? FragmentedFlowPortionRect().MaxY()
                                       : FragmentedFlowPortionRect().MaxX();
    }
    KRYS_NODISCARD LayoutUnit LogicalHeightInFragmentedFlow() const noexcept
    {
      return IsHorizontalWritingMode() ? FragmentedFlowPortionRect().Height()
                                       : FragmentedFlowPortionRect().Width();
    }

    KRYS_NODISCARD uint32 ComputedColumnCount() const noexcept
    {
      return _computedColumnCount;
    }
    KRYS_NODISCARD LayoutUnit ComputedColumnWidth() const noexcept
    {
      return _computedColumnWidth;
    }
    KRYS_NODISCARD LayoutUnit ComputedColumnHeight() const noexcept
    {
      return _computedColumnHeight;
    }
    KRYS_NODISCARD bool ColumnHeightComputed() const noexcept
    {
      return _columnHeightComputed;
    }

    void SetComputedColumnWidthAndCount(LayoutUnit width, uint32 count) noexcept
    {
      _computedColumnWidth = width;
      _computedColumnCount = count;
    }

    KRYS_NODISCARD LayoutUnit HeightAdjustedForSetOffset(LayoutUnit height) const noexcept;

    void UpdateMinimumColumnHeight(LayoutUnit height) noexcept
    {
      _minimumColumnHeight = std::max(height, _minimumColumnHeight);
    }
    KRYS_NODISCARD LayoutUnit MinimumColumnHeight() const noexcept
    {
      return _minimumColumnHeight;
    }

    void UpdateSpaceShortageForSizeContainment(LayoutUnit shortage) noexcept
    {
      if (_spaceShortageForSizeContainment <= 0)
      {
        _spaceShortageForSizeContainment = shortage;
        return;
      }
      _spaceShortageForSizeContainment = std::min(shortage, _spaceShortageForSizeContainment);
    }

    KRYS_NODISCARD uint32 ForcedBreaksCount() const noexcept
    {
      return static_cast<uint32>(_contentRuns.size());
    }
    void ClearForcedBreaks() noexcept;
    void AddForcedBreak(LayoutUnit offsetFromFirstPage) noexcept;

    /// @brief (Re-)calculates the column height: for balancing, or to fit a constrained multicol height. With
    /// @p initial, a balanced set guesses an initial height; otherwise it stretches the height a tad.
    /// @return Whether the column height changed, so another layout pass is needed.
    bool RecalculateColumnHeight(bool initial) noexcept;

    /// @brief Records space shortage (the amount of space that would have been enough to prevent some element
    /// from being moved to the next column) at a column break. The smallest shortage found is what the
    /// column height is stretched by, if the columns turn out not to be tall enough.
    void RecordSpaceShortage(LayoutUnit spaceShortage) noexcept;

    void UpdateLogicalWidth() noexcept override;

    void PrepareForLayout(bool initial) noexcept;
    /// @brief Begins laying out content for this column set. This happens at the beginning of flow layout,
    /// and when advancing from a previous column set or spanner to this one.
    void BeginFlow(RenderBlock *container) noexcept;
    /// @brief Finishes laying out content for this column set. This happens at the end of flow layout, and
    /// when advancing to the next column set or spanner.
    void EndFlow(RenderBlock *container, LayoutUnit bottomInContainer) noexcept;
    /// @brief Whether this set has been flowed in this layout pass.
    KRYS_NODISCARD bool HasBeenFlowed() const noexcept
    {
      return LogicalBottomInFragmentedFlow() != RenderFragmentedFlow::MaxLogicalHeight();
    }

    KRYS_NODISCARD bool RequiresBalancing() const noexcept;

    KRYS_NODISCARD LayoutPoint ColumnTranslationForOffset(LayoutUnit offset) const noexcept;

    void PaintColumnRules(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept override;

    KRYS_NODISCARD LayoutPoint TranslateFragmentPointToFragmentedFlow(
      const LayoutPoint &logicalPoint,
      ColumnHitTestTranslationMode clampMode =
        ColumnHitTestTranslationMode::DoNotClampHitTestTranslationToColumns) const noexcept;

    KRYS_NODISCARD LayoutRect ColumnRectAt(uint32 index) const noexcept;
    KRYS_NODISCARD uint32 ColumnCount() const noexcept;

    KRYS_NODISCARD LayoutUnit ColumnGap() const noexcept;

  private:
    void AddOverflowFromInFlowChildren(EnumSet<ComputeOverflowOptions> options = {}) noexcept override;

    void Layout() noexcept override;

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

    void PaintObject(PaintInfo &, const LayoutPoint &) noexcept override
    {
    }

    KRYS_NODISCARD LayoutUnit PageLogicalWidth() const noexcept override
    {
      return _computedColumnWidth;
    }
    KRYS_NODISCARD LayoutUnit PageLogicalHeight() const noexcept override
    {
      return _computedColumnHeight;
    }

    KRYS_NODISCARD LayoutUnit PageLogicalTopForOffset(LayoutUnit offset) const noexcept override;

    KRYS_NODISCARD LayoutUnit LogicalHeightOfAllFragmentedFlowContent() const noexcept override
    {
      return LogicalHeightInFragmentedFlow();
    }

    void RepaintFragmentedFlowContent(const LayoutRect &repaintRect) const noexcept override;

    void CollectLayerFragments(LayerFragments &fragments, const LayoutRect &layerBoundingBox,
                               const LayoutRect &dirtyRect) const noexcept override;

    KRYS_NODISCARD List<LayoutRect>
      FragmentRectsForFlowContentRect(const LayoutRect &rect) const noexcept final;
    KRYS_NODISCARD bool ContentRectSpansFragments(const LayoutRect &rect) const noexcept final;

    KRYS_NODISCARD dom::PositionWithAffinity
      PositionForPoint(const LayoutPoint &logicalPoint, HitTestSource source,
                       const RenderFragmentContainer *fragment = nullptr) noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMultiColumnSet";
    }

    KRYS_NODISCARD LayoutUnit CalculateMaxColumnHeight() const noexcept;

    KRYS_NODISCARD LayoutUnit ColumnLogicalLeft(uint32 index) const noexcept;

    KRYS_NODISCARD LayoutRect FragmentedFlowPortionRectAt(uint32 index) const noexcept;
    KRYS_NODISCARD LayoutRect FragmentedFlowPortionOverflowRect(const LayoutRect &fragmentedFlowPortion,
                                                                uint32 index,
                                                                uint32 columnCount) const noexcept;

    KRYS_NODISCARD uint32 ColumnIndexAtOffset(
      LayoutUnit offset,
      ColumnIndexCalculationMode mode = ColumnIndexCalculationMode::ClampToExistingColumns) const noexcept;

    KRYS_NODISCARD Pair<uint32, uint32>
      FirstAndLastColumnsFromOffsets(LayoutUnit topOffset, LayoutUnit bottomOffset) const noexcept;

    void SetAndConstrainColumnHeight(LayoutUnit newHeight) noexcept;

    /// @brief The index of the content run with the currently tallest columns, taking all implicit breaks
    /// assumed so far into account.
    KRYS_NODISCARD uint32 FindRunWithTallestColumns() const noexcept;

    /// @brief Given the current list of content runs, makes assumptions about where implicit breaks need to
    /// go (if there's room for any at all; depending on the number of explicit breaks), and stores the
    /// results. This is needed in order to balance the columns.
    void DistributeImplicitBreaks() noexcept;

    KRYS_NODISCARD LayoutUnit CalculateBalancedHeight(bool initial) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMultiColumnSet)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMultiColumnSet();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
