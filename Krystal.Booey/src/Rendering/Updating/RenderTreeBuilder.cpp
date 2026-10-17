#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFirstLetter.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderInline.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderList.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderRuby.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderTable.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  RenderTreeBuilder *RenderTreeBuilder::s_current = nullptr;

  namespace
  {
    enum class IsRemoval : bool
    {
      No,
      Yes
    };
    void InvalidateLineLayout(RenderObject &renderer, IsRemoval isRemoval) noexcept
    {
      auto *container = layout_integration::LineLayout::BlockContainer(renderer);
      if (!container)
      {
        return;
      }

      // FIXME: Marquees that ran preferred width computation without layout, once there are marquees.
      if (isRemoval == IsRemoval::Yes && !renderer.EverHadLayout())
      {
        // Certain mutations can make renderer to be removed before running layout. In such cases we don't
        // have to try to run invalidation only remove it from layout tree.
        if (auto *inlineLayout = container->InlineLayout())
        {
          (void)inlineLayout->RemovedFromTree(*renderer.Parent(), renderer);
        }
        return;
      }

      auto shouldInvalidateLineLayout = [&](layout_integration::LineLayout &inlineLayout)
      {
        if (layout_integration::LineLayout::ShouldInvalidateLineLayoutAfterTreeMutation(
              *container, renderer, inlineLayout, isRemoval == IsRemoval::Yes))
        {
          return true;
        }
        if (isRemoval == IsRemoval::Yes)
        {
          return !inlineLayout.RemovedFromTree(*renderer.Parent(), renderer);
        }
        return !inlineLayout.InsertedIntoTree(*renderer.Parent(), renderer);
      };
      if (auto *inlineLayout = container->InlineLayout();
          inlineLayout && shouldInvalidateLineLayout(*inlineLayout))
      {
        container->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InsertionOrRemoval);
      }
    }

    void GetInlineRun(RenderObject *start, RenderObject *boundary, RenderObject *&inlineRunStart,
                      RenderObject *&inlineRunEnd) noexcept
    {
      // Beginning at |start| we find the largest contiguous run of inlines that
      // we can. We denote the run with start and end points, |inlineRunStart|
      // and |inlineRunEnd|. Note that these two values may be the same if
      // we encounter only one inline.
      //
      // We skip any non-inlines we encounter as long as we haven't found any
      // inlines yet.
      //
      // |boundary| indicates a non-inclusive boundary point. Regardless of whether |boundary|
      // is inline or not, we will not include it in a run with inlines before it. It's as though we
      // encountered a non-inline.

      // Start by skipping as many non-inlines as we can.
      auto *curr = start;
      bool sawInline = false;
      do
      {
        while (curr && !(curr->IsInline() || curr->IsFloatingOrOutOfFlowPositioned()))
        {
          curr = curr->NextSibling();
        }

        inlineRunStart = inlineRunEnd = curr;

        if (!curr)
        {
          return; // No more inline children to be found.
        }

        sawInline = curr->IsInline();

        curr = curr->NextSibling();
        while (curr && (curr->IsInline() || curr->IsFloatingOrOutOfFlowPositioned()) && (curr != boundary))
        {
          inlineRunEnd = curr;
          if (curr->IsInline())
          {
            sawInline = true;
          }
          curr = curr->NextSibling();
        }
      } while (!sawInline);
    }
  }

  RenderTreeBuilder::RenderTreeBuilder(RenderView &view) noexcept
      : _view(view), _tableBuilder(CreateUnique<Table>(*this)), _blockBuilder(CreateUnique<Block>(*this)),
        _blockFlowBuilder(CreateUnique<BlockFlow>(*this)), _inlineBuilder(CreateUnique<Inline>(*this)),
        _firstLetterBuilder(CreateUnique<FirstLetter>(*this)), _listBuilder(CreateUnique<List>(*this)),
        _rubyBuilder(CreateUnique<Ruby>(*this))
  {
    krys_assert(!s_current || &_view != &s_current->_view);
    _previous = s_current;
    s_current = this;
  }

  RenderTreeBuilder::~RenderTreeBuilder() noexcept
  {
    s_current = _previous;
  }

  bool RenderTreeBuilder::IsRebuildRootForChildren(const RenderElement &renderer) noexcept
  {
    // If a (non-anonymous) child is added or removed to a rebuild root then we'll rebuild the full
    // subtree instead of trying to maintain the correct anonymous box structure on per-child basis.
    // This can greatly simplify the code needed to maintain the correct structure.

    auto display = renderer.Style().Display();
    if (display == DisplayType::Ruby || display == DisplayType::RubyBlock)
    {
      return true;
    }

    return false;
  }

  void RenderTreeBuilder::Destroy(RenderObject &renderer,
                                  CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    krys_debug_assert(renderer.Parent());
    krys_debug_assert(!renderer.BeingDestroyed());

    auto notifyDescendantRenderersBeforeSubtreeTearDownIfApplicable = [&]
    {
      if (renderer.RenderTreeBeingDestroyed())
      {
        return;
      }
      auto *rendererToDelete = DynamicDowncast<RenderElement>(renderer);
      if (!rendererToDelete || !rendererToDelete->FirstChild())
      {
        return;
      }

      for (auto &descendant : DescendantsOfType<RenderObject>(*rendererToDelete))
      {
        descendant.WillBeRemovedFromTree();
      }
    };
    notifyDescendantRenderersBeforeSubtreeTearDownIfApplicable();

    auto toDestroy = Detach(*renderer.Parent(), renderer, WillBeDestroyed::Yes, canCollapseAnonymousBlock);

    if (auto *textFragment = DynamicDowncast<RenderTextFragment>(renderer))
    {
      FirstLetterBuilder().CleanupOnDestroy(*textFragment);
    }

    // FIXME: The continuation clean-up, once there are continuations.

    auto tearDownSubTreeIfApplicable = [&]
    {
      auto *rendererToDelete = DynamicDowncast<RenderElement>(toDestroy.get());
      if (!rendererToDelete)
      {
        return;
      }

      SetForScope subtreeTearDownType(_tearDownType, TearDownType::SubtreeWithRootAlreadyDetached);
      while (rendererToDelete->FirstChild())
      {
        auto &firstChild = *rendererToDelete->FirstChild();
        if (auto *node = firstChild.GetNode())
        {
          node->SetRenderer(nullptr);
        }
        Destroy(firstChild);
      }
    };
    // FIXME: webkit.org/b/182909.
    tearDownSubTreeIfApplicable();

    // FIXME: Renderers detached during layout wait in the layout context's detached renderer list, once
    // there's layout. Until then `toDestroy` is destroyed here.
  }

  void RenderTreeBuilder::Attach(RenderElement &parent, RenderPtr<RenderObject> child,
                                 RenderObject *beforeChild) noexcept
  {
    // FIXME: Report visually non-empty content to the view, once there's painting.
    krys_debug_assert(!parent.BeingDestroyed());
    krys_debug_assert(child);
    AttachInternal(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::AttachInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                                         RenderObject *beforeChild) noexcept
  {
    auto insertRecursiveIfNeeded = [&](RenderElement &parentCandidate)
    {
      if (&parent == &parentCandidate)
      {
        // FIXME: Parents inside multicols can't call internal attach directly, once there are any.
        AttachToRenderElement(parent, krys::move(child), beforeChild);
        return;
      }
      AttachInternal(parentCandidate, krys::move(child), beforeChild);
    };

    krys_debug_assert(&parent.View() == &_view);

    if (auto *beforeChildText = DynamicDowncast<RenderText>(beforeChild))
    {
      if (auto *wrapperInline = beforeChildText->InlineWrapperForDisplayContents())
      {
        beforeChild = wrapperInline;
      }
    }
    // FIXME: A beforeChild that's a column spanner, once there are multi-column flows.

    // FIXME: SVG text, buttons, menu lists, SVG containers and roots and MathML, once there are their
    // renderers and builders.

    if (parent.Style().Display() == DisplayType::Ruby || parent.Style().Display() == DisplayType::RubyBlock)
    {
      auto &parentCandidate =
        RubyBuilder().FindOrCreateParentForStyleBasedRubyChild(parent, *child, beforeChild);
      if (&parentCandidate == &parent)
      {
        RubyBuilder().AttachForStyleBasedRuby(parentCandidate, krys::move(child), beforeChild);
        return;
      }
      insertRecursiveIfNeeded(parentCandidate);
      return;
    }

    if (auto *parentBlockFlow = DynamicDowncast<RenderBlockFlow>(parent))
    {
      BlockFlowBuilder().Attach(*parentBlockFlow, krys::move(child), beforeChild);
      return;
    }

    if (auto *row = DynamicDowncast<RenderTableRow>(parent))
    {
      auto &parentCandidate = TableBuilder().FindOrCreateParentForChild(*row, *child, beforeChild);
      if (&parentCandidate == &parent)
      {
        TableBuilder().Attach(*row, krys::move(child), beforeChild);
        return;
      }
      insertRecursiveIfNeeded(parentCandidate);
      return;
    }

    if (auto *tableSection = DynamicDowncast<RenderTableSection>(parent))
    {
      auto &parentCandidate = TableBuilder().FindOrCreateParentForChild(*tableSection, *child, beforeChild);
      if (&parent == &parentCandidate)
      {
        TableBuilder().Attach(*tableSection, krys::move(child), beforeChild);
        return;
      }
      insertRecursiveIfNeeded(parentCandidate);
      return;
    }

    if (auto *table = DynamicDowncast<RenderTable>(parent))
    {
      auto &parentCandidate = TableBuilder().FindOrCreateParentForChild(*table, *child, beforeChild);
      if (&parentCandidate == &parent)
      {
        TableBuilder().Attach(*table, krys::move(child), beforeChild);
        return;
      }
      insertRecursiveIfNeeded(parentCandidate);
      return;
    }

    if (auto *gridParent = DynamicDowncast<RenderGrid>(parent))
    {
      AttachToRenderGrid(*gridParent, krys::move(child), beforeChild);
      return;
    }

    if (auto *parentBlock = DynamicDowncast<RenderBlock>(parent))
    {
      BlockBuilder().Attach(*parentBlock, krys::move(child), beforeChild);
      return;
    }

    if (auto *inlineParent = DynamicDowncast<RenderInline>(parent))
    {
      InlineBuilder().Attach(*inlineParent, krys::move(child), beforeChild);
      return;
    }

    AttachToRenderElement(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::AttachIgnoringContinuation(RenderElement &parent, RenderPtr<RenderObject> child,
                                                     RenderObject *beforeChild) noexcept
  {
    if (auto *inlineParent = DynamicDowncast<RenderInline>(parent))
    {
      InlineBuilder().AttachIgnoringContinuation(*inlineParent, krys::move(child), beforeChild);
      return;
    }

    if (auto *parentBlock = DynamicDowncast<RenderBlock>(parent))
    {
      BlockBuilder().AttachIgnoringContinuation(*parentBlock, krys::move(child), beforeChild);
      return;
    }

    AttachInternal(parent, krys::move(child), beforeChild);
  }

  RenderPtr<RenderObject>
    RenderTreeBuilder::Detach(RenderElement &parent, RenderObject &child, WillBeDestroyed willBeDestroyed,
                              CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    krys_debug_assert(!parent.BeingDestroyed());
    krys_debug_assert(!child.BeingDestroyed());

    // FIXME: SVG text, menu lists, buttons and SVG inlines, containers and roots, once there are their
    // renderers and builders.

    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(parent))
    {
      return BlockBuilder().Detach(*blockFlow, child, willBeDestroyed, canCollapseAnonymousBlock);
    }

    if (auto *grid = DynamicDowncast<RenderGrid>(parent))
    {
      return DetachFromRenderGrid(*grid, child, willBeDestroyed);
    }

    if (auto *block = DynamicDowncast<RenderBlock>(parent))
    {
      return BlockBuilder().Detach(*block, child, willBeDestroyed, canCollapseAnonymousBlock);
    }

    return DetachFromRenderElement(parent, child, willBeDestroyed);
  }

  void RenderTreeBuilder::AttachToRenderElement(RenderElement &parent, RenderPtr<RenderObject> child,
                                                RenderObject *beforeChild) noexcept
  {
    if (TableBuilder().ChildRequiresTable(parent, *child))
    {
      RenderTable *table = nullptr;
      auto *afterChild =
        DynamicDowncast<RenderTable>(beforeChild ? beforeChild->PreviousSibling() : parent.LastChild());
      if (afterChild && afterChild->IsAnonymous() && !afterChild->IsBeforeContent())
      {
        table = afterChild;
      }
      else
      {
        auto newTable = Table::CreateAnonymousTableWithStyle(parent.GetDocument(), parent.Style());
        table = newTable.get();
        Attach(parent, krys::move(newTable), beforeChild);
      }

      Attach(*table, krys::move(child));
      return;
    }
    auto &newChild = *child.get();
    AttachToRenderElementInternal(parent, krys::move(child), beforeChild);
    parent.DidAttachChild(newChild, beforeChild);
  }

  void RenderTreeBuilder::AttachToRenderElementInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                                                        RenderObject *beforeChild) noexcept
  {
    // FIXME: Assert layout isn't mutating the render tree, once there's layout.
    krys_debug_assert(parent.CanHaveChildren() || parent.CanHaveGeneratedChildren());
    krys_debug_assert(!child->Parent());
    krys_debug_assert(
      !parent.IsRenderBlockFlow()
      || (!child->IsRenderTableSection() && !child->IsRenderTableRow() && !child->IsRenderTableCell()));

    while (beforeChild && beforeChild->Parent() && beforeChild->Parent() != &parent)
    {
      beforeChild = beforeChild->Parent();
    }

    if (beforeChild && !beforeChild->Parent())
    {
      // Should never let the RenderView be beforeChild (as who is the parent then)
      krys_debug_assert(false);
      beforeChild = nullptr;
    }

    krys_debug_assert(!beforeChild || beforeChild->Parent() == &parent);
    krys_debug_assert(!Is<RenderText>(beforeChild)
                      || !Downcast<RenderText>(*beforeChild).InlineWrapperForDisplayContents());

    // Take the ownership.
    auto *newChild = parent.AttachRendererInternal(krys::move(child), beforeChild);
    if (parent.RenderTreeBeingDestroyed())
    {
      krys_debug_assert(false);
      return;
    }

    newChild->InsertedIntoTree();
    InvalidateLineLayout(*newChild, IsRemoval::No);

    if (_internalMovesType == IsInternalMove::No)
    {
      // FIXME: The fragmented flow state and multi-column descendants, once there are fragmented flows.
      if (auto *listItemRenderer = DynamicDowncast<RenderListItem>(*newChild))
      {
        listItemRenderer->UpdateListMarkerNumbers();
      }
    }

    newChild->SetNeedsLayoutAndPreferredWidthsUpdate();
    auto isOutOfFlowBox = newChild->Style().HasOutOfFlowPosition();
    if (!isOutOfFlowBox)
    {
      parent.SetNeedsPreferredWidthsUpdate();
    }

    if (!parent.NormalChildNeedsLayout())
    {
      if (isOutOfFlowBox)
      {
        auto isEligibleForStaticPositionLayoutOnly = [&]
        {
          // setNeedsLayoutAndPreferredWidthsUpdate above already takes care of propagating dirty bits on the
          // ancestor chain, but in order to compute static position for out of flow boxes, the parent has to
          // run normal flow layout as well (as opposed to simplified)
          if (newChild->ContainingBlock() != &parent)
          {
            return false;
          }
          // FIXME: Video, once there's video.
          // Since we can't actually run static position only layout for a block container
          // (RenderBlockFlow::layoutBlock() does not have such fine grained layout flow) floats get rebuilt
          // which assumes (see intruding floats) parent block containers do the same.
          if (auto *renderBlock = DynamicDowncast<RenderBlock>(parent))
          {
            return !renderBlock->ContainsFloats();
          }
          return true;
        };
        if (isEligibleForStaticPositionLayoutOnly())
        {
          // FIXME: Introduce a dirty bit to bridge the gap between parent and containing block which would
          // not trigger layout but a simple traversal all the way to the direct parent and also expand it
          // non-direct parent cases.
          parent.SetOutOfFlowChildNeedsStaticPositionLayout();
        }
        else
        {
          parent.SetChildNeedsLayout();
        }
      }
      else
      {
        parent.SetChildNeedsLayout();
      }
    }

    // FIXME: The accessibility cache and outline-auto ancestors, once there are any.
  }

  void RenderTreeBuilder::Move(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject &child,
                               RenderObject *beforeChild,
                               NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    // We assume that callers have cleared their positioned objects list for child moves so the
    // positioned renderer maps don't become stale. It would be too slow to do the map lookup on each call.
    krys_debug_assert(&from == child.Parent());
    krys_debug_assert(!beforeChild || &to == beforeChild->Parent());
    // FIXME: Remove percent height descendants, once there's layout.
    if (normalizeAfterInsertion == NormalizeAfterInsertion::Yes
        && (to.IsRenderBlock() || to.IsRenderInline()))
    {
      // Takes care of adding the new child correctly if toBlock and fromBlock
      // have different kind of children (block vs inline).
      auto childToMove = DetachFromRenderElement(from, child, WillBeDestroyed::No);
      Attach(to, krys::move(childToMove), beforeChild);
    }
    else
    {
      SetForScope internalMoveScope(_internalMovesType, IsInternalMove::Yes);
      auto childToMove = DetachFromRenderElement(from, child, WillBeDestroyed::No);
      AttachToRenderElementInternal(to, krys::move(childToMove), beforeChild);
    }

    auto findBFCRootAndDestroyInlineTree = [&]
    {
      for (RenderElement *containingBlock = &from; containingBlock;
           containingBlock = containingBlock->ContainingBlock())
      {
        containingBlock->SetNeedsLayout();
        if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*containingBlock))
        {
          blockFlow->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);
          break;
        }
      }
    };
    // When moving a subtree out of a BFC we need to make sure that the line boxes generated for the inline
    // tree are not accessible anymore from the renderers. Let's find the BFC root and nuke the inline tree
    // (At some point we are going to destroy the subtree instead of moving these renderers around.)
    if (Is<RenderInline>(child))
    {
      findBFCRootAndDestroyInlineTree();
    }
  }

  void RenderTreeBuilder::Move(RenderBoxModelObject &from, RenderBoxModelObject &to, RenderObject &child,
                               NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    Move(from, to, child, nullptr, normalizeAfterInsertion);
  }

  void RenderTreeBuilder::MoveAllChildren(RenderBoxModelObject &from, RenderBoxModelObject &to,
                                          NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    MoveAllChildren(from, to, nullptr, normalizeAfterInsertion);
  }

  void RenderTreeBuilder::MoveAllChildren(RenderBoxModelObject &from, RenderBoxModelObject &to,
                                          RenderObject *beforeChild,
                                          NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    MoveChildren(from, to, from.FirstChild(), nullptr, beforeChild, normalizeAfterInsertion);
  }

  void RenderTreeBuilder::MoveChildren(RenderBoxModelObject &from, RenderBoxModelObject &to,
                                       RenderObject *startChild, RenderObject *endChild,
                                       NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    MoveChildren(from, to, startChild, endChild, nullptr, normalizeAfterInsertion);
  }

  void RenderTreeBuilder::MoveChildren(RenderBoxModelObject &from, RenderBoxModelObject &to,
                                       RenderObject *startChild, RenderObject *endChild,
                                       RenderObject *beforeChild,
                                       NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    // This condition is rarely hit since this function is usually called on
    // anonymous blocks which can no longer carry positioned objects (see r120761)
    // or when fullRemoveInsert is false.
    if (normalizeAfterInsertion == NormalizeAfterInsertion::Yes)
    {
      if (auto *blockFlow = DynamicDowncast<RenderBlock>(from))
      {
        blockFlow->RemoveOutOfFlowBoxes(nullptr);
        RenderBlock::RemovePercentHeightDescendant(*blockFlow);
        RemoveFloatingObjects(*blockFlow);
      }
    }

    krys_debug_assert(!beforeChild || &to == beforeChild->Parent());
    for (auto *child = startChild; child && child != endChild;)
    {
      // Save our next sibling as moveChildTo will clear it.
      auto *nextSibling = child->NextSibling();

      // FIXME: This logic here fails to detect the first letter in certain cases
      // and skips a valid sibling renderer (see webkit.org/b/163737).
      // Check to make sure we're not saving the firstLetter as the nextSibling.
      // When the |child| object will be moved, its firstLetter will be recreated,
      // so saving it now in nextSibling would leave us with a stale object.
      if (Is<RenderTextFragment>(*child) && Is<RenderText>(nextSibling))
      {
        if (auto *block = Downcast<RenderTextFragment>(*child).BlockForAccompanyingFirstLetter())
        {
          auto [firstLetter, firstLetterContainer] = block->FirstLetterAndContainer(child);
          // This is the first letter, skip it.
          if (firstLetter == nextSibling)
          {
            nextSibling = nextSibling->NextSibling();
          }
        }
      }

      Move(from, to, *child, beforeChild, normalizeAfterInsertion);
      child = nextSibling;
    }
  }

  void RenderTreeBuilder::MoveAllChildrenIncludingFloats(
    RenderBlock &from, RenderBlock &to, NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(from))
    {
      BlockFlowBuilder().MoveAllChildrenIncludingFloats(*blockFlow, to, normalizeAfterInsertion);
      return;
    }
    MoveAllChildren(from, to, normalizeAfterInsertion);
  }

  void RenderTreeBuilder::NormalizeTreeAfterStyleChange(RenderElement &renderer,
                                                        RenderStyle &oldStyle) noexcept
  {
    if (!renderer.Parent())
    {
      return;
    }

    bool wasFloating = oldStyle.IsFloating();
    bool wasOutOfFlowPositioned = oldStyle.HasOutOfFlowPosition();
    bool isFloating = renderer.Style().IsFloating();
    bool isOutOfFlowPositioned = renderer.Style().HasOutOfFlowPosition();
    bool startsAffectingParent = false;
    bool noLongerAffectsParent = false;
    // FIXME: Fragmented flow state changes (handleFragmentedFlowStateChange), once there are multi-column
    // flows.

    auto &parent = *renderer.Parent();
    if (Is<RenderBlock>(parent))
    {
      noLongerAffectsParent =
        (!wasFloating && isFloating) || (!wasOutOfFlowPositioned && isOutOfFlowPositioned);
    }

    if (Is<RenderBlockFlow>(parent) || Is<RenderInline>(parent))
    {
      startsAffectingParent =
        (wasFloating || wasOutOfFlowPositioned) && !isFloating && !isOutOfFlowPositioned;
      krys_debug_assert(!startsAffectingParent || !noLongerAffectsParent);
    }

    if (startsAffectingParent)
    {
      // We have gone from not affecting the inline status of the parent flow to suddenly
      // having an impact. See if there is a mismatch between the parent flow's
      // childrenInline() state and our state.
      if (renderer.IsInline() != renderer.Parent()->ChildrenInline())
      {
        ChildFlowStateChangesAndAffectsParentBlock(renderer);
      }
      // WARNING: original parent might be deleted at this point.
      return;
    }

    if (noLongerAffectsParent)
    {
      ChildFlowStateChangesAndNoLongerAffectsParentBlock(renderer);

      if (isFloating)
      {
        if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderer))
        {
          auto clearDescendantFloats = [&]
          {
            // These descendent floats can not intrude other, sibling block containers anymore.
            for (auto &descendant : DescendantsOfType<RenderBox>(*blockFlow))
            {
              if (descendant.IsFloating())
              {
                descendant.RemoveFloatingAndInvalidateForLayout();
              }
            }
          };
          clearDescendantFloats();
          RemoveFloatingObjects(*blockFlow);
          // Fresh floats need to be reparented if they actually belong to the previous anonymous block.
          // It copies the logic of RenderBlock::addChildIgnoringContinuation
          if (auto *previousSibling = DynamicDowncast<RenderBlock>(blockFlow->PreviousSibling());
              previousSibling && previousSibling->IsAnonymousBlock())
          {
            Move(Downcast<RenderBoxModelObject>(parent), *previousSibling, renderer,
                 NormalizeAfterInsertion::No);
          }
        }
      }
    }
  }

  void RenderTreeBuilder::CreateAnonymousWrappersForInlineContent(RenderBlock &parent,
                                                                  RenderObject *insertionPoint) noexcept
  {
    // makeChildrenNonInline takes a block whose children are *all* inline and it
    // makes sure that inline children are coalesced under anonymous
    // blocks. If |insertionPoint| is defined, then it represents the insertion point for
    // the new block child that is causing us to have to wrap all the inlines. This
    // means that we cannot coalesce inlines before |insertionPoint| with inlines following
    // |insertionPoint|, because the new child is going to be inserted in between the inlines,
    // splitting them.
    krys_debug_assert(parent.IsNonReplacedAtomicInlineLevelBox() || !parent.IsInline());
    krys_debug_assert(!insertionPoint || insertionPoint->Parent() == &parent);

    parent.SetChildrenInline(false);

    auto *child = parent.FirstChild();
    if (!child)
    {
      return;
    }

    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(parent))
    {
      blockFlow->InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);
    }

    while (child)
    {
      RenderObject *inlineRunStart = nullptr;
      RenderObject *inlineRunEnd = nullptr;
      GetInlineRun(child, insertionPoint, inlineRunStart, inlineRunEnd);

      if (!inlineRunStart)
      {
        break;
      }

      child = inlineRunEnd->NextSibling();

      auto newBlock = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
      auto &block = *newBlock;
      AttachToRenderElementInternal(parent, krys::move(newBlock), inlineRunStart);
      MoveChildren(parent, block, inlineRunStart, child, NormalizeAfterInsertion::No);
    }
#ifndef NDEBUG
    for (auto *c = parent.FirstChild(); c; c = c->NextSibling())
    {
      krys_debug_assert(!c->IsInline());
    }
#endif
    // FIXME: Repaint, once there's painting.
  }

  RenderObject *RenderTreeBuilder::SplitAnonymousBoxesAroundChild(RenderBoxModelObject &parent,
                                                                  RenderObject &originalBeforeChild) noexcept
  {
    // FIXME: Adjust beforeChild if it is a column spanner that has been moved out of its original position,
    // once there are multi-column flows.
    auto *beforeChild = &originalBeforeChild;
    bool didSplitParentAnonymousBoxes = false;

    while (beforeChild->Parent() != &parent)
    {
      auto &boxToSplit = Downcast<RenderBox>(static_cast<RenderObject &>(*beforeChild->Parent()));
      if (boxToSplit.FirstChild() != beforeChild && boxToSplit.IsAnonymous())
      {
        didSplitParentAnonymousBoxes = true;

        // We have to split the parent box into two boxes and move children
        // from |beforeChild| to end into the new post box.
        auto newPostBox = CreateAnonymousBoxWithSameTypeAndWithStyle(boxToSplit, parent.Style());
        auto &postBox = *newPostBox;
        postBox.SetChildrenInline(boxToSplit.ChildrenInline());
        auto *parentBox = Downcast<RenderBoxModelObject>(static_cast<RenderObject *>(boxToSplit.Parent()));
        // We need to invalidate the |parentBox| before inserting the new node
        // so that the table repainting logic knows the structure is dirty.
        // See for example RenderTableCell:clippedOverflowRectForRepaint.
        MarkBoxForRelayoutAfterSplit(*parentBox);
        AttachToRenderElementInternal(*parentBox, krys::move(newPostBox), boxToSplit.NextSibling());
        MoveChildren(boxToSplit, postBox, beforeChild, nullptr, NormalizeAfterInsertion::Yes);

        MarkBoxForRelayoutAfterSplit(boxToSplit);
        MarkBoxForRelayoutAfterSplit(postBox);

        beforeChild = &postBox;
      }
      else
      {
        beforeChild = &boxToSplit;
      }
    }

    if (didSplitParentAnonymousBoxes)
    {
      MarkBoxForRelayoutAfterSplit(parent);
    }

    krys_debug_assert(beforeChild->Parent() == &parent);
    return beforeChild;
  }

  void RenderTreeBuilder::ChildFlowStateChangesAndAffectsParentBlock(RenderElement &child) noexcept
  {
    if (!child.IsInline())
    {
      auto *parent = child.Parent();
      if (auto *parentBlockRenderer = DynamicDowncast<RenderBlock>(*parent))
      {
        BlockBuilder().ChildBecameNonInline(*parentBlockRenderer, child);
      }
      else if (auto *parentInlineRenderer = DynamicDowncast<RenderInline>(*parent))
      {
        InlineBuilder().ChildBecameNonInline(*parentInlineRenderer, child);
      }
      // WARNING: original parent might be deleted at this point.
      if (auto *newParent = child.Parent(); newParent != parent)
      {
        if (auto *gridRenderer = DynamicDowncast<RenderGrid>(newParent))
        {
          // We need to re-run the grid items placement if it had gained a new item.
          gridRenderer->SetNeedsItemPlacement();
        }
      }
      return;
    }
    // An anonymous block must be made to wrap this inline.
    auto *parent = child.Parent();
    auto newBlock = Block::CreateAnonymousBlockWithStyle(parent->GetDocument(), parent->Style());
    auto &block = *newBlock;
    AttachToRenderElementInternal(*parent, krys::move(newBlock), &child);
    auto thisToMove = DetachFromRenderElement(*parent, child, WillBeDestroyed::No);
    AttachToRenderElementInternal(block, krys::move(thisToMove));
  }

  void RenderTreeBuilder::RemoveAnonymousWrappersForInlineChildrenIfNeeded(RenderElement &parent) noexcept
  {
    auto *blockParent = DynamicDowncast<RenderBlock>(parent);
    if (!blockParent || !blockParent->CanDropAnonymousBlockChild())
    {
      return;
    }

    // We have changed to floated or out-of-flow positioning so maybe all our parent's
    // children can be inline now. Bail if there are any block children left on the line,
    // otherwise we can proceed to stripping solitary anonymous wrappers from the inlines.
    // FIXME: We should also handle split inlines here - we exclude them at the moment by returning
    // if we find a continuation.
    Maybe<bool> shouldAllChildrenBeInline;
    for (auto *current = blockParent->FirstChild(); current; current = current->NextSibling())
    {
      if (current->Style().IsFloating() || current->Style().HasOutOfFlowPosition())
      {
        continue;
      }

      auto *renderBlockChild = DynamicDowncast<RenderBlock>(*current);
      if (!renderBlockChild)
      {
        return;
      }
      if (!renderBlockChild->IsAnonymousBlock())
      {
        return;
      }
      // Anonymous block not in continuation. Check if it holds a set of inline or block children and try not
      // to mix them.
      auto *firstChild = renderBlockChild->FirstChild();
      if (!firstChild)
      {
        continue;
      }
      auto isInlineLevelBox = firstChild->IsInline();
      if (!shouldAllChildrenBeInline)
      {
        shouldAllChildrenBeInline = isInlineLevelBox;
        continue;
      }
      // Mixing inline and block level boxes?
      if (*shouldAllChildrenBeInline != isInlineLevelBox)
      {
        return;
      }
    }

    RenderObject *next = nullptr;
    for (auto *current = blockParent->FirstChild(); current; current = next)
    {
      next = current->NextSibling();
      if (auto *block = DynamicDowncast<RenderBlock>(*current); block && block->IsAnonymousBlock())
      {
        BlockBuilder().DropAnonymousBoxChild(*blockParent, *block);
      }
    }
  }

  void RenderTreeBuilder::ChildFlowStateChangesAndNoLongerAffectsParentBlock(RenderElement &child) noexcept
  {
    krys_debug_assert(child.Parent());
    RemoveAnonymousWrappersForInlineChildrenIfNeeded(*child.Parent());
  }

  void RenderTreeBuilder::DestroyAndCleanUpAnonymousWrappers(RenderObject &rendererToDestroy,
                                                             const RenderElement *subtreeDestroyRoot) noexcept
  {
    SetForScope tearDownType(_tearDownType, !subtreeDestroyRoot || &rendererToDestroy == subtreeDestroyRoot
                                              ? TearDownType::Root
                                              : TearDownType::SubtreeWithRootStillAttached);
    SetForScope tearDownDestroyRoot(
      _subtreeDestroyRoot,
      _tearDownType == TearDownType::SubtreeWithRootStillAttached ? subtreeDestroyRoot : nullptr);

    // If the tree is destroyed, there is no need for a clean-up phase.
    if (rendererToDestroy.RenderTreeBeingDestroyed())
    {
      Destroy(rendererToDestroy);
      return;
    }

    auto isAnonymousAndSafeToDelete = [](const RenderElement &renderer)
    {
      // FIXME: SVG viewport containers, once there are any.
      return renderer.IsAnonymous() && !renderer.IsRenderView() && !renderer.IsRenderFragmentedFlow();
    };

    auto destroyRootIncludingAnonymous = [&]() -> RenderObject &
    {
      auto *destroyRoot = &rendererToDestroy;
      while (!Is<RenderView>(*destroyRoot))
      {
        auto &destroyRootParent = *destroyRoot->Parent();
        if (!isAnonymousAndSafeToDelete(destroyRootParent))
        {
          break;
        }
        bool destroyingOnlyChild =
          destroyRootParent.FirstChild() == destroyRoot && destroyRootParent.LastChild() == destroyRoot;
        if (!destroyingOnlyChild)
        {
          break;
        }
        destroyRoot = &destroyRootParent;
      }
      return *destroyRoot;
    };

    auto *destroyRoot = &destroyRootIncludingAnonymous();

    auto clearFloatsAndOutOfFlowPositionedObjects = [&]
    {
      // Remove floats and out-of-flow positioned objects from their containing block before detaching
      // the renderer from the tree. It includes all the anonymous block descendants that we are about
      // to destroy as well as part of the cleanup process below.
      auto *destroyRootElement = DynamicDowncast<RenderElement>(destroyRoot);
      if (!destroyRootElement)
      {
        return;
      }
      for (auto &descendant : DescendantsOfType<RenderBox>(*destroyRootElement))
      {
        if (descendant.IsFloatingOrOutOfFlowPositioned())
        {
          descendant.RemoveFloatingOrOutOfFlowChildFromBlockLists();
        }
      }
      if (auto *box = DynamicDowncast<RenderBox>(destroyRoot); box && box->IsFloatingOrOutOfFlowPositioned())
      {
        box->RemoveFloatingOrOutOfFlowChildFromBlockLists();
      }
    };
    clearFloatsAndOutOfFlowPositionedObjects();

    auto collapseAndDestroyAnonymousSiblings = [&]
    {
      // FIXME: Probably need to handle other table parts here as well.
      if (auto *cell = DynamicDowncast<RenderTableCell>(destroyRoot))
      {
        TableBuilder().CollapseAndDestroyAnonymousSiblingCells(*cell);
        return;
      }

      if (auto *row = DynamicDowncast<RenderTableRow>(destroyRoot))
      {
        TableBuilder().CollapseAndDestroyAnonymousSiblingRows(*row);
        return;
      }
    };
    collapseAndDestroyAnonymousSiblings();

    // FIXME: Do not try to collapse/cleanup the anonymous wrappers inside destroy (see webkit.org/b/186746).
    auto *destroyRootParent = destroyRoot->Parent();
    if (&rendererToDestroy != destroyRoot)
    {
      // Destroy the child renderer first, before we start tearing down the anonymous wrapper ancestor chain.
      SetForScope anonymousDestroyRoot(_anonymousDestroyRoot, static_cast<const RenderObject *>(destroyRoot));
      Destroy(rendererToDestroy);
    }

    {
      SetForScope anonymousDestroyRoot(
        _anonymousDestroyRoot,
        destroyRoot != &rendererToDestroy ? static_cast<const RenderObject *>(destroyRoot) : nullptr);
      Destroy(*destroyRoot);
    }

    if (!destroyRootParent)
    {
      return;
    }

    SetForScope anonymousDestroyRoot(_anonymousDestroyRoot,
                                     static_cast<const RenderObject *>(destroyRootParent));
    RemoveAnonymousWrappersForInlineChildrenIfNeeded(*destroyRootParent);

    // Anonymous parent might have become empty, try to delete it too.
    if (isAnonymousAndSafeToDelete(*destroyRootParent) && !destroyRootParent->FirstChild())
    {
      DestroyAndCleanUpAnonymousWrappers(*destroyRootParent, destroyRootParent);
    }
    // WARNING: rendererToDestroy is deleted here.
  }

  void RenderTreeBuilder::UpdateAfterDescendants(RenderElement &renderer) noexcept
  {
    // FIXME: SVG roots, once there are SVG renderers.

    // Do not early return here in any case. For example, RenderListItem derives
    // from RenderBlockFlow and indirectly from RenderBlock thus fulfilling all
    // update conditions below.
    if (auto *block = DynamicDowncast<RenderBlock>(renderer))
    {
      FirstLetterBuilder().UpdateAfterDescendants(*block);
    }
    if (auto *listItem = DynamicDowncast<RenderListItem>(renderer))
    {
      ListBuilder().UpdateItemMarker(*listItem);
    }
    // FIXME: Multi-column flows and form controls, once there are their builders.
  }

  RenderPtr<RenderObject> RenderTreeBuilder::DetachFromRenderGrid(RenderGrid &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto takenChild = BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    // Positioned grid items do not take up space or otherwise participate in the layout of the grid,
    // for that reason we don't need to mark the grid as dirty when they are removed.
    if (child.IsOutOfFlowPositioned())
    {
      return takenChild;
    }

    // The grid needs to be recomputed as it might contain auto-placed items that will change their position.
    parent.SetNeedsItemPlacement();
    return takenChild;
  }

  static void ResetRendererStateOnDetach(RenderElement &parent, RenderObject &child,
                                         RenderTreeBuilder::WillBeDestroyed willBeDestroyed,
                                         RenderTreeBuilder::IsInternalMove isInternalMove) noexcept
  {
    if (child.IsFloatingOrOutOfFlowPositioned())
    {
      Downcast<RenderBox>(child).RemoveFloatingOrOutOfFlowChildFromBlockLists();
    }
    else if (auto *parentFlexibleBox = DynamicDowncast<RenderFlexibleBox>(parent))
    {
      if (auto *childBox = DynamicDowncast<RenderBox>(child))
      {
        parentFlexibleBox->ClearCachedFlexItemIntrinsicContentLogicalHeight(*childBox);
        parentFlexibleBox->ClearCachedMainSizeForFlexItem(*childBox);
      }
    }

    if (willBeDestroyed == RenderTreeBuilder::WillBeDestroyed::No)
    {
      child.SetNeedsLayoutAndPreferredWidthsUpdate();
    }

    // FIXME: The legacy text boxes of SVG text, once there's SVG.

    if (auto *listItemRenderer = DynamicDowncast<RenderListItem>(child);
        listItemRenderer && isInternalMove == RenderTreeBuilder::IsInternalMove::No)
    {
      listItemRenderer->UpdateListMarkerNumbers();
    }
  }

  RenderPtr<RenderObject> RenderTreeBuilder::DetachFromRenderElement(RenderElement &parent,
                                                                     RenderObject &child,
                                                                     WillBeDestroyed willBeDestroyed) noexcept
  {
    // FIXME: Assert layout isn't mutating the render tree, once there's layout.
    krys_debug_assert(parent.CanHaveChildren() || parent.CanHaveGeneratedChildren());
    krys_debug_assert(child.Parent() == &parent);

    if (parent.RenderTreeBeingDestroyed() || _tearDownType == TearDownType::SubtreeWithRootAlreadyDetached)
    {
      return parent.DetachRendererInternal(child);
    }

    if (child.EverHadLayout())
    {
      ResetRendererStateOnDetach(parent, child, willBeDestroyed, _internalMovesType);
    }

    if (_tearDownType == TearDownType::Root || Is<RenderInline>(_subtreeDestroyRoot))
    {
      // In case of partial damage on the inline content (the block root is not going away), we need to
      // initiate inline layout invalidation on leaf renderers too.
      InvalidateLineLayout(child, IsRemoval::Yes);
    }

    // FIXME: The fragmented flow state, once there are fragmented flows.

    child.WillBeRemovedFromTree();
    // WARNING: There should be no code running between willBeRemovedFromTree() and the actual removal below.
    // This is needed to avoid race conditions where willBeRemovedFromTree() would dirty the tree's structure
    // and the code running here would force an untimely rebuilding, leaving |child| dangling.
    auto childToTake = parent.DetachRendererInternal(child);

    // FIXME: The accessibility cache, once there is one.

    return childToTake;
  }

  void RenderTreeBuilder::AttachToRenderGrid(RenderGrid &parent, RenderPtr<RenderObject> child,
                                             RenderObject *beforeChild) noexcept
  {
    // The grid needs to be recomputed as it might contain auto-placed items that
    // will change their position. Positioned grid items do not take up space or
    // otherwise participate in the layout of the grid, for that reason we don't
    // need to mark the grid as dirty when they are added.
    if (!child->IsOutOfFlowPositioned())
    {
      parent.SetNeedsItemPlacement();
    }

    BlockBuilder().Attach(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::MarkBoxForRelayoutAfterSplit(RenderBoxModelObject &box) noexcept
  {
    // FIXME: The table code should handle that automatically. If not,
    // we should fix it and remove the table part checks.
    if (auto *table = DynamicDowncast<RenderTable>(box))
    {
      // Because we may have added some sections with already computed column structures, we need to
      // sync the table structure with them now. This avoids crashes when adding new cells to the table.
      table->ForceSectionsRecalc();
    }
    else if (auto *tableSection = DynamicDowncast<RenderTableSection>(box))
    {
      tableSection->SetNeedsCellRecalc();
    }

    box.SetNeedsLayoutAndPreferredWidthsUpdate();
  }

  void RenderTreeBuilder::RemoveFloatingObjects(RenderBlock &renderer) noexcept
  {
    if (renderer.RenderTreeBeingDestroyed())
    {
      return;
    }

    auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderer);
    if (!blockFlow)
    {
      return;
    }

    auto *floatingObjects = blockFlow->GetFloatingObjectSet();
    if (!floatingObjects)
    {
      return;
    }
    // Here we remove the floating objects from the descendants as well.
    krys::List<FloatingObject *> copyOfFloatingObjects;
    for (const auto &floatingObject : *floatingObjects)
    {
      copyOfFloatingObjects.push_back(floatingObject.get());
    }
    for (auto *floatingObject : copyOfFloatingObjects)
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }
      floatingObject->Renderer()->RemoveFloatingOrOutOfFlowChildFromBlockLists();
    }
  }

  RenderPtr<RenderBox>
    RenderTreeBuilder::CreateAnonymousBoxWithSameTypeAndWithStyle(const RenderBox &renderer,
                                                                  const RenderStyle &style) noexcept
  {
    if (Is<RenderTableCell>(renderer))
    {
      return Table::CreateAnonymousTableCellWithStyle(renderer.GetDocument(), style);
    }

    if (Is<RenderTableRow>(renderer))
    {
      return Table::CreateAnonymousTableRowWithStyle(renderer.GetDocument(), style);
    }

    if (Is<RenderTableSection>(renderer))
    {
      return Table::CreateAnonymousTableSectionWithStyle(renderer.GetDocument(), style);
    }

    if (Is<RenderTable>(renderer))
    {
      return Table::CreateAnonymousTableWithStyle(renderer.GetDocument(), style);
    }

    if (Is<RenderBlock>(renderer))
    {
      return Block::CreateAnonymousBlockWithStyle(renderer.GetDocument(), style);
    }

    krys_debug_assert(false);
    return {};
  }
}
