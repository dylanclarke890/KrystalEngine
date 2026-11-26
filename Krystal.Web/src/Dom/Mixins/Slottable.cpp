#include "Krystal.Web/Dom/Mixins/Slottable.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Trees/SlotAlgorithms.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web::dom::mixins
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
