#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document;

  class ProcessingInstruction : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProcessingInstruction);

  private:
    DOMString _target;

  public:
    ProcessingInstruction(Document &document, DOMString &&target, DOMString &&data) noexcept;

    KRYS_NODISCARD const DOMString &Target() const noexcept
    {
      return _target;
    }

    KRYS_NODISCARD DOMString NodeName() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ProcessingInstruction)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.GetNodeType() == Krys::HTML::NodeType::PROCESSING_INSTRUCTION_NODE;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()