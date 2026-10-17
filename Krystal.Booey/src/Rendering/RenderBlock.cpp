#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Core/UnicodeBidi.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/LayoutScope.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
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

  namespace
  {
    // The weak maps upstream are keyed by address here: a containing block's entries are removed when it's
    // destroyed, and a box's when it's destroyed or leaves its containing block.
    class OutOfFlowDescendantsMap
    {
    private:
      using DescendantsMap = HashMap<const RenderBlock *, UniquePtr<TrackedRendererListHashSet>>;
      using ContainerMap = HashMap<const RenderBox *, const RenderBlock *>;

      DescendantsMap _descendantsMap;
      ContainerMap _containerMap;

    public:
      void AddDescendant(const RenderBlock &containingBlock, RenderBox &outOfFlowDescendant) noexcept
      {
        // Protect against double insert where a descendant would end up with multiple containing blocks.
        if (auto previous = _containerMap.find(&outOfFlowDescendant);
            previous != _containerMap.end() && previous->second != &containingBlock)
        {
          if (auto *descendants = PositionedRenderers(*previous->second))
          {
            descendants->remove(&outOfFlowDescendant);
          }
        }

        auto &descendants = _descendantsMap[&containingBlock];
        if (!descendants)
        {
          descendants = CreateUnique<TrackedRendererListHashSet>();
        }

        auto isNewEntry = false;
        if (!Is<RenderView>(containingBlock) || descendants->isEmpty())
        {
          isNewEntry = descendants->add(&outOfFlowDescendant).second;
        }
        else if (outOfFlowDescendant.IsFixedPositioned()
                 || dom::IsInTopLayerOrBackdrop(outOfFlowDescendant.Style(),
                                                outOfFlowDescendant.GetElement()))
        {
          isNewEntry = descendants->appendOrMoveToLast(&outOfFlowDescendant).second;
        }
        else
        {
          auto ensureLayoutDepentBoxPosition = [&]
          {
            // RenderView is a special containing block as it may hold both absolute and fixed positioned
            // containing blocks. When a fixed positioned box is also a descendant of an absolute positioned
            // box anchored to the RenderView, we have to make sure that the absolute positioned box is
            // inserted before the fixed box to follow block layout dependency.
            for (auto it = descendants->begin(); it != descendants->end(); ++it)
            {
              if ((*it)->IsFixedPositioned())
              {
                isNewEntry = descendants->insertBefore(it, &outOfFlowDescendant).second;
                return;
              }
            }
            isNewEntry = descendants->appendOrMoveToLast(&outOfFlowDescendant).second;
          };
          ensureLayoutDepentBoxPosition();
        }

        if (!isNewEntry)
        {
          krys_debug_assert(_containerMap.contains(&outOfFlowDescendant));
          return;
        }
        _containerMap[&outOfFlowDescendant] = &containingBlock;
      }

      void RemoveDescendant(const RenderBox &outOfFlowDescendant) noexcept
      {
        auto containerIterator = _containerMap.find(&outOfFlowDescendant);
        if (containerIterator == _containerMap.end())
        {
          return;
        }
        const auto *containingBlock = containerIterator->second;
        _containerMap.erase(containerIterator);

        auto descendantsIterator = _descendantsMap.find(containingBlock);
        krys_debug_assert(descendantsIterator != _descendantsMap.end());
        if (descendantsIterator == _descendantsMap.end())
        {
          return;
        }

        auto &descendants = descendantsIterator->second;
        auto *descendant = const_cast<RenderBox *>(&outOfFlowDescendant);
        krys_debug_assert(descendants->contains(descendant));

        descendants->remove(descendant);
        if (descendants->isEmpty())
        {
          _descendantsMap.erase(descendantsIterator);
        }
      }

      void RemoveContainingBlock(const RenderBlock &containingBlock) noexcept
      {
        auto descendantsIterator = _descendantsMap.find(&containingBlock);
        if (descendantsIterator == _descendantsMap.end())
        {
          return;
        }
        auto descendants = krys::move(descendantsIterator->second);
        _descendantsMap.erase(descendantsIterator);

        for (auto *renderer : *descendants)
        {
          _containerMap.erase(renderer);
        }
      }

      KRYS_NODISCARD TrackedRendererListHashSet *
        PositionedRenderers(const RenderBlock &containingBlock) const noexcept
      {
        auto it = _descendantsMap.find(&containingBlock);
        return it != _descendantsMap.end() ? it->second.get() : nullptr;
      }
    };

    KRYS_NODISCARD OutOfFlowDescendantsMap &GetOutOfFlowDescendantsMap() noexcept
    {
      static OutOfFlowDescendantsMap mapForOutOfFlowDescendants;
      return mapForOutOfFlowDescendants;
    }
  }

  void RenderBlock::WillBeDestroyed() noexcept
  {
    RemoveFromUpdateScrollInfoAfterLayoutTransaction();
    RemoveContainerFromTrackedRendererMaps(*this);
    GetOutOfFlowDescendantsMap().RemoveContainingBlock(*this);
    RenderBox::WillBeDestroyed();
  }

  TrackedRendererListHashSet *RenderBlock::OutOfFlowBoxes() const noexcept
  {
    return GetOutOfFlowDescendantsMap().PositionedRenderers(*this);
  }

  void RenderBlock::AddOutOfFlowBox(RenderBox &outOfFlowBox) noexcept
  {
    krys_debug_assert(outOfFlowBox.IsOutOfFlowPositioned());
    krys_debug_assert(!IsAnonymousBlock());

    outOfFlowBox.ClearGridAreaContentSize();

    if (outOfFlowBox.IsRenderFragmentedFlow())
    {
      return;
    }
    // FIXME: Find out if we can do this as part of outOfFlowBox.setChildNeedsLayout(MarkOnlyThis)
    if (outOfFlowBox.NeedsLayout())
    {
      // We should turn this bit on only while in layout.
      krys_debug_assert(OutOfFlowChildNeedsLayout() || View().FrameView().LayoutContext().IsInLayout());
      SetOutOfFlowChildNeedsLayoutBit(true);
    }
    GetOutOfFlowDescendantsMap().AddDescendant(*this, outOfFlowBox);
  }

  void RenderBlock::RemoveOutOfFlowBox(const RenderBox &rendererToRemove) noexcept
  {
    GetOutOfFlowDescendantsMap().RemoveDescendant(rendererToRemove);
  }

  namespace
  {
    void MarkRendererAndParentForLayout(RenderBox &renderer) noexcept
    {
      renderer.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      if (renderer.ShouldInvalidatePreferredWidths())
      {
        renderer.SetNeedsPreferredWidthsUpdate(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
      auto *parentBlock = RenderObject::ContainingBlockForPositionType(PositionType::Static, renderer);
      if (!parentBlock)
      {
        krys_debug_assert(false);
        return;
      }
      // Parent has to be mark for layout to run static positioning on the out-of-flow content.
      parentBlock->SetChildNeedsLayout();
    }
  }

  void RenderBlock::RemoveOutOfFlowBoxes(const RenderBlock *newContainingBlockCandidate,
                                         ContainingBlockState containingBlockState) noexcept
  {
    auto *outOfFlowDescendants = OutOfFlowBoxes();
    if (!outOfFlowDescendants)
    {
      return;
    }

    List<RenderBox *> renderersToRemove;
    if (!newContainingBlockCandidate)
    {
      // We don't form containing block for these boxes anymore (either through style change or internal
      // render tree shuffle)
      for (auto *renderer : *outOfFlowDescendants)
      {
        renderersToRemove.push_back(renderer);

        MarkRendererAndParentForLayout(*renderer);
        auto markNewContainingBlockForLayout = [&]
        {
          auto isAbsolutePositioned = renderer->IsAbsolutelyPositioned();
          // During style change we can't tell which ancestor is going to be the final containing block, so
          // let's just mark the new candidate dirty.
          auto *newContainingBlock = ContainingBlock();
          for (; newContainingBlock
                 && (isAbsolutePositioned ? !newContainingBlock->CanContainAbsolutelyPositionedObjects()
                                          : newContainingBlock->CanContainFixedPositionObjects());
               newContainingBlock = newContainingBlock->ContainingBlock())
          {
          }
          if (newContainingBlock)
          {
            newContainingBlock->SetNeedsLayout();
          }
        };
        markNewContainingBlockForLayout();
      }
    }
    else if (containingBlockState == ContainingBlockState::NewContainingBlock)
    {
      // Some of the positioned boxes are getting transferred over to newContainingBlockCandidate.
      for (auto *renderer : *outOfFlowDescendants)
      {
        if (!renderer->IsDescendantOf(newContainingBlockCandidate))
        {
          continue;
        }
        renderersToRemove.push_back(renderer);
        MarkRendererAndParentForLayout(*renderer);
      }
    }
    else
    {
      krys_debug_assert(false);
    }

    for (auto *renderer : renderersToRemove)
    {
      RemoveOutOfFlowBox(*renderer);
    }
  }

  bool RenderBlock::IsContainingBlockAncestorFor(RenderObject &renderer) const noexcept
  {
    for (const auto *ancestor = renderer.ContainingBlock(); ancestor; ancestor = ancestor->ContainingBlock())
    {
      if (ancestor == this)
      {
        return true;
      }
    }
    return false;
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

  TextRun RenderBlock::ConstructTextRun(utf8_stringview stringView, const RenderStyle &style,
                                        ExpansionBehavior expansion, EnumSet<TextRunFlag> flags) noexcept
  {
    auto textDirection = TextDirection::LTR;
    bool directionalOverride = style.RtlOrdering() == Order::Visual;
    if (flags.Contains(TextRunFlag::RespectDirection))
    {
      textDirection = style.GetWritingMode().BidiDirection();
    }
    if (flags.Contains(TextRunFlag::RespectDirectionOverride))
    {
      directionalOverride |= IsOverride(style.GetUnicodeBidi());
    }

    // The security disc characters (-webkit-text-security) were dropped, so the text is used as it is.
    return TextRun {stringView, 0, 0, expansion, textDirection, directionalOverride};
  }

  TextRun RenderBlock::ConstructTextRun(const RenderText &text, const RenderStyle &style,
                                        ExpansionBehavior expansion) noexcept
  {
    return ConstructTextRun(utf8_stringview {text.Text()}, style, expansion);
  }

  TextRun RenderBlock::ConstructTextRun(const RenderText &text, size_t offset, size_t length,
                                        const RenderStyle &style, ExpansionBehavior expansion) noexcept
  {
    krys_debug_assert(offset + length <= text.Text().size());
    return ConstructTextRun(utf8_stringview {text.Text()}.substr(offset, length), style, expansion);
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

  void RenderBlock::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    const RenderStyle *oldStyle = HasInitializedStyle() ? &Style() : nullptr;
    SetBlockLevelReplacedOrAtomicInline(newStyle.IsDisplayInlineType());
    if (oldStyle)
    {
      RemoveOutOfFlowBoxesIfNeededOnStyleChange(*this, *oldStyle, newStyle);
      // FIXME: A legend that starts floating rejoins normal layout, once there are fieldsets.
    }
    RenderBox::StyleWillChange(diff, newStyle);
  }

  bool RenderBlock::ScrollbarWidthDidChange(const RenderStyle &oldStyle, const RenderStyle &newStyle,
                                            ScrollbarOrientation orientation) const noexcept
  {
    return (orientation == ScrollbarOrientation::Vertical ? IncludeVerticalScrollbarSize()
                                                          : IncludeHorizontalScrollbarSize())
           && oldStyle.GetScrollbarWidth() != newStyle.GetScrollbarWidth();
  }

  bool RenderBlock::PaddingBoxLogicaHeightChanged(const RenderStyle &oldStyle,
                                                  const RenderStyle &newStyle) const noexcept
  {
    auto scrollbarHeightDidChange = [&](auto orientation)
    {
      return (orientation == ScrollbarOrientation::Vertical ? IncludeVerticalScrollbarSize()
                                                            : IncludeHorizontalScrollbarSize())
             && oldStyle.GetScrollbarWidth() != newStyle.GetScrollbarWidth();
    };
    if (newStyle.GetWritingMode().IsHorizontal())
    {
      return oldStyle.BorderTopWidth() != newStyle.BorderTopWidth()
             || oldStyle.BorderBottomWidth() != newStyle.BorderBottomWidth()
             || scrollbarHeightDidChange(ScrollbarOrientation::Horizontal);
    }
    return oldStyle.BorderLeftWidth() != newStyle.BorderLeftWidth()
           || oldStyle.BorderRightWidth() != newStyle.BorderRightWidth()
           || scrollbarHeightDidChange(ScrollbarOrientation::Vertical);
  }

  bool RenderBlock::ContentBoxLogicalWidthChanged(const RenderStyle &oldStyle,
                                                  const RenderStyle &newStyle) const noexcept
  {
    if (newStyle.GetWritingMode().IsHorizontal())
    {
      return oldStyle.BorderLeftWidth() != newStyle.BorderLeftWidth()
             || oldStyle.BorderRightWidth() != newStyle.BorderRightWidth()
             || oldStyle.PaddingLeft() != newStyle.PaddingLeft()
             || oldStyle.PaddingRight() != newStyle.PaddingRight()
             || ScrollbarWidthDidChange(oldStyle, newStyle, ScrollbarOrientation::Vertical);
    }

    return oldStyle.BorderTopWidth() != newStyle.BorderTopWidth()
           || oldStyle.BorderBottomWidth() != newStyle.BorderBottomWidth()
           || oldStyle.PaddingTop() != newStyle.PaddingTop()
           || oldStyle.PaddingBottom() != newStyle.PaddingBottom()
           || ScrollbarWidthDidChange(oldStyle, newStyle, ScrollbarOrientation::Horizontal);
  }

  void RenderBlock::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBox::StyleDidChange(diff, oldStyle);

    // FIXME: Adjust the fragmented flow state when the containing block changes, once there are fragmented
    // flows.

    PropagateStyleToAnonymousChildren(StylePropagationType::BlockAndRubyChildren);

    // It's possible for our border/padding to change, but for the overall logical width of the block to
    // end up being the same. We keep track of this change so in layoutBlock, we can know to set
    // relayoutChildren=true.
    auto shouldForceRelayoutChildren = false;
    if (oldStyle && diff == style::DifferenceResult::Layout && NeedsLayout())
    {
      // Out-of-flow boxes anchored to the padding box.
      shouldForceRelayoutChildren =
        ContentBoxLogicalWidthChanged(*oldStyle, Style())
        || (OutOfFlowBoxes() && PaddingBoxLogicaHeightChanged(*oldStyle, Style()));
    }
    SetShouldForceRelayoutChildren(shouldForceRelayoutChildren);
  }

  bool RenderBlock::ChildrenPreventSelfCollapsing() const noexcept
  {
    // Whether or not we collapse is dependent on whether all our normal flow children
    // are also self-collapsing.
    for (RenderBox *child = FirstChildBox(); child; child = child->NextSiblingBox())
    {
      if (child->IsFloatingOrOutOfFlowPositioned())
      {
        continue;
      }
      if (!child->IsSelfCollapsingBlock())
      {
        return true;
      }
    }
    return false;
  }

  bool RenderBlock::IsSelfCollapsingBlock() const noexcept
  {
    // We are not self-collapsing if we
    // (a) have a non-zero height according to layout (an optimization to avoid wasting time)
    // (b) are a table,
    // (c) have border/padding,
    // (d) have a min-height
    // (e) have specified that one of our margins can't collapse using a CSS extension

    if (IsOutOfFlowPositioned())
    {
      return false;
    }

    auto minHeightIsPositive = [&]
    {
      const auto &logicalMinHeight = Style().LogicalMinHeight();
      if (auto fixedValue = logicalMinHeight.TryFixed())
      {
        return fixedValue->IsPositive();
      }
      if (auto percentageValue = logicalMinHeight.TryPercentage())
      {
        return percentageValue->IsPositive();
      }
      return logicalMinHeight.IsCalculated();
    };

    auto heightIsZeroOrAuto = [&]
    {
      auto handleNonZeroPercentageOrCalc = [&]
      {
        // While in quirks mode there's always a fixed height ancestor to resolve percent value against (ICB),
        // in standards mode we can only use the containing block.
        if (GetDocument().InQuirksMode())
        {
          return false;
        }
        auto *containingBlock = this->ContainingBlock();
        if (!containingBlock)
        {
          krys_debug_assert(false);
          return false;
        }
        return Is<RenderView>(*containingBlock) || !containingBlock->Style().LogicalHeight().IsFixed();
      };

      const auto &logicalHeight = Style().LogicalHeight();
      if (auto fixedValue = logicalHeight.TryFixed())
      {
        return fixedValue->IsZero();
      }
      if (auto percentageValue = logicalHeight.TryPercentage())
      {
        if (percentageValue->IsZero())
        {
          return true;
        }
        return handleNonZeroPercentageOrCalc();
      }
      if (logicalHeight.IsCalculated())
      {
        return handleNonZeroPercentageOrCalc();
      }
      return logicalHeight.IsAuto();
    };

    if (LogicalHeight() > 0 || IsRenderTable() || BorderAndPaddingLogicalHeight() || minHeightIsPositive())
    {
      return false;
    }

    if (heightIsZeroOrAuto())
    {
      // If the height is 0 or auto, then whether or not we are a self-collapsing block depends
      // on whether we have content that is all self-collapsing or not.
      return !CreatesNewFormattingContext() && !ChildrenPreventSelfCollapsing();
    }

    return false;
  }

  void RenderBlock::Layout() noexcept
  {
    // Table cells call layoutBlock directly, so don't add any logic here. Put code into layoutBlock().
    {
      auto scope = LayoutScope {*this};
      LayoutBlock(RelayoutChildren::No);
    }

    // FIXME: Clear the layout overflow of a lightweight control clip, once there are form controls.

    InvalidateBackgroundObscurationStatus();
  }

  bool RenderBlock::RecomputeLogicalWidth() noexcept
  {
    LayoutUnit oldWidth = LogicalWidth();

    UpdateLogicalWidth();

    bool hasBorderOrPaddingLogicalWidthChanged = this->HasBorderOrPaddingLogicalWidthChanged();
    SetShouldForceRelayoutChildren(false);

    return oldWidth != LogicalWidth() || hasBorderOrPaddingLogicalWidthChanged;
  }

  namespace
  {
    // Lays out the boxes of an inline subtree that aren't part of its lines, and clears the rest. Out-of-flow
    // boxes are given to their containing block, with the start of the block's content as their static
    // position.
    void LayoutInlineContentWithoutLineLayout(RenderBlock &block, RenderElement &parent) noexcept
    {
      for (auto *child = parent.FirstChild(); child; child = child->NextSibling())
      {
        if (auto *box = DynamicDowncast<RenderBox>(*child); box && box->IsOutOfFlowPositioned())
        {
          box->ContainingBlock()->AddOutOfFlowBox(*box);
          box->Layer()->SetStaticInlinePosition(block.StartOffsetForContent());
          box->Layer()->SetStaticBlockPosition(block.BorderAndPaddingBefore());
          continue;
        }
        if (!child->NeedsLayout())
        {
          continue;
        }
        if (auto *renderInline = DynamicDowncast<RenderInline>(*child))
        {
          LayoutInlineContentWithoutLineLayout(block, *renderInline);
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
    // FIXME: Stands in for the layout of the table classes until they're ported (upstream asserts this isn't
    // reached): lays out the children without placing or sizing anything, and clears the inline content,
    // which has no lines yet.
    LayoutChildrenWithoutLineLayout();
  }

  void RenderBlock::LayoutChildrenWithoutLineLayout() noexcept
  {
    LayoutInlineContentWithoutLineLayout(*this, *this);
  }

  // Overflow is always relative to the border-box of the element in question.
  // Therefore, if the element has a vertical scrollbar placed on the left, an overflow rect at x=2px would
  // conceptually intersect the scrollbar.
  void RenderBlock::ComputeOverflow(LayoutUnit oldClientAfterEdge,
                                    EnumSet<ComputeOverflowOptions> options) noexcept
  {
    ClearOverflow();
    AddOverflowFromInFlowChildren(options);
    AddOverflowFromOutOfFlowBoxes();

    if (HasNonVisibleOverflow())
    {
      auto includePaddingAfter = [&]
      {
        // When we have overflow clip, propagate the original spillout since it will include collapsed bottom
        // margins and bottom padding.
        auto clientRect = FlippedClientBoxRect();
        auto rectToApply = clientRect;
        // Set the axis we don't care about to be 1, since we want this overflow to always be considered
        // reachable.
        if (IsHorizontalWritingMode())
        {
          rectToApply.SetWidth(1_lu);
          rectToApply.SetHeight(std::max(0_lu, oldClientAfterEdge - clientRect.Y()));
        }
        else
        {
          rectToApply.SetWidth(std::max(0_lu, oldClientAfterEdge - clientRect.X()));
          rectToApply.SetHeight(1_lu);
        }
        AddLayoutOverflow(rectToApply);
      };
      includePaddingAfter();
      if (HasRenderOverflow())
      {
        _overflow->SetLayoutClientAfterEdge(oldClientAfterEdge);
      }
    }

    // Add visual overflow from box-shadow, border-image-outset and outline.
    AddVisualEffectOverflow();

    // Add visual overflow from theme.
    AddVisualOverflowFromTheme();
  }

  void RenderBlock::ClearLayoutOverflow() noexcept
  {
    if (!_overflow)
    {
      return;
    }

    if (VisualOverflowRect() == BorderBoxRect())
    {
      // FIXME: Implement complete solution for fragments overflow.
      ClearOverflow();
      return;
    }

    _overflow->SetLayoutOverflow(BorderBoxRect());
  }

  void RenderBlock::AddOverflowFromOutOfFlowBoxes() noexcept
  {
    TrackedRendererListHashSet *outOfFlowDescendants = OutOfFlowBoxes();
    if (!outOfFlowDescendants)
    {
      return;
    }

    for (auto *outOfFlowBox : *outOfFlowDescendants)
    {
      // Fixed positioned elements don't contribute to layout overflow, since they don't scroll with the
      // content.
      if (outOfFlowBox->IsAbsolutelyPositioned())
      {
        AddOverflowFromContainedBox(*outOfFlowBox);
      }
    }
  }

  void RenderBlock::AddVisualOverflowFromTheme() noexcept
  {
    if (!Style().HasUsedAppearance())
    {
      return;
    }

    // FIXME: The theme's repaint rect (RenderTheme::adjustRepaintRect), once there's a theme.
    AddVisualOverflow(LayoutRect(SnappedIntRect(BorderBoxRect())));
  }

  void RenderBlock::SetLogicalLeftForChild(RenderBox &child, LayoutUnit logicalLeft,
                                           ApplyLayoutDeltaMode applyDelta) noexcept
  {
    if (IsHorizontalWritingMode())
    {
      if (applyDelta == ApplyLayoutDelta)
      {
        View().FrameView().LayoutContext().AddLayoutDelta(LayoutSize(child.X() - logicalLeft, 0_lu));
      }
      child.SetX(logicalLeft);
    }
    else
    {
      if (applyDelta == ApplyLayoutDelta)
      {
        View().FrameView().LayoutContext().AddLayoutDelta(LayoutSize(0_lu, child.Y() - logicalLeft));
      }
      child.SetY(logicalLeft);
    }
  }

  void RenderBlock::SetLogicalTopForChild(RenderBox &child, LayoutUnit logicalTop,
                                          ApplyLayoutDeltaMode applyDelta) noexcept
  {
    if (IsHorizontalWritingMode())
    {
      if (applyDelta == ApplyLayoutDelta)
      {
        View().FrameView().LayoutContext().AddLayoutDelta(LayoutSize(0_lu, child.Y() - logicalTop));
      }
      child.SetY(logicalTop);
    }
    else
    {
      if (applyDelta == ApplyLayoutDelta)
      {
        View().FrameView().LayoutContext().AddLayoutDelta(LayoutSize(child.X() - logicalTop, 0_lu));
      }
      child.SetX(logicalTop);
    }
  }

  void RenderBlock::UpdateBlockChildDirtyBitsBeforeLayout(RelayoutChildren relayoutChildren,
                                                          RenderBox &child) noexcept
  {
    if (child.IsOutOfFlowPositioned())
    {
      return;
    }

    // FIXME: Technically percentage height objects only need a relayout if their percentage isn't going to be
    // turned into an auto value. Add a method to determine this, so that we can avoid the relayout.
    auto childHasRelativeHeight = [&]
    {
      const auto &style = child.Style();
      return style.Height().IsPercentOrCalculated() || style.MinHeight().IsPercentOrCalculated()
             || style.MaxHeight().IsPercentOrCalculated();
    };
    if (relayoutChildren == RelayoutChildren::Yes || (childHasRelativeHeight() && !IsRenderView()))
    {
      child.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
    }

    // If relayoutChildren is set and the child has percentage padding or an embedded content box, we also
    // need to invalidate the childs pref widths.
    if (relayoutChildren == RelayoutChildren::Yes && child.ShouldInvalidatePreferredWidths())
    {
      child.SetNeedsPreferredWidthsUpdate(MarkingBehavior::MarkOnlyThis);
    }
  }

  void RenderBlock::SimplifiedNormalFlowLayout() noexcept
  {
    krys_debug_assert(!ChildrenInline());

    for (auto &box : ChildrenOfType<RenderBox>(*this))
    {
      if (!box.IsOutOfFlowPositioned())
      {
        box.LayoutIfNeeded();
      }
    }
  }

  bool RenderBlock::CanPerformSimplifiedLayout() const noexcept
  {
    if (SelfNeedsLayout() || NormalChildNeedsLayout() || OutOfFlowChildNeedsStaticPositionLayout())
    {
      return false;
    }
    // FIXME: Content skipped by content-visibility (wasSkippedDuringLastLayoutDueToContentVisibility,
    // isSkippedContentRootForLayout), once layout skips content.
    return OutOfFlowChildNeedsLayout() || NeedsSimplifiedNormalFlowLayout();
  }

  bool RenderBlock::SimplifiedLayout() noexcept
  {
    if (!CanPerformSimplifiedLayout())
    {
      return false;
    }

    LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                      IsTransformed() || Style().GetWritingMode().IsBlockFlipped());
    if (NeedsOutOfFlowMovementLayout() && !TryLayoutDoingOutOfFlowMovementOnly())
    {
      return false;
    }

    // Lay out positioned descendants or objects that just need to recompute overflow.
    if (NeedsSimplifiedNormalFlowLayout())
    {
      SimplifiedNormalFlowLayout();
    }

    // FIXME: Apply a forced break after the content of a fragmented flow, once there are fragmented flows.

    // Lay out our positioned objects if our positioned child bit is set.
    // Also, if an absolute position element inside a relative positioned container moves, and the absolute
    // element has a fixed position child, neither the fixed element nor its container learn of the movement
    // since outOfFlowChildNeedsLayout() is only marked as far as the relative positioned container. So if we
    // can have fixed pos objects in our positioned objects list check if any of them are statically
    // positioned and thus need to move with their absolute ancestors.
    bool canContainFixedPosObjects = CanContainFixedPositionObjects();
    if (OutOfFlowChildNeedsLayout() || canContainFixedPosObjects)
    {
      LayoutOutOfFlowBoxes(RelayoutChildren::No, !OutOfFlowChildNeedsLayout() && canContainFixedPosObjects);
    }

    // Recompute our overflow information.
    // FIXME: We could do better here by computing a temporary overflow object from layoutOutOfFlowBoxes and
    // only updating our overflow if we either used to have overflow or if the new temporary object has
    // overflow. For now just always recompute overflow.  This is no worse performance-wise than the old code
    // that called rightmostPosition and lowestPosition on every relayout so it's not a regression.
    // computeOverflow expects the bottom edge before we clamp our height. Since this information isn't
    // available during simplifiedLayout, we cache the value in m_overflow.
    LayoutUnit oldClientAfterEdge =
      HasRenderOverflow() ? _overflow->LayoutClientAfterEdge() : ClientLogicalBottom();
    ComputeOverflow(oldClientAfterEdge, {ComputeOverflowOptions::RecomputeFloats});

    UpdateLayerTransform();

    UpdateScrollInfoAfterLayout();

    ClearNeedsLayout();
    return true;
  }

  void RenderBlock::MarkFixedPositionBoxForLayoutIfNeeded(RenderBox &positionedChild) noexcept
  {
    if (positionedChild.Style().GetPosition() != PositionType::Fixed)
    {
      return;
    }

    bool hasStaticBlockPosition = positionedChild.Style().HasStaticBlockPosition(IsHorizontalWritingMode());
    bool hasStaticInlinePosition = positionedChild.Style().HasStaticInlinePosition(IsHorizontalWritingMode());
    if (!hasStaticBlockPosition && !hasStaticInlinePosition)
    {
      return;
    }

    auto *parent = positionedChild.Parent();
    while (parent && !Is<RenderView>(*parent) && parent->Style().GetPosition() != PositionType::Absolute)
    {
      parent = parent->Parent();
    }
    if (!parent || parent->Style().GetPosition() != PositionType::Absolute)
    {
      return;
    }

    if (hasStaticInlinePosition)
    {
      LogicalExtentComputedValues computedValues;
      positionedChild.ComputeLogicalWidth(computedValues);
      LayoutUnit newLeft = computedValues.Position;
      if (newLeft != positionedChild.LogicalLeft())
      {
        positionedChild.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
    }
    else if (hasStaticBlockPosition)
    {
      auto logicalTop = positionedChild.LogicalTop();
      if (logicalTop
          != positionedChild.ComputeLogicalHeight(positionedChild.LogicalHeight(), logicalTop).Position)
      {
        positionedChild.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      }
    }
  }

  void RenderBlock::LayoutOutOfFlowBox(RenderBox &outOfFlowBox, RelayoutChildren relayoutChildren,
                                       bool fixedPositionObjectsOnly) noexcept
  {
    krys_debug_assert(outOfFlowBox.IsOutOfFlowPositioned());

    // FIXME: Content skipped by content-visibility (isSkippedContentRootForLayout), once layout skips
    // content, and the box's fragment range, once there are fragmented flows.

    // A fixed position element with an absolute positioned ancestor has no way of knowing if the latter has
    // changed position. So if this is a fixed position element, mark it for layout if it has an abspos
    // ancestor and needs to move with that ancestor, i.e. it has static position.
    MarkFixedPositionBoxForLayoutIfNeeded(outOfFlowBox);
    if (fixedPositionObjectsOnly)
    {
      outOfFlowBox.LayoutIfNeeded();
      return;
    }

    // When a non-positioned block element moves, it may have positioned children that are implicitly
    // positioned relative to the non-positioned block.  Rather than trying to detect all of these movement
    // cases, we just always lay out positioned objects that are positioned implicitly like this.  Such
    // objects are rare, and so in typical DHTML menu usage (where everything is positioned explicitly) this
    // should not incur a performance penalty.
    if (relayoutChildren == RelayoutChildren::Yes
        || (outOfFlowBox.Style().HasStaticBlockPosition(IsHorizontalWritingMode())
            && outOfFlowBox.Parent() != this))
    {
      outOfFlowBox.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
    }

    // If relayoutChildren is set and the child has percentage padding or an embedded content box, we also
    // need to invalidate the childs pref widths.
    if (relayoutChildren == RelayoutChildren::Yes && outOfFlowBox.ShouldInvalidatePreferredWidths())
    {
      outOfFlowBox.SetNeedsPreferredWidthsUpdate(MarkingBehavior::MarkOnlyThis);
    }

    // FIXME: Mark the box for relayout when its pagination changed, once there's pagination.

    // We don't have to do a full layout.  We just have to update our position. Try that first. If we have
    // shrink-to-fit width and we hit the available width constraint, the layoutIfNeeded() will catch it and
    // do a full layout.
    if (outOfFlowBox.NeedsOutOfFlowMovementLayoutOnly() && outOfFlowBox.TryLayoutDoingOutOfFlowMovementOnly())
    {
      // FIXME: Capture the scroll snapshots of anchor-positioned boxes, once there's anchor positioning.
      outOfFlowBox.ClearNeedsLayout();
    }

    // If we are paginated or in a line grid, compute a vertical position for our object now.
    // If it's wrong we'll lay out again.
    LayoutUnit oldLogicalTop;
    auto *layoutState = View().FrameView().LayoutContext().LayoutState();
    bool needsBlockDirectionLocationSetBeforeLayout =
      outOfFlowBox.NeedsLayout() && layoutState && layoutState->NeedsBlockDirectionLocationSetBeforeLayout();
    if (needsBlockDirectionLocationSetBeforeLayout)
    {
      if (IsHorizontalWritingMode() == outOfFlowBox.IsHorizontalWritingMode())
      {
        outOfFlowBox.UpdateLogicalHeight();
      }
      else
      {
        outOfFlowBox.UpdateLogicalWidth();
      }
      oldLogicalTop = LogicalTopForChild(outOfFlowBox);
    }

    outOfFlowBox.LayoutIfNeeded();

    auto *parent = outOfFlowBox.Parent();
    bool layoutChanged = false;
    if (auto *flexibleBox = DynamicDowncast<RenderFlexibleBox>(*parent);
        flexibleBox && flexibleBox->SetStaticPositionForPositionedLayout(outOfFlowBox))
    {
      // The static position of an abspos child of a flexbox depends on its size
      // (for example, they can be centered). So we may have to reposition the
      // item after layout.
      // FIXME: We could probably avoid a layout here and just reposition?
      layoutChanged = true;
    }

    // Lay out again if our estimate was wrong.
    if (layoutChanged
        || (needsBlockDirectionLocationSetBeforeLayout && LogicalTopForChild(outOfFlowBox) != oldLogicalTop))
    {
      outOfFlowBox.SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
      outOfFlowBox.LayoutIfNeeded();
    }

    // FIXME: Relayout the box when its fragment range changed, and adjust a size-contained box for
    // pagination, once there are fragmented flows and pagination.
  }

  void RenderBlock::LayoutOutOfFlowBoxes(RelayoutChildren relayoutChildren,
                                         bool fixedPositionObjectsOnly) noexcept
  {
    auto *outOfFlowDescendants = OutOfFlowBoxes();
    if (!outOfFlowDescendants)
    {
      return;
    }

    // Do not cache outOfFlowDescendants->end() in a local variable, since |outOfFlowDescendants| can be
    // mutated as it is walked. We always need to fetch the new end() value dynamically.
    for (auto it = outOfFlowDescendants->begin(); it != outOfFlowDescendants->end(); ++it)
    {
      LayoutOutOfFlowBox(**it, relayoutChildren, fixedPositionObjectsOnly);
    }
  }

  void RenderBlock::MarkOutOfFlowBoxesForLayout() noexcept
  {
    auto *outOfFlowDescendants = OutOfFlowBoxes();
    if (!outOfFlowDescendants)
    {
      return;
    }

    for (auto *descendant : *outOfFlowDescendants)
    {
      descendant->SetChildNeedsLayout();
    }
  }

  bool RenderBlock::HasLineIfEmpty() const noexcept
  {
    // FIXME: A root editable element has a line even when empty, once there's editing.
    return false;
  }

  Maybe<LayoutUnit> RenderBlock::FirstLineBaseline() const noexcept
  {
    if (ShouldApplyLayoutContainment())
    {
      return null;
    }

    if (IsWritingModeRoot() && !IsFlexItem())
    {
      return null;
    }

    auto firstInFlowBaseline = [&]() -> Maybe<LayoutUnit>
    {
      for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
      {
        // FIXME: Skip a legend placed in the border, once there are fieldsets.
        if (auto baseline = child->FirstLineBaseline())
        {
          // FIXME: Keep the subpixel logical top when subpixel inline layout is enabled.
          return LayoutUnit(child->LogicalTop().ToInt()) + *baseline;
        }
      }
      return null;
    };
    return firstInFlowBaseline();
  }

  Maybe<LayoutUnit> RenderBlock::LastLineBaseline() const noexcept
  {
    if (ShouldApplyLayoutContainment())
    {
      return null;
    }

    if (IsWritingModeRoot() && !IsFlexItem())
    {
      return null;
    }

    auto lastInFlowBaseline = [&]() -> Maybe<LayoutUnit>
    {
      for (auto *child = LastInFlowChildBox(); child; child = child->PreviousInFlowSiblingBox())
      {
        // FIXME: Skip a legend placed in the border, once there are fieldsets.
        if (auto baseline = child->LastLineBaseline())
        {
          // FIXME: Keep the subpixel logical top when subpixel inline layout is enabled.
          return LayoutUnit(child->LogicalTop().ToInt()) + *baseline;
        }
      }
      return null;
    };
    return lastInFlowBaseline();
  }

  void RenderBlock::SetTrimmedMarginForChild(RenderBox &child, style::MarginTrimSide side) noexcept
  {
    switch (side)
    {
      case style::MarginTrimSide::BlockStart:  SetMarginBeforeForChild(child, 0_lu); break;
      case style::MarginTrimSide::BlockEnd:    SetMarginAfterForChild(child, 0_lu); break;
      case style::MarginTrimSide::InlineStart: SetMarginStartForChild(child, 0_lu); break;
      case style::MarginTrimSide::InlineEnd:   SetMarginEndForChild(child, 0_lu); break;
    }

    child.MarkMarginAsTrimmed(side);
  }

  LayoutUnit RenderBlock::CollapsedMarginBeforeForChild(const RenderBox &child) const noexcept
  {
    // If the child has the same directionality as we do, then we can just return its
    // collapsed margin.
    if (!child.IsWritingModeRoot())
    {
      return child.CollapsedMarginBefore();
    }

    // The child has a different directionality.  If the child is parallel, then it's just
    // flipped relative to us.  We can use the collapsed margin for the opposite edge.
    if (child.IsHorizontalWritingMode() == IsHorizontalWritingMode())
    {
      return child.CollapsedMarginAfter();
    }

    // The child is perpendicular to us, which means its margins don't collapse but are on the
    // "logical left/right" sides of the child box.  We can just return the raw margin in this case.
    return MarginBeforeForChild(child);
  }

  LayoutUnit RenderBlock::CollapsedMarginAfterForChild(const RenderBox &child) const noexcept
  {
    // If the child has the same directionality as we do, then we can just return its
    // collapsed margin.
    if (!child.IsWritingModeRoot())
    {
      return child.CollapsedMarginAfter();
    }

    // The child has a different directionality.  If the child is parallel, then it's just
    // flipped relative to us.  We can use the collapsed margin for the opposite edge.
    if (child.IsHorizontalWritingMode() == IsHorizontalWritingMode())
    {
      return child.CollapsedMarginBefore();
    }

    // The child is perpendicular to us, which means its margins don't collapse but are on the
    // "logical left/right" side of the child box.  We can just return the raw margin in this case.
    return MarginAfterForChild(child);
  }

  bool RenderBlock::HasMarginBeforeQuirk(const RenderBox &child) const noexcept
  {
    // If the child has the same directionality as we do, then we can just return its
    // margin quirk.
    if (!child.IsWritingModeRoot())
    {
      auto *childBlock = DynamicDowncast<RenderBlock>(child);
      return childBlock ? childBlock->HasMarginBeforeQuirk() : child.Style().MarginBefore().HasQuirk();
    }

    // The child has a different directionality. If the child is parallel, then it's just
    // flipped relative to us. We can use the opposite edge.
    if (child.IsHorizontalWritingMode() == IsHorizontalWritingMode())
    {
      auto *childBlock = DynamicDowncast<RenderBlock>(child);
      return childBlock ? childBlock->HasMarginAfterQuirk() : child.Style().MarginAfter().HasQuirk();
    }

    // The child is perpendicular to us and box sides are never quirky in html.css, and we don't really care
    // about whether or not authors specified quirky ems, since they're an implementation detail.
    return false;
  }

  bool RenderBlock::HasMarginAfterQuirk(const RenderBox &child) const noexcept
  {
    // If the child has the same directionality as we do, then we can just return its
    // margin quirk.
    if (!child.IsWritingModeRoot())
    {
      auto *childBlock = DynamicDowncast<RenderBlock>(child);
      return childBlock ? childBlock->HasMarginAfterQuirk() : child.Style().MarginAfter().HasQuirk();
    }

    // The child has a different directionality. If the child is parallel, then it's just
    // flipped relative to us. We can use the opposite edge.
    if (child.IsHorizontalWritingMode() == IsHorizontalWritingMode())
    {
      auto *childBlock = DynamicDowncast<RenderBlock>(child);
      return childBlock ? childBlock->HasMarginBeforeQuirk() : child.Style().MarginBefore().HasQuirk();
    }

    // The child is perpendicular to us and box sides are never quirky in html.css, and we don't really care
    // about whether or not authors specified quirky ems, since they're an implementation detail.
    return false;
  }

  void RenderBlock::UpdateDescendantTransformsAfterLayout() noexcept
  {
    // FIXME: Update the transforms of the boxes whose transform depends on this container's size
    // (takeBoxesNeedingTransformUpdateAfterContainerLayout), once there are motion paths.
  }

  void RenderBlock::LayoutExcludedChildren(RelayoutChildren) noexcept
  {
    // FIXME: Lay out a fieldset's legend inside its border, once there are fieldsets.
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

    LayoutUnit childMinWidth;
    LayoutUnit childMaxWidth;
    if (ComputePreferredWidthsForExcludedChildren(childMinWidth, childMaxWidth))
    {
      minLogicalWidth = std::max(childMinWidth, minLogicalWidth);
      maxLogicalWidth = std::max(childMaxWidth, maxLogicalWidth);
    }

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

  namespace
  {
    KRYS_NODISCARD inline bool IsDelayingUpdateScrollInfoAfterLayout(const RenderBlock &renderer) noexcept
    {
      auto *transaction =
        renderer.View().FrameView().LayoutContext().UpdateScrollInfoAfterLayoutTransactionIfExists();
      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=97937
      // Workaround for now. We cannot delay the scroll info for overflow
      // for items with opposite writing directions, as the contents needs
      // to overflow in that direction
      return transaction && transaction->NestedCount && !renderer.GetWritingMode().IsBlockFlipped();
    }
  }

  void RenderBlock::BeginUpdateScrollInfoAfterLayoutTransaction() noexcept
  {
    ++View().FrameView().LayoutContext().GetUpdateScrollInfoAfterLayoutTransaction().NestedCount;
  }

  void RenderBlock::EndAndCommitUpdateScrollInfoAfterLayoutTransaction() noexcept
  {
    auto *transaction = View().FrameView().LayoutContext().UpdateScrollInfoAfterLayoutTransactionIfExists();
    krys_debug_assert(transaction);
    if (--transaction->NestedCount)
    {
      return;
    }

    // Calling RenderLayer::updateScrollInfoAfterLayout() may cause its associated block to layout again and
    // updates its scroll info (i.e. call RenderBlock::updateScrollInfoAfterLayout()). We decrement the
    // nestedCount first so that all subsequent calls to RenderBlock::updateScrollInfoAfterLayout() are
    // dispatched immediately. That is, to ensure that such subsequent calls aren't added to |transaction|
    // while we are processing it.
    auto blocksToUpdate = List<RenderBlock *>(transaction->Blocks.begin(), transaction->Blocks.end());
    transaction->Blocks.clear();

    for (auto *block : blocksToUpdate)
    {
      // FIXME: Clear the layout overflow of a lightweight control clip, once there are form controls.
      if (block->HasNonVisibleOverflow())
      {
        block->Layer()->UpdateScrollInfoAfterLayout();
      }
    }
  }

  void RenderBlock::RemoveFromUpdateScrollInfoAfterLayoutTransaction() noexcept
  {
    if (auto *transaction =
          View().FrameView().LayoutContext().UpdateScrollInfoAfterLayoutTransactionIfExists())
    {
      transaction->Blocks.remove(this);
    }
  }

  void RenderBlock::UpdateScrollInfoAfterLayout() noexcept
  {
    auto hasNonVisibleOverflow = HasNonVisibleOverflow();

    if (IsDelayingUpdateScrollInfoAfterLayout(*this))
    {
      // FIXME: Also delay a lightweight control clip's update, once there are form controls.
      if (hasNonVisibleOverflow)
      {
        View().FrameView().LayoutContext().UpdateScrollInfoAfterLayoutTransactionIfExists()->Blocks.add(this);
        return;
      }
    }

    if (hasNonVisibleOverflow && Layer())
    {
      Layer()->UpdateScrollInfoAfterLayout();
    }
  }

  LayoutUnit RenderBlock::MarginIntrinsicLogicalWidthForChild(RenderBox &child) const noexcept
  {
    // A margin has three types: fixed, percentage, and auto (variable).
    // Auto and percentage margins become 0 when computing min/max width.
    // Fixed margins can be added in as is.
    auto &marginLeft = child.Style().MarginStart(GetWritingMode());
    auto &marginRight = child.Style().MarginEnd(GetWritingMode());
    const auto &zoomFactor = child.Style().UsedZoomForLength();
    LayoutUnit margin;
    if (auto fixedMarginLeft = marginLeft.TryFixed();
        fixedMarginLeft && !ShouldTrimChildMargin(style::MarginTrimSide::InlineStart, child))
    {
      margin += fixedMarginLeft->ResolveZoom(zoomFactor);
    }
    if (auto fixedMarginRight = marginRight.TryFixed();
        fixedMarginRight && !ShouldTrimChildMargin(style::MarginTrimSide::InlineEnd, child))
    {
      margin += fixedMarginRight->ResolveZoom(zoomFactor);
    }
    return margin;
  }

  bool RenderBlock::ComputePreferredWidthsForExcludedChildren(LayoutUnit &, LayoutUnit &) const noexcept
  {
    // FIXME: The rendered legend of a fieldset, once there are fieldsets.
    return false;
  }

  LayoutUnit RenderBlock::LayoutOverflowLogicalBottom(const RenderBlock &renderer) noexcept
  {
    krys_debug_assert(Is<RenderGrid>(renderer) || Is<RenderFlexibleBox>(renderer));
    auto maxChildLogicalBottom = LayoutUnit {};
    for (auto &child : ChildrenOfType<RenderBox>(renderer))
    {
      if (child.IsOutOfFlowPositioned())
      {
        continue;
      }
      auto childLogicalBottom = renderer.LogicalTopForChild(child) + renderer.LogicalHeightForChild(child)
                                + renderer.MarginAfterForChild(child);
      maxChildLogicalBottom = std::max(maxChildLogicalBottom, childLogicalBottom);
    }
    return std::max(renderer.ClientLogicalBottom(), maxChildLogicalBottom + renderer.PaddingAfter());
  }
}
