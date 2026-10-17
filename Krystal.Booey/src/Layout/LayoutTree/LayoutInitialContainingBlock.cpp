#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"

namespace krys::boo::layout
{
  InitialContainingBlock::InitialContainingBlock(RenderStyle &&style,
                                                 UniquePtr<RenderStyle> &&firstLineStyle) noexcept
      : ElementBox({}, krys::move(style), krys::move(firstLineStyle), Box::InitialContainingBlockFlag)
  {
  }
}
