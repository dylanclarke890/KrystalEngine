#include "Krystal.Booey/Dom/Mixins/Slottable.hpp"
#include "Krystal.Booey/Dom/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom::mixins
{
  RefPtr<HTMLSlotElement> Slottable::AssignedSlot(Text &node) noexcept
  {
    return ShareRefPtr(SlotAlgorithms::FindSlot(node, true));
  }

  RefPtr<HTMLSlotElement> Slottable::AssignedSlot(Element &node) noexcept
  {
    return ShareRefPtr(SlotAlgorithms::FindSlot(node, true));
  }
}
