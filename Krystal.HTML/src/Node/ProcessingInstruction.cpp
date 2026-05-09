#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ProcessingInstruction::ProcessingInstruction(Document &document, DOMString &&target,
                                               DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), NodeType::PROCESSING_INSTRUCTION_NODE),
        _target(Krys::Move(target))
  {
  }
}