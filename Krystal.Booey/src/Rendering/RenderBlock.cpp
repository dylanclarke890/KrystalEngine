#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderBlock::RenderBlock(Type type, dom::Element &element, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBox(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsRenderBlock,
                  typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBlock());
  }

  RenderBlock::RenderBlock(Type type, dom::Document &document, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBox(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsRenderBlock,
                  typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBlock());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBlock::~RenderBlock() noexcept = default;

  utf8_stringview RenderBlock::RenderName() const noexcept
  {
    if (IsBody())
    {
      return u8"RenderBody"; // FIXME: Temporary hack until we know that the regression tests pass.
    }
    if (IsFieldset())
    {
      return u8"RenderFieldSet"; // FIXME: Remove eventually, but done to keep tests from breaking.
    }
    if (IsFloating())
    {
      return u8"RenderBlock (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderBlock (positioned)";
    }
    if (IsAnonymousBlock())
    {
      return u8"RenderBlock (anonymous)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderBlock (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderBlock (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderBlock (relative positioned)";
    }
    if (IsStickilyPositioned())
    {
      return u8"RenderBlock (sticky positioned)";
    }
    return u8"RenderBlock";
  }

  namespace
  {
    // The weak maps upstream are keyed by address here: entries are removed when either renderer is
    // destroyed.
    using TrackedDescendantsMap = HashMap<const RenderBlock *, UniquePtr<TrackedRendererListHashSet>>;
    using TrackedContainerMap = HashMap<const RenderBox *, HashSet<const RenderBlock *>>;

    TrackedDescendantsMap *s_percentHeightDescendantsMap = nullptr;
    TrackedContainerMap *s_percentHeightContainerMap = nullptr;

    void InsertIntoTrackedRendererMaps(const RenderBlock &container, RenderBox &descendant) noexcept
    {
      if (!s_percentHeightDescendantsMap)
      {
        s_percentHeightDescendantsMap = new TrackedDescendantsMap;
        s_percentHeightContainerMap = new TrackedContainerMap;
      }

      auto &descendantSet = (*s_percentHeightDescendantsMap)[&container];
      if (!descendantSet)
      {
        descendantSet = CreateUnique<TrackedRendererListHashSet>();
      }

      bool added = descendantSet->add(&descendant).second;
      if (!added)
      {
        krys_debug_assert(s_percentHeightContainerMap->contains(&descendant));
        krys_debug_assert(s_percentHeightContainerMap->at(&descendant).contains(&container));
        return;
      }

      auto &containerSet = (*s_percentHeightContainerMap)[&descendant];
      krys_debug_assert(!containerSet.contains(&container));
      containerSet.insert(&container);
    }

    void RemoveFromTrackedRendererMaps(RenderBox &descendant) noexcept
    {
      if (!s_percentHeightDescendantsMap)
      {
        return;
      }

      auto containerSetIterator = s_percentHeightContainerMap->find(&descendant);
      if (containerSetIterator == s_percentHeightContainerMap->end())
      {
        return;
      }
      auto containerSet = krys::move(containerSetIterator->second);
      s_percentHeightContainerMap->erase(containerSetIterator);
      for (const auto *container : containerSet)
      {
        // FIXME: Disabling this assert temporarily until we fix the layout
        // bugs associated with positioned objects not properly cleared from
        // their ancestor chain before being moved. See webkit bug 93766.
        // krys_debug_assert(descendant.IsDescendantOf(container));
        auto descendantsMapIterator = s_percentHeightDescendantsMap->find(container);
        krys_debug_assert(descendantsMapIterator != s_percentHeightDescendantsMap->end());
        if (descendantsMapIterator == s_percentHeightDescendantsMap->end())
        {
          continue;
        }
        auto &descendantSet = descendantsMapIterator->second;
        krys_debug_assert(descendantSet->contains(&descendant));
        descendantSet->remove(&descendant);
        if (descendantSet->isEmpty())
        {
          s_percentHeightDescendantsMap->erase(descendantsMapIterator);
        }
      }
    }

    void RemoveContainerFromTrackedRendererMaps(const RenderBlock &container) noexcept
    {
      if (!s_percentHeightDescendantsMap)
      {
        return;
      }

      auto descendantsMapIterator = s_percentHeightDescendantsMap->find(&container);
      if (descendantsMapIterator == s_percentHeightDescendantsMap->end())
      {
        return;
      }
      for (auto *descendant : *descendantsMapIterator->second)
      {
        auto containerSetIterator = s_percentHeightContainerMap->find(descendant);
        if (containerSetIterator == s_percentHeightContainerMap->end())
        {
          continue;
        }
        containerSetIterator->second.erase(&container);
        if (containerSetIterator->second.empty())
        {
          s_percentHeightContainerMap->erase(containerSetIterator);
        }
      }
      s_percentHeightDescendantsMap->erase(descendantsMapIterator);
    }
  }

  void RenderBlock::WillBeDestroyed() noexcept
  {
    RemoveContainerFromTrackedRendererMaps(*this);
    RenderBox::WillBeDestroyed();
  }

  void RenderBlock::AddPercentHeightDescendant(RenderBox &descendant) noexcept
  {
    InsertIntoTrackedRendererMaps(*this, descendant);
  }

  void RenderBlock::RemovePercentHeightDescendant(RenderBox &descendant) noexcept
  {
    // We query the map directly, rather than looking at style's
    // logicalHeight()/logicalMinHeight()/logicalMaxHeight() since those
    // can change with writing mode/directional changes.
    RemoveFromTrackedRendererMaps(descendant);
  }

  TrackedRendererListHashSet *RenderBlock::PercentHeightDescendants() const noexcept
  {
    if (!s_percentHeightDescendantsMap)
    {
      return nullptr;
    }
    auto it = s_percentHeightDescendantsMap->find(this);
    return it != s_percentHeightDescendantsMap->end() ? it->second.get() : nullptr;
  }

  bool RenderBlock::HasPercentHeightContainerMap() noexcept
  {
    return s_percentHeightContainerMap;
  }

  void RenderBlock::ClearPercentHeightDescendantsFrom(RenderBox &parent) noexcept
  {
    if (!s_percentHeightContainerMap)
    {
      return;
    }

    for (RenderObject *child = parent.FirstChild(); child; child = child->NextInPreOrder(&parent))
    {
      if (auto *box = DynamicDowncast<RenderBox>(*child))
      {
        RemoveFromTrackedRendererMaps(*box);
      }
    }
  }

  LayoutUnit RenderBlock::LogicalLeftOffsetForContent() const noexcept
  {
    LayoutUnit logicalLeftOffset =
      Style().GetWritingMode().IsHorizontal() ? BorderLeft() + PaddingLeft() : BorderTop() + PaddingTop();
    if (IsHorizontalWritingMode() && Style().ShouldPlaceVerticalScrollbarOnLeft())
    {
      logicalLeftOffset += LayoutUnit(VerticalScrollbarWidth());
    }
    return logicalLeftOffset;
  }

  LayoutUnit RenderBlock::LogicalRightOffsetForContent() const noexcept
  {
    LayoutUnit logicalRightOffset =
      Style().GetWritingMode().IsHorizontal() ? BorderLeft() + PaddingLeft() : BorderTop() + PaddingTop();
    if (IsHorizontalWritingMode() && Style().ShouldPlaceVerticalScrollbarOnLeft())
    {
      logicalRightOffset += LayoutUnit(VerticalScrollbarWidth());
    }
    logicalRightOffset += ContentBoxLogicalWidth();
    return logicalRightOffset;
  }

  LayoutUnit RenderBlock::EndOffsetForContent() const noexcept
  {
    return !Style().GetWritingMode().IsLogicalLeftInlineStart()
             ? LogicalLeftOffsetForContent()
             : LogicalWidth() - LogicalRightOffsetForContent();
  }

  LayoutUnit RenderBlock::StartOffsetForContent() const noexcept
  {
    return Style().GetWritingMode().IsLogicalLeftInlineStart()
             ? LogicalLeftOffsetForContent()
             : LogicalWidth() - LogicalRightOffsetForContent();
  }

  LayoutUnit RenderBlock::LogicalRightOffsetForLine(LayoutUnit position,
                                                    LayoutUnit logicalHeight) const noexcept
  {
    return LogicalRightFloatOffsetForLine(position, LogicalRightOffsetForContent(), logicalHeight);
  }

  LayoutUnit RenderBlock::LogicalLeftOffsetForLine(LayoutUnit position,
                                                   LayoutUnit logicalHeight) const noexcept
  {
    return LogicalLeftFloatOffsetForLine(position, LogicalLeftOffsetForContent(), logicalHeight);
  }

  LayoutUnit RenderBlock::EndOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const noexcept
  {
    return !Style().GetWritingMode().IsLogicalLeftInlineStart()
             ? LogicalLeftOffsetForLine(position, logicalHeight)
             : LogicalWidth() - LogicalRightOffsetForLine(position, logicalHeight);
  }

  LayoutUnit RenderBlock::StartOffsetForLine(LayoutUnit position, LayoutUnit logicalHeight) const noexcept
  {
    return Style().GetWritingMode().IsLogicalLeftInlineStart()
             ? LogicalLeftOffsetForLine(position, logicalHeight)
             : LogicalWidth() - LogicalRightOffsetForLine(position, logicalHeight);
  }

  LayoutUnit RenderBlock::AvailableLogicalWidthForLine(LayoutUnit position,
                                                       LayoutUnit logicalHeight) const noexcept
  {
    auto logicalRightOffsetForLine = LogicalRightOffsetForLine(position, logicalHeight);
    auto logicalLeftOffsetForLine = LogicalLeftOffsetForLine(position, logicalHeight);
    return std::max(0_lu, logicalRightOffsetForLine - logicalLeftOffsetForLine);
  }

  bool RenderBlock::HasDefiniteLogicalHeight() const noexcept
  {
    return static_cast<bool>(AvailableLogicalHeightForPercentageComputation());
  }

  Maybe<LayoutUnit> RenderBlock::AvailableLogicalHeightForPercentageComputation() const noexcept
  {
    // For anonymous blocks that are skipped during percentage height calculation,
    // we consider them to have an indefinite height.
    if (SkipContainingBlockForPercentHeightCalculation(*this, false))
    {
      return null;
    }

    auto availableHeight = [&]() -> Maybe<LayoutUnit>
    {
      if (auto overridingLogicalHeightForFlex =
            (IsFlexItem() ? Downcast<RenderFlexibleBox>(Parent())
                              ->UsedFlexItemOverridingLogicalHeightForPercentageResolution(*this)
                          : null))
      {
        return ContentBoxLogicalHeight(*overridingLogicalHeightForFlex);
      }

      if (auto overridingLogicalHeightForGrid = (IsGridItem() ? OverridingBorderBoxLogicalHeight() : null))
      {
        return ContentBoxLogicalHeight(*overridingLogicalHeightForGrid);
      }

      const auto &style = Style();
      if (auto fixedLogicalHeight = style.LogicalHeight().TryFixed())
      {
        auto contentBoxHeight = AdjustContentBoxLogicalHeightForBoxSizing(
          LayoutUnit {fixedLogicalHeight->ResolveZoom(style.UsedZoomForLength())});
        return std::max(0_lu, ConstrainContentBoxLogicalHeightByMinMax(
                                contentBoxHeight - LayoutUnit(ScrollbarLogicalHeight()), null));
      }

      if (ShouldComputeLogicalHeightFromAspectRatio())
      {
        // Only grid is expected to be in a state where it is calculating pref width and having unknown
        // logical width.
        if (IsRenderGrid() && NeedsPreferredLogicalWidthsUpdate() && !style.LogicalWidth().IsSpecified())
        {
          return null;
        }
        return BlockSizeFromAspectRatio(HorizontalBorderAndPaddingExtent(), VerticalBorderAndPaddingExtent(),
                                        static_cast<double>(LayoutUnit {style.LogicalAspectRatio()}),
                                        style.BoxSizingForAspectRatio(), LogicalWidth(),
                                        style.GetAspectRatio(), IsRenderReplaced());
      }

      // A positioned element that specified both top/bottom or that specifies
      // height should be treated as though it has a height explicitly specified
      // that can be used for any percentage computations.
      auto isOutOfFlowPositionedWithSpecifiedHeight =
        IsOutOfFlowPositioned()
        && (!style.LogicalHeight().IsAuto()
            || (!style.LogicalTop().IsAuto() && !style.LogicalBottom().IsAuto()));
      if (isOutOfFlowPositionedWithSpecifiedHeight)
      {
        // Don't allow this to affect the block' size() member variable, since this
        // can get called while the block is still laying out its kids.
        return std::max(0_lu, ComputeLogicalHeight(LogicalHeight(), 0_lu).Extent
                                - BorderAndPaddingLogicalHeight() - LayoutUnit(ScrollbarLogicalHeight()));
      }

      if (style.LogicalHeight().IsPercentOrCalculated())
      {
        if (auto heightWithScrollbar = ComputePercentageLogicalHeight(style.LogicalHeight()))
        {
          auto contentBoxHeightWithScrollbar =
            AdjustContentBoxLogicalHeightForBoxSizing(*heightWithScrollbar);
          // We need to adjust for min/max height because this method does not handle the min/max of the
          // current block, its caller does. So the return value from the recursive call will not have been
          // adjusted yet.
          return std::max(0_lu,
                          ConstrainContentBoxLogicalHeightByMinMax(
                            contentBoxHeightWithScrollbar - LayoutUnit(ScrollbarLogicalHeight()), null));
        }
        return null;
      }

      if (IsRenderView())
      {
        return View().PageOrViewLogicalHeight();
      }

      return null;
    };
    return availableHeight();
  }

  namespace
  {
    KRYS_NODISCARD bool IsRenderBlockFlowOrRenderButton(RenderElement &renderElement) noexcept
    {
      // We include isRenderButton in this check because buttons are implemented
      // using flex box but should still support first-line|first-letter.
      // The flex box and specs require that flex box and grid do not support
      // first-line|first-letter, though.
      // FIXME: Remove when buttons are implemented with align-items instead of
      // flex box.
      return renderElement.IsRenderBlockFlow() || renderElement.IsRenderButton();
    }

    KRYS_NODISCARD RenderBlock *FindFirstLetterBlock(RenderBlock *start) noexcept
    {
      RenderBlock *firstLetterBlock = start;
      while (true)
      {
        bool canHaveFirstLetterRenderer =
          firstLetterBlock->Style().HasPseudoStyle(PseudoElementType::FirstLetter)
          && firstLetterBlock->CanHaveGeneratedChildren()
          && IsRenderBlockFlowOrRenderButton(*firstLetterBlock);
        if (canHaveFirstLetterRenderer)
        {
          return firstLetterBlock;
        }

        RenderElement *parentBlock = firstLetterBlock->Parent();
        if (firstLetterBlock->IsBlockLevelReplacedOrAtomicInline() || !parentBlock
            || parentBlock->FirstChild() != firstLetterBlock
            || !IsRenderBlockFlowOrRenderButton(*parentBlock))
        {
          return nullptr;
        }
        firstLetterBlock = Downcast<RenderBlock>(parentBlock);
      }

      return nullptr;
    }
  }

  Pair<RenderObject *, RenderElement *>
    RenderBlock::FirstLetterAndContainer(RenderObject *skipThisAsFirstLetter) noexcept
  {
    // Don't recur
    if (Style().GetPseudoElementType() == PseudoElementType::FirstLetter)
    {
      return {};
    }

    // FIXME: We need to destroy the first-letter object if it is no longer the first child. Need to find
    // an efficient way to check for that situation though before implementing anything.
    RenderElement *firstLetterContainer = FindFirstLetterBlock(this);
    if (!firstLetterContainer)
    {
      return {};
    }

    // Drill into inlines looking for our first text descendant.
    auto *firstLetter = firstLetterContainer->FirstChild();
    while (firstLetter)
    {
      if (Is<RenderText>(*firstLetter))
      {
        if (firstLetter == skipThisAsFirstLetter)
        {
          firstLetter = firstLetter->NextSibling();
          continue;
        }
        break;
      }

      auto &current = Downcast<RenderElement>(*firstLetter);
      if (Is<RenderListMarker>(current))
      {
        firstLetter = current.NextSibling();
      }
      else if (current.IsFloatingOrOutOfFlowPositioned())
      {
        if (current.Style().GetPseudoElementType() == PseudoElementType::FirstLetter)
        {
          firstLetter = current.FirstChild();
          break;
        }
        firstLetter = current.NextSibling();
      }
      // FIXME: Menu lists, once there are form controls.
      else if (current.IsBlockLevelReplacedOrAtomicInline() || current.IsRenderButton())
      {
        break;
      }
      else if (current.IsFlexibleBoxIncludingDeprecated() || current.IsRenderGrid())
      {
        return {};
      }
      else if (current.Style().HasPseudoStyle(PseudoElementType::FirstLetter)
               && current.CanHaveGeneratedChildren())
      {
        // We found a lower-level node with first-letter, which supersedes the higher-level style
        firstLetterContainer = &current;
        firstLetter = current.FirstChild();
      }
      else
      {
        firstLetter = current.FirstChild();
      }
    }

    if (!firstLetter)
    {
      return {};
    }

    return {firstLetter, firstLetterContainer};
  }

  bool RenderBlock::EstablishesIndependentFormattingContextIgnoringDisplayType(
    const RenderStyle &style) const noexcept
  {
    if (!GetElement())
    {
      krys_debug_assert(IsAnonymous());
      return false;
    }

    auto isBlockBoxWithPotentiallyScrollableOverflow = [&]
    {
      return style.IsDisplayBlockLevel() && style.DoesDisplayGenerateBlockContainer()
             && HasNonVisibleOverflow() && style.OverflowX() != Overflow::Clip
             && style.OverflowX() != Overflow::Visible;
    };

    return style.IsFloating() || style.HasOutOfFlowPosition() || isBlockBoxWithPotentiallyScrollableOverflow()
           || style.UsedContain().Contains(style::ContainValue::Layout)
           || style.GetContainerType() != ContainerType::Normal
           || boo::ShouldApplyPaintContainment(style, *GetElement())
           || (style.IsDisplayBlockLevel() && !style.GetBlockStepSize().IsNone());
  }

  bool RenderBlock::EstablishesIndependentFormattingContext() const noexcept
  {
    const auto &style = Style();
    if (EstablishesIndependentFormattingContextIgnoringDisplayType(style))
    {
      return true;
    }

    if (IsGridItem())
    {
      // Grid items establish a new independent formatting context, unless they're a subgrid
      // https://drafts.csswg.org/css-grid-2/#grid-item-display
      if (!style.GridTemplateColumns().Subgrid && !style.GridTemplateRows().Subgrid)
      {
        return true;
      }
      // Masonry makes grid items not subgrids.
      if (auto *parentGridBox = DynamicDowncast<RenderGrid>(Parent()))
      {
        return parentGridBox->IsMasonry();
      }
    }

    return false;
  }

  bool RenderBlock::CreatesNewFormattingContext() const noexcept
  {
    // Writing-mode changes establish an independent block formatting context
    // if the box is a block-container.
    // https://drafts.csswg.org/css-writing-modes/#block-flow
    if (IsWritingModeRoot() && IsBlockContainer())
    {
      return true;
    }
    const auto &style = Style();
    if (IsBlockContainer() && !style.GetAlignContent().IsNormal())
    {
      return true;
    }
    // FIXME: SVG foreign objects, once there's SVG.
    return IsNonReplacedAtomicInlineLevelBox()
           || style.IsDisplayFlexibleBoxIncludingDeprecatedOrGridFormattingContextBox()
           || IsFlexItemIncludingDeprecated() || IsRenderTable() || IsRenderTableCell()
           || IsRenderTableCaption() || IsFieldset() || IsDocumentElementRenderer()
           || IsRenderFragmentedFlow() || style.SpecifiesColumns() || style.GetColumnSpan() == ColumnSpan::All
           || style.Display() == DisplayType::FlowRoot || EstablishesIndependentFormattingContext();
  }

  void RenderBlock::Layout() noexcept
  {
    // Table cells call layoutBlock directly, so don't add any logic here. Put code into layoutBlock().
    // FIXME: The layout scope, once there's a layout state.
    LayoutBlock(RelayoutChildren::No);

    // FIXME: Clear the layout overflow of a lightweight control clip, once there are form controls.

    InvalidateBackgroundObscurationStatus();
  }

  namespace
  {
    // Lays out the boxes of an inline subtree that aren't part of its lines, and clears the rest.
    void LayoutInlineContentWithoutLineLayout(RenderElement &parent) noexcept
    {
      for (auto *child = parent.FirstChild(); child; child = child->NextSibling())
      {
        if (!child->NeedsLayout())
        {
          continue;
        }
        if (auto *renderInline = DynamicDowncast<RenderInline>(*child))
        {
          LayoutInlineContentWithoutLineLayout(*renderInline);
          renderInline->ClearNeedsLayout();
        }
        else if (auto *element = DynamicDowncast<RenderElement>(*child))
        {
          element->Layout();
        }
        else
        {
          child->ClearNeedsLayout();
        }
      }
    }
  }

  void RenderBlock::LayoutBlock(RelayoutChildren, LayoutUnit) noexcept
  {
    // FIXME: Stands in for the block layout of the subclasses (RenderBlockFlow, the flex box, the grid and
    // the table classes) until they come in the next layout slices: lays out the children without placing or
    // sizing anything, and clears the inline content, which has no lines yet.
    LayoutInlineContentWithoutLineLayout(*this);
    ClearNeedsLayout();
  }

  void RenderBlock::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                  LayoutUnit &maxLogicalWidth) const noexcept
  {
    krys_debug_assert(!ChildrenInline());
    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto width = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *width;
        maxLogicalWidth = *width;
      }
    }
    else if (!ShouldApplyInlineSizeContainment())
    {
      ComputeBlockPreferredLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }

    maxLogicalWidth = std::max(minLogicalWidth, maxLogicalWidth);

    auto scrollbarWidth = LayoutUnit(IntrinsicScrollbarLogicalWidthIncludingGutter());
    maxLogicalWidth += scrollbarWidth;
    minLogicalWidth += scrollbarWidth;
  }

  void RenderBlock::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    _minPreferredLogicalWidth = 0_lu;
    _maxPreferredLogicalWidth = 0_lu;

    const auto &styleToUse = Style();
    auto logicalWidth = OverridingLogicalWidthForFlexBasisComputation().value_or(styleToUse.LogicalWidth());
    if (auto fixedLogicalWidth = logicalWidth.TryFixed();
        !IsRenderTableCell() && fixedLogicalWidth && fixedLogicalWidth->IsPositiveOrZero()
        && !(IsDeprecatedFlexItem()
             && !static_cast<int32>(fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength()))))
    {
      _minPreferredLogicalWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(styleToUse.UsedZoomForLength())});
      _maxPreferredLogicalWidth = _minPreferredLogicalWidth;
    }
    else if (logicalWidth.IsMaxContent())
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth;
    }
    else if (ShouldComputeLogicalWidthFromAspectRatio())
    {
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth =
        (ComputeLogicalWidthFromAspectRatio() - BorderAndPaddingLogicalWidth());
      _minPreferredLogicalWidth = std::max(0_lu, _minPreferredLogicalWidth);
      _maxPreferredLogicalWidth = std::max(0_lu, _maxPreferredLogicalWidth);
    }
    else
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }

    RenderBox::ComputePreferredLogicalWidths(styleToUse.LogicalMinWidth(), styleToUse.LogicalMaxWidth(),
                                             BorderAndPaddingLogicalWidth());

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderBlock::ComputeBlockPreferredLogicalWidths(LayoutUnit &minLogicalWidth,
                                                       LayoutUnit &maxLogicalWidth) const noexcept
  {
    krys_debug_assert(!ShouldApplyInlineSizeContainment());
    auto *containingBlock = this->ContainingBlock();
    if (!containingBlock)
    {
      krys_debug_assert(false);
      return;
    }

    // FIXME: The rendered legend of a fieldset (computePreferredWidthsForExcludedChildren), once there are
    // fieldsets.

    LayoutUnit floatLeftWidth;
    LayoutUnit floatRightWidth;
    auto nowrap = Style().GetTextWrapMode() == TextWrapMode::NoWrap
                  && Style().GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Collapse;
    for (auto &childBox : ChildrenOfType<RenderBox>(*this))
    {
      // Positioned children don't affect the min/max width. Legends in fieldsets are skipped here
      // since they compute outside of any one layout system. Other children excluded from
      // normal layout are only used with block flows, so it's ok to calculate them here.
      // FIXME: Skip legends placed in the border, once there are fieldsets.
      if (childBox.IsOutOfFlowPositioned())
      {
        continue;
      }

      const auto &childStyle = childBox.Style();
      // Either the box itself of its content avoids floats.
      auto childAvoidsFloats =
        childBox.AvoidsFloats() || (childBox.IsAnonymousBlock() && childBox.ChildrenInline());
      if (childBox.IsFloating() || childAvoidsFloats)
      {
        LayoutUnit floatTotalWidth = floatLeftWidth + floatRightWidth;
        auto childUsedClear = RenderStyle::GetUsedClear(childBox);
        if (childUsedClear == UsedClear::Left || childUsedClear == UsedClear::Both)
        {
          maxLogicalWidth = std::max(floatTotalWidth, maxLogicalWidth);
          floatLeftWidth = 0_lu;
        }
        if (childUsedClear == UsedClear::Right || childUsedClear == UsedClear::Both)
        {
          maxLogicalWidth = std::max(floatTotalWidth, maxLogicalWidth);
          floatRightWidth = 0_lu;
        }
      }

      // A margin basically has three types: fixed, percentage, and auto (variable).
      // Auto and percentage margins simply become 0 when computing min/max width.
      // Fixed margins can be added in as is.
      LayoutUnit marginStart;
      LayoutUnit marginEnd;
      const auto &childZoomFactor = childStyle.UsedZoomForLength();
      if (auto fixedMarginStart = childStyle.MarginStart(Style().GetWritingMode()).TryFixed())
      {
        marginStart += LayoutUnit {fixedMarginStart->ResolveZoom(childZoomFactor)};
      }
      if (auto fixedMarginEnd = childStyle.MarginEnd(Style().GetWritingMode()).TryFixed())
      {
        marginEnd += LayoutUnit {fixedMarginEnd->ResolveZoom(childZoomFactor)};
      }
      auto margin = marginStart + marginEnd;

      LayoutUnit childMinPreferredLogicalWidth;
      LayoutUnit childMaxPreferredLogicalWidth;
      ComputeChildPreferredLogicalWidths(const_cast<RenderBox &>(childBox), childMinPreferredLogicalWidth,
                                         childMaxPreferredLogicalWidth);

      auto logicalWidth = childMinPreferredLogicalWidth + margin;
      minLogicalWidth = std::max(logicalWidth, minLogicalWidth);

      // IE ignores tables for calculation of nowrap. Makes some sense.
      if (nowrap && !childBox.IsRenderTable())
      {
        maxLogicalWidth = std::max(logicalWidth, maxLogicalWidth);
      }

      logicalWidth = childMaxPreferredLogicalWidth + margin;

      if (!childBox.IsFloating())
      {
        if (childAvoidsFloats)
        {
          // Determine a left and right max value based off whether or not the floats can fit in the
          // margins of the object.  For negative margins, we will attempt to overlap the float if the
          // negative margin is smaller than the float width.
          bool ltr = containingBlock->Style().GetWritingMode().IsLogicalLeftInlineStart();
          LayoutUnit marginLogicalLeft = ltr ? marginStart : marginEnd;
          LayoutUnit marginLogicalRight = ltr ? marginEnd : marginStart;
          LayoutUnit maxLeft = marginLogicalLeft > 0 ? std::max(floatLeftWidth, marginLogicalLeft)
                                                     : floatLeftWidth + marginLogicalLeft;
          LayoutUnit maxRight = marginLogicalRight > 0 ? std::max(floatRightWidth, marginLogicalRight)
                                                       : floatRightWidth + marginLogicalRight;
          logicalWidth = childMaxPreferredLogicalWidth + maxLeft + maxRight;
          logicalWidth = std::max(logicalWidth, floatLeftWidth + floatRightWidth);
        }
        else
        {
          maxLogicalWidth = std::max(floatLeftWidth + floatRightWidth, maxLogicalWidth);
        }
        floatLeftWidth = 0_lu;
        floatRightWidth = 0_lu;
      }

      if (childBox.IsFloating())
      {
        if (RenderStyle::GetUsedFloat(childBox) == UsedFloat::Left)
        {
          floatLeftWidth += logicalWidth;
        }
        else
        {
          floatRightWidth += logicalWidth;
        }
      }
      else
      {
        maxLogicalWidth = std::max(logicalWidth, maxLogicalWidth);
      }
    }

    // Always make sure these values are non-negative.
    minLogicalWidth = std::max(0_lu, minLogicalWidth);
    maxLogicalWidth = std::max(0_lu, maxLogicalWidth);

    maxLogicalWidth = std::max(floatLeftWidth + floatRightWidth, maxLogicalWidth);
  }

  void RenderBlock::ComputeChildIntrinsicLogicalWidths(RenderBox &child, LayoutUnit &minPreferredLogicalWidth,
                                                       LayoutUnit &maxPreferredLogicalWidth) const noexcept
  {
    minPreferredLogicalWidth = child.MinPreferredLogicalWidth();
    maxPreferredLogicalWidth = child.MaxPreferredLogicalWidth();
  }

  void RenderBlock::ComputeChildPreferredLogicalWidths(RenderBox &childBox,
                                                       LayoutUnit &minPreferredLogicalWidth,
                                                       LayoutUnit &maxPreferredLogicalWidth) const noexcept
  {
    if (childBox.IsHorizontalWritingMode() != IsHorizontalWritingMode())
    {
      // If the child is an orthogonal flow, child's height determines the width,
      // but the height is not available until layout.
      // http://dev.w3.org/csswg/css-writing-modes-3/#orthogonal-shrink-to-fit
      if (!childBox.NeedsLayout())
      {
        minPreferredLogicalWidth = childBox.LogicalHeight();
        maxPreferredLogicalWidth = childBox.LogicalHeight();
        return;
      }
      const auto &childBoxStyle = childBox.Style();
      if (auto fixedChildBoxStyleLogicalWidth = childBoxStyle.LogicalWidth().TryFixed();
          childBox.ShouldComputeLogicalHeightFromAspectRatio() && fixedChildBoxStyleLogicalWidth)
      {
        auto aspectRatioSize = BlockSizeFromAspectRatio(
          childBox.HorizontalBorderAndPaddingExtent(), childBox.VerticalBorderAndPaddingExtent(),
          LayoutUnit {childBoxStyle.LogicalAspectRatio()}.ToDouble(), childBoxStyle.BoxSizingForAspectRatio(),
          LayoutUnit {fixedChildBoxStyleLogicalWidth->ResolveZoom(childBoxStyle.UsedZoomForLength())},
          Style().GetAspectRatio(), IsRenderReplaced());
        minPreferredLogicalWidth = aspectRatioSize;
        maxPreferredLogicalWidth = aspectRatioSize;
        return;
      }
      auto logicalHeightWithoutLayout = childBox.ComputeLogicalHeightWithoutLayout();
      minPreferredLogicalWidth = logicalHeightWithoutLayout;
      maxPreferredLogicalWidth = logicalHeightWithoutLayout;
      return;
    }

    ComputeChildIntrinsicLogicalWidths(childBox, minPreferredLogicalWidth, maxPreferredLogicalWidth);

    // For non-replaced blocks if the inline size is min|max-content or a definite
    // size the min|max-content contribution is that size plus border, padding and
    // margin https://drafts.csswg.org/css-sizing/#block-intrinsic
    if (!Is<RenderBlock>(childBox))
    {
      return;
    }
    const auto &computedInlineSize = childBox.Style().LogicalWidth();
    if (computedInlineSize.IsMaxContent())
    {
      minPreferredLogicalWidth = maxPreferredLogicalWidth;
    }
    else if (computedInlineSize.IsMinContent())
    {
      maxPreferredLogicalWidth = minPreferredLogicalWidth;
    }
  }
}
