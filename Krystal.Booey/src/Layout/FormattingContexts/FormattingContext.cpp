#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"

namespace krys::boo::layout
{
  const InitialContainingBlock &FormattingContext::GetInitialContainingBlock(const Box &layoutBox) noexcept
  {
    if (auto *initialContainingBlock = DynamicDowncast<InitialContainingBlock>(layoutBox))
    {
      return *initialContainingBlock;
    }

    auto *ancestor = &layoutBox.Parent();
    for (; !Is<InitialContainingBlock>(*ancestor); ancestor = &ancestor->Parent())
    {
    }

    return Downcast<InitialContainingBlock>(*ancestor);
  }

  const ElementBox &FormattingContext::ContainingBlock(const Box &layoutBox) noexcept
  {
    // If we ever end up here with the ICB, we must be doing something not-so-great.
    krys_assert(!Is<InitialContainingBlock>(layoutBox));
    // The containing block in which the root element lives is a rectangle called the initial containing
    // block. For other elements, if the element's position is 'relative' or 'static', the containing block is
    // formed by the content edge of the nearest block container ancestor box or which establishes a
    // formatting context. If the element has 'position: fixed', the containing block is established by the
    // viewport If the element has 'position: absolute', the containing block is established by the nearest
    // ancestor with a 'position' of 'absolute', 'relative' or 'fixed'.
    if (!layoutBox.IsPositioned() || layoutBox.IsInFlowPositioned())
    {
      auto *ancestor = &layoutBox.Parent();
      for (; !Is<InitialContainingBlock>(*ancestor); ancestor = &ancestor->Parent())
      {
        if (ancestor->IsContainingBlockForInFlow())
        {
          return *ancestor;
        }
      }
      return *ancestor;
    }

    if (layoutBox.IsFixedPositioned())
    {
      auto *ancestor = &layoutBox.Parent();
      for (; !Is<InitialContainingBlock>(*ancestor); ancestor = &ancestor->Parent())
      {
        if (ancestor->IsContainingBlockForFixedPosition())
        {
          return *ancestor;
        }
      }
      return *ancestor;
    }

    if (layoutBox.IsOutOfFlowPositioned())
    {
      auto *ancestor = &layoutBox.Parent();
      for (; !Is<InitialContainingBlock>(*ancestor); ancestor = &ancestor->Parent())
      {
        if (ancestor->IsContainingBlockForOutOfFlowPosition())
        {
          return *ancestor;
        }
      }
      return *ancestor;
    }

    krys_debug_assert(false);
    return layoutBox.Parent();
  }

#if KRYS_DEBUG
  const ElementBox &FormattingContext::FormattingContextRoot(const Box &layoutBox) noexcept
  {
    // We should never need to ask this question on the ICB.
    krys_debug_assert(!Is<InitialContainingBlock>(layoutBox));
    // A box lives in the same formatting context as its containing block unless the containing block
    // establishes a formatting context. However relatively positioned (inflow) inline container lives in the
    // formatting context where its parent lives unless the parent establishes a formatting context.
    //
    // <div id=outer style="position: absolute"><div id=inner><span style="position:
    // relative">content</span></div></div> While the relatively positioned inline container (span) is placed
    // relative to its containing block "outer", it lives in the inline formatting context established by
    // "inner".
    auto &ancestor = layoutBox.IsInlineLevelBox() && layoutBox.IsInFlowPositioned()
                       ? layoutBox.Parent()
                       : ContainingBlock(layoutBox);
    if (ancestor.EstablishesFormattingContext())
    {
      return ancestor;
    }
    return FormattingContextRoot(ancestor);
  }
#endif
}
