#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlockFlow.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool CanDropAnonymousBlock(const RenderBlock &anonymousBlock) noexcept
    {
      // FIXME: Continuations, once there are any.
      if (anonymousBlock.BeingDestroyed())
      {
        return false;
      }
      return true;
    }

    KRYS_NODISCARD bool CanMergeContiguousAnonymousBlocks(const RenderObject &rendererToBeRemoved,
                                                          const RenderObject *previous,
                                                          const RenderObject *next,
                                                          const RenderObject *) noexcept
    {
      krys_debug_assert(!rendererToBeRemoved.RenderTreeBeingDestroyed());

      if (rendererToBeRemoved.IsInline())
      {
        return false;
      }

      if (previous)
      {
        auto *previousBlock = DynamicDowncast<RenderBlock>(*previous);
        if (!previousBlock || !previousBlock->IsAnonymousBlock() || !CanDropAnonymousBlock(*previousBlock))
        {
          return false;
        }
      }

      if (next)
      {
        auto *nextBlock = DynamicDowncast<RenderBlock>(*next);
        if (!nextBlock || !nextBlock->IsAnonymousBlock() || !CanDropAnonymousBlock(*nextBlock))
        {
          return false;
        }
      }

      // FIXME: Merging the pre and post anonymous blocks of a continuation when the box that triggered it
      // goes away, once there are continuations.
      return true;
    }

    struct ParentAndBeforeChild
    {
      RenderElement *Parent {nullptr};
      RenderObject *BeforeChild {nullptr};
    };

    KRYS_NODISCARD Maybe<ParentAndBeforeChild>
      FindParentAndBeforeChildForNonSibling(RenderBlock &parent, const RenderObject &child,
                                            RenderObject &beforeChild) noexcept
    {
      auto *beforeChildContainer = beforeChild.Parent();
      while (beforeChildContainer->Parent() != &parent)
      {
        beforeChildContainer = beforeChildContainer->Parent();
      }

      krys_debug_assert(beforeChildContainer);
      if (!beforeChildContainer || !beforeChildContainer->IsAnonymous())
      {
        return null;
      }

      if (beforeChildContainer->IsInline() && child.IsInline())
      {
        // The before child happens to be a block level box wrapped in an anonymous inline-block in an inline
        // context (e.g. ruby). Let's attach this new child before the anonymous inline-block wrapper.
        krys_debug_assert(beforeChildContainer->IsNonReplacedAtomicInlineLevelBox());
        return ParentAndBeforeChild {&parent, beforeChildContainer};
      }
      krys_assert(!beforeChildContainer->IsInline() || beforeChildContainer->IsRenderTable());

      // If the requested beforeChild is not one of our children, then this is because
      // there is an anonymous container within this object that contains the beforeChild.
      auto &beforeChildAnonymousContainer = *beforeChildContainer;
      if (beforeChildAnonymousContainer.IsAnonymousBlock())
      {
        auto mayUseBeforeChildContainerAsParent = [&]
        {
          auto isFlexOrGridItemContainer = [&]
          {
            if (auto *renderBox = DynamicDowncast<RenderBox>(beforeChildAnonymousContainer))
            {
              return renderBox->IsFlexItemIncludingDeprecated() || renderBox->IsGridItem();
            }
            return false;
          };
          if (child.IsOutOfFlowPositioned() && isFlexOrGridItemContainer())
          {
            // Do not try to move an out-of-flow block box under an anonymous flex/grid item. It should stay a
            // direct child of the flex/grid container. https://www.w3.org/TR/css-flexbox-1/#abspos-items As
            // it is out-of-flow, an absolutely-positioned child of a flex container does not participate in
            // flex layout. The static position of an absolutely-positioned child of a flex container is
            // determined such that the child is positioned as if it were the sole flex item in the flex
            // container,
            return false;
          }
          return child.IsInline() || beforeChildAnonymousContainer.FirstChild() != &beforeChild;
        };
        if (mayUseBeforeChildContainerAsParent())
        {
          return ParentAndBeforeChild {&beforeChildAnonymousContainer, &beforeChild};
        }
        return ParentAndBeforeChild {&parent, beforeChild.Parent()};
      }

      krys_debug_assert(beforeChildAnonymousContainer.IsRenderTable());
      if (child.IsTablePart())
      {
        // Insert into the anonymous table.
        return ParentAndBeforeChild {&beforeChildAnonymousContainer, &beforeChild};
      }

      // parent needs splitting.
      return ParentAndBeforeChild {};
    }
  }

  RenderTreeBuilder::Block::Block(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::Block::MoveAllChildrenToInternal(RenderBoxModelObject &from,
                                                           RenderElement &newParent) noexcept
  {
    while (from.FirstChild())
    {
      newParent.AttachRendererInternal(from.DetachRendererInternal(*from.FirstChild()), &from);
    }
  }

  void RenderTreeBuilder::Block::Attach(RenderBlock &parent, RenderPtr<RenderObject> child,
                                        RenderObject *beforeChild) noexcept
  {
    // FIXME: Inserting into a continuation (insertChildToContinuation), once there are continuations.
    AttachIgnoringContinuation(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::Block::AttachIgnoringContinuation(RenderBlock &parent,
                                                            RenderPtr<RenderObject> child,
                                                            RenderObject *beforeChild) noexcept
  {
    auto parentAndBeforeChildMayNeedAdjustment = beforeChild && beforeChild->Parent() != &parent;
    if (parentAndBeforeChildMayNeedAdjustment)
    {
      if (auto parentAndBeforeChild = FindParentAndBeforeChildForNonSibling(parent, *child, *beforeChild))
      {
        if (parentAndBeforeChild->Parent)
        {
          _builder.Attach(*parentAndBeforeChild->Parent, krys::move(child),
                          parentAndBeforeChild->BeforeChild);
          return;
        }
        beforeChild = _builder.SplitAnonymousBoxesAroundChild(parent, *beforeChild);
        krys_assert(beforeChild->Parent() == &parent);
      }
    }

    if (child->IsFloatingOrOutOfFlowPositioned())
    {
      if (parent.ChildrenInline() || Is<RenderFlexibleBox>(parent) || parent.IsRenderGrid())
      {
        _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
        return;
      }
      // In case of sibling block box(es), let's check if we can add this out of flow/floating box under a
      // previous sibling anonymous block.
      auto *previousSiblingBlock =
        DynamicDowncast<RenderBlock>(beforeChild ? beforeChild->PreviousSibling() : parent.LastChild());
      if (!previousSiblingBlock || !previousSiblingBlock->IsAnonymousBlock())
      {
        _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
        return;
      }
      _builder.Attach(*previousSiblingBlock, krys::move(child));
      return;
    }

    // Parent and inflow child match.
    if ((parent.ChildrenInline() && child->IsInline()) || (!parent.ChildrenInline() && !child->IsInline()))
    {
      return _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
    }

    // Inline parent with block child.
    if (parent.ChildrenInline())
    {
      krys_debug_assert(!child->IsInline() && !child->IsFloatingOrOutOfFlowPositioned());
      // A block has to either have all of its children inline, or all of its children as blocks.
      // So, if our children are currently inline and a block child has to be inserted, we move all our
      // inline children into anonymous block boxes.
      // This is a block with inline content. Wrap the inline content in anonymous blocks.
      _builder.CreateAnonymousWrappersForInlineContent(parent, beforeChild);
      if (beforeChild && beforeChild->Parent() != &parent)
      {
        beforeChild = beforeChild->Parent();
        krys_debug_assert(Is<RenderBlock>(*beforeChild)
                          && Downcast<RenderBlock>(*beforeChild).IsAnonymousBlock());
        krys_debug_assert(beforeChild->Parent() == &parent);
      }
      _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);

      if (Is<RenderBlock>(parent.Parent()) && parent.IsAnonymousBlock())
      {
        RemoveLeftoverAnonymousBlock(parent);
      }
      return;
    }

    // Block parent with inline child.
    // If we're inserting an inline child but all of our children are blocks, then we have to make sure
    // it is put into an anomyous block box. We try to use an existing anonymous box if possible, otherwise
    // a new one is created and inserted into our list of children in the appropriate position.
    auto *previousSibling = beforeChild ? beforeChild->PreviousSibling() : parent.LastChild();
    if (auto *previousBlock = DynamicDowncast<RenderBlock>(previousSibling);
        previousBlock && previousBlock->IsAnonymousBlock())
    {
      _builder.Attach(*previousBlock, krys::move(child));
      return;
    }

    // No suitable existing anonymous box - create a new one.
    auto newBox = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
    auto &box = *newBox;
    _builder.AttachToRenderElement(parent, krys::move(newBox), beforeChild);
    _builder.Attach(box, krys::move(child));
  }

  void RenderTreeBuilder::Block::ChildBecameNonInline(RenderBlock &parent, RenderElement &) noexcept
  {
    _builder.CreateAnonymousWrappersForInlineContent(parent);
    if (parent.IsAnonymousBlock() && Is<RenderBlock>(parent.Parent()))
    {
      RemoveLeftoverAnonymousBlock(parent);
    }
    // parent may be dead here
  }

  void RenderTreeBuilder::Block::RemoveLeftoverAnonymousBlock(RenderBlock &anonymousBlock) noexcept
  {
    krys_debug_assert(anonymousBlock.IsAnonymousBlock());
    krys_debug_assert(!anonymousBlock.ChildrenInline());
    krys_debug_assert(anonymousBlock.Parent());

    // FIXME: Continuations, and buttons and text controls (which keep their anonymous blocks), once there are
    // any.

    auto *parent = anonymousBlock.Parent();

    _builder.RemoveFloatingObjects(anonymousBlock);
    // FIXME: This should really just be a moveAllChilrenTo (see webkit.org/b/182495)
    MoveAllChildrenToInternal(anonymousBlock, *parent);
    auto toBeDestroyed = _builder.DetachFromRenderElement(*parent, anonymousBlock, WillBeDestroyed::Yes);
    // anonymousBlock is dead here.
  }

  RenderPtr<RenderObject>
    RenderTreeBuilder::Block::Detach(RenderBlock &parent, RenderObject &child,
                                     WillBeDestroyed willBeDestroyed,
                                     CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    // No need to waste time in merging or removing empty anonymous blocks.
    // We can just bail out if our document is getting destroyed.
    if (parent.RenderTreeBeingDestroyed())
    {
      return _builder.DetachFromRenderElement(parent, child, willBeDestroyed);
    }

    // If this child is a block, and if our previous and next siblings are both anonymous blocks
    // with inline content, then we can fold the inline content back together.
    auto *previousSibling = child.PreviousSibling();
    auto *nextSibling = child.NextSibling();
    bool canMergeAnonymousBlocks = canCollapseAnonymousBlock == CanCollapseAnonymousBlock::Yes
                                   && CanMergeContiguousAnonymousBlocks(child, previousSibling, nextSibling,
                                                                        _builder._anonymousDestroyRoot);

    auto takenChild = _builder.DetachFromRenderElement(parent, child, willBeDestroyed);

    if (canMergeAnonymousBlocks && previousSibling && nextSibling)
    {
      auto &previousBlock = Downcast<RenderBlock>(*previousSibling);
      auto &nextBlock = Downcast<RenderBlock>(*nextSibling);

      previousBlock.SetNeedsLayoutAndPreferredWidthsUpdate();
      if (previousBlock.ChildrenInline() != nextBlock.ChildrenInline())
      {
        auto &inlineChildrenBlock = previousBlock.ChildrenInline() ? previousBlock : nextBlock;
        auto &blockChildrenBlock = previousBlock.ChildrenInline() ? nextBlock : previousBlock;

        // Place the inline children block inside of the block children block instead of deleting it.
        // In order to reuse it, we have to reset it to just be a generic anonymous block. Make sure
        // to clear out inherited column properties by just making a new style, and to also clear the
        // column span flag if it is set.
        // Cache this value as it might get changed in setStyle() call.
        inlineChildrenBlock.SetStyle(
          RenderStyle::CreateAnonymousStyleWithDisplay(parent.Style(), DisplayType::Block));
        auto blockToMove = _builder.DetachFromRenderElement(parent, inlineChildrenBlock, WillBeDestroyed::No);

        // Now just put the inlineChildrenBlock inside the blockChildrenBlock.
        RenderObject *beforeChild =
          &previousBlock == &inlineChildrenBlock ? blockChildrenBlock.FirstChild() : nullptr;
        _builder.AttachToRenderElementInternal(blockChildrenBlock, krys::move(blockToMove), beforeChild);
        nextBlock.SetNeedsLayoutAndPreferredWidthsUpdate();

        // inlineChildrenBlock got reparented to blockChildrenBlock, so it is no longer a child
        // of "this". we null out previousSibling or nextSibling so that is not used later in the function.
        if (&inlineChildrenBlock == &previousBlock)
        {
          previousSibling = nullptr;
        }
        else
        {
          nextSibling = nullptr;
        }
      }
      else
      {
        // Take all the children out of the |nextSibling| block and put them in
        // the |previousSibling| block.
        _builder.MoveAllChildrenIncludingFloats(nextBlock, previousBlock, NormalizeAfterInsertion::No);

        // Delete the now-empty block's lines and nuke it.
        // FIXME: Invalidate the line layout, once there's line layout.
        _builder.Destroy(nextBlock);
        nextSibling = nullptr;
      }
    }

    if (canCollapseAnonymousBlock == CanCollapseAnonymousBlock::Yes && parent.CanDropAnonymousBlockChild())
    {
      RenderObject *remaining = previousSibling ? previousSibling : nextSibling;
      if (canMergeAnonymousBlocks && remaining && !remaining->PreviousSibling() && !remaining->NextSibling())
      {
        // The removal has knocked us down to containing only a single anonymous box. We can pull the content
        // right back up into our box.
        DropAnonymousBoxChild(parent, Downcast<RenderBlock>(*remaining));
      }
      else
      {
        auto anonymousBlock = [&]() -> RenderBlock *
        {
          if (!previousSibling && !nextSibling)
          {
            return nullptr;
          }
          if (auto *previousBlock = DynamicDowncast<RenderBlock>(previousSibling);
              previousBlock && previousBlock->IsAnonymousBlock())
          {
            return previousBlock;
          }
          if (auto *nextBlock = DynamicDowncast<RenderBlock>(nextSibling);
              nextBlock && nextBlock->IsAnonymousBlock())
          {
            return nextBlock;
          }
          return nullptr;
        };

        if (auto *anonBlock = anonymousBlock(); anonBlock && CanDropAnonymousBlock(*anonBlock))
        {
          bool dropAnonymousBlock = true;
          for (auto &sibling : ChildrenOfType<RenderObject>(parent))
          {
            if (&sibling == anonBlock)
            {
              continue;
            }
            if (!sibling.IsFloating())
            {
              dropAnonymousBlock = false;
              break;
            }
          }
          if (dropAnonymousBlock)
          {
            DropAnonymousBoxChild(parent, *anonBlock);
          }
        }
      }
    }

    // FIXME: If this was our last child, clear out the line boxes, once there's line layout.
    return takenChild;
  }

  void RenderTreeBuilder::Block::DropAnonymousBoxChild(RenderBlock &parent, RenderBlock &child) noexcept
  {
    parent.SetNeedsLayoutAndPreferredWidthsUpdate();
    parent.SetChildrenInline(child.ChildrenInline());

    // FIXME: This should really just be a moveAllChilrenTo (see webkit.org/b/182495)
    MoveAllChildrenToInternal(child, parent);
    auto toBeDeleted = _builder.DetachFromRenderElement(parent, child, WillBeDestroyed::Yes);

    // FIXME: Delete the now-empty block's lines, once there's line layout.
  }

  RenderPtr<RenderObject>
    RenderTreeBuilder::Block::Detach(RenderBlockFlow &parent, RenderObject &child,
                                     WillBeDestroyed willBeDestroyed,
                                     CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    // FIXME: Multi-column flows, once there are any.
    return Detach(static_cast<RenderBlock &>(parent), child, willBeDestroyed, canCollapseAnonymousBlock);
  }

  RenderPtr<RenderBlockFlow>
    RenderTreeBuilder::Block::CreateAnonymousBlockWithStyle(dom::Document &document,
                                                            const RenderStyle &style) noexcept
  {
    auto newBox = CreateRenderer<RenderBlockFlow>(
      RenderObject::Type::BlockFlow, document,
      RenderStyle::CreateAnonymousStyleWithDisplay(style, DisplayType::Block));
    newBox->InitializeStyle();
    return newBox;
  }
}
