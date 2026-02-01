#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Document/Document.hpp"

namespace Krys::HTML
{
  ProcessingInstruction::ProcessingInstruction(Document &document, DOMString &&target,
                                               DOMString &&data) noexcept
      : CharacterData(document, Krys::Move(data), NodeType::PROCESSING_INSTRUCTION_NODE),
        _target(Krys::Move(target))
  {
  }

  DOMString ProcessingInstruction::NodeName() const noexcept
  {
    return _target;
  }
}