#include "Krystal.Booey/Layout/FormattingState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"

namespace krys::boo::layout
{
  FormattingState::FormattingState(Type type, LayoutState &layoutState) noexcept
      : _layoutState(ShareCheckedRef(layoutState)), _type(type)
  {
  }

  FormattingState::~FormattingState() noexcept = default;

  void FormattingState::SetIntrinsicWidthConstraintsForBox(
    const Box &layoutBox, IntrinsicWidthConstraints intrinsicWidthConstraints) noexcept
  {
    krys_debug_assert(!_intrinsicWidthConstraintsForBoxes.contains(&layoutBox));
    krys_debug_assert(
      &_layoutState->FormattingStateForFormattingContext(FormattingContext::FormattingContextRoot(layoutBox))
      == this);
    _intrinsicWidthConstraintsForBoxes[&layoutBox] = intrinsicWidthConstraints;
  }

  void FormattingState::ClearIntrinsicWidthConstraints(const Box &layoutBox) noexcept
  {
    _intrinsicWidthConstraints = null;
    _intrinsicWidthConstraintsForBoxes.erase(&layoutBox);
  }

  Maybe<IntrinsicWidthConstraints>
    FormattingState::IntrinsicWidthConstraintsForBox(const Box &layoutBox) const noexcept
  {
    krys_debug_assert(
      &_layoutState->FormattingStateForFormattingContext(FormattingContext::FormattingContextRoot(layoutBox))
      == this);
    auto iterator = _intrinsicWidthConstraintsForBoxes.find(&layoutBox);
    if (iterator == _intrinsicWidthConstraintsForBoxes.end())
    {
      return null;
    }
    return iterator->second;
  }

  BoxGeometry &FormattingState::GetBoxGeometry(const Box &layoutBox) noexcept
  {
    // FIXME: Remove this when all FormattingStates transtioned to a cache setup.
    return _layoutState->EnsureGeometryForBox(layoutBox);
  }
}
