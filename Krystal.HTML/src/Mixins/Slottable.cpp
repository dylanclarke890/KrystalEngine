#include "Krystal.HTML/Mixins/Slottable.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"

namespace Krys::HTML::Mixins
{
  RawPtr<HTMLSlotElement> Slottable::AssignedSlot(Text &node) noexcept
  {
    return SlotAlgorithms::FindSlot(node, true);
  }

  RawPtr<HTMLSlotElement> Slottable::AssignedSlot(Element &node) noexcept
  {
    return SlotAlgorithms::FindSlot(node, true);
  }
}