#include "Krystal.Booey/Rendering/Style/PositionTryOrder.hpp"

namespace krys::boo::style
{
  LogicalBoxAxis BoxAxisForPositionTryOrder(PositionTryOrder order, WritingMode writingMode) noexcept
  {
    switch (order)
    {
      case PositionTryOrder::MostWidth:      return MapAxisPhysicalToLogical(writingMode, BoxAxis::Horizontal);
      case PositionTryOrder::MostHeight:     return MapAxisPhysicalToLogical(writingMode, BoxAxis::Vertical);
      case PositionTryOrder::MostBlockSize:  return LogicalBoxAxis::Block;
      case PositionTryOrder::MostInlineSize: return LogicalBoxAxis::Inline;
      case PositionTryOrder::Normal:         break;
    }
    krys_debug_assert_msg(false, "Unreachable");
    return LogicalBoxAxis::Inline;
  }
}
