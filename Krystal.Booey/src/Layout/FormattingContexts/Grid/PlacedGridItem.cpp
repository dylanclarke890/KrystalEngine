#include "Krystal.Booey/Layout/FormattingContexts/Grid/PlacedGridItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/UnplacedGridItem.hpp"

namespace krys::boo::layout
{
  PlacedGridItem::PlacedGridItem(const UnplacedGridItem &unplacedGridItem, GridAreaLines gridAreaLines,
                                 const ComputedSizes &inlineAxisSizes, const ComputedSizes &blockAxisSizes,
                                 const StyleSelfAlignmentData &inlineAxisAlignment,
                                 const StyleSelfAlignmentData &blockAxisAlignment,
                                 const style::ZoomFactor &usedZoom) noexcept
      : _layoutBox(unplacedGridItem._layoutBox), _inlineAxisSizes(inlineAxisSizes),
        _blockAxisSizes(blockAxisSizes), _inlineAxisAlignment(inlineAxisAlignment),
        _blockAxisAlignment(blockAxisAlignment), _usedZoom(usedZoom), _gridAreaLines(gridAreaLines)
  {
  }
}
