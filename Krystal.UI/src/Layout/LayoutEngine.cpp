#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include "Krystal.UI/Elements/Element.hpp"

namespace Krys::UI
{
  void LayoutEngine::Reflow(class Element &element, Maths::Vec2 containingBlock)
  {
    assert(containingBlock.x >= 0 && containingBlock.y >= 0);
    NodeCalculateLayout(element.LayoutNode, containingBlock.x, containingBlock.y, Direction::LTR);
  }
}