#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"

namespace krys::boo::layout
{
  BlockFormattingState::BlockFormattingState(LayoutState &layoutState,
                                             const ElementBox &blockFormattingContextRoot) noexcept
      : FormattingState(Type::Block, layoutState), _placedFloats(blockFormattingContextRoot)
  {
  }

  BlockFormattingState::~BlockFormattingState() noexcept = default;

  void BlockFormattingState::ShrinkToFit() noexcept
  {
    _placedFloats.ShrinkToFit();
  }
}
