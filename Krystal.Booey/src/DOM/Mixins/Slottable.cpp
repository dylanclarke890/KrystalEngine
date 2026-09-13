#include "Krystal.Booey/DOM/Mixins/Slottable.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::mixins
{
  RefPtr<html::HTMLSlotElement> Slottable::AssignedSlot(Text &node) noexcept
  {
    return ShareRefPtr(SlotAlgorithms::FindSlot(node, true));
  }

  RefPtr<html::HTMLSlotElement> Slottable::AssignedSlot(Element &node) noexcept
  {
    return ShareRefPtr(SlotAlgorithms::FindSlot(node, true));
  }
}