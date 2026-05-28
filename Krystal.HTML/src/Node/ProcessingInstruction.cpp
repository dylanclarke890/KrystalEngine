#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"

namespace Krys::HTML
{
  ProcessingInstruction::ProcessingInstruction(Document &document, DOMString &&target,
                                               DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), NodeType::PROCESSING_INSTRUCTION_NODE),
        _target(Krys::Move(target))
  {
  }
}