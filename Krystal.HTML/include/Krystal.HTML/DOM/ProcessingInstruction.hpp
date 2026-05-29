#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class ProcessingInstruction : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProcessingInstruction);

  private:
    DOMString _target;

  public:
    ProcessingInstruction(Document &document, DOMString &&target, DOMString &&data) noexcept;

#pragma region ProcessingInstruction

    KRYS_NODISCARD const DOMString &Target() const noexcept
    {
      return _target;
    }

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return _target;
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ProcessingInstruction)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsProcessingInstructionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()