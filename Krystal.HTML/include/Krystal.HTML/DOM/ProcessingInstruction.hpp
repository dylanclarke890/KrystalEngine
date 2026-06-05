#pragma once

#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#interface-processinginstruction
  class ProcessingInstruction : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ProcessingInstruction);

    friend class Document;
    friend class NodeAlgorithms;

  private:
    DOMString _target;

  protected:
    ProcessingInstruction(Document &document, DOMString &&target, DOMString &&data) noexcept;

  public:
#pragma region ProcessingInstruction - https://dom.spec.whatwg.org/#processinginstruction

    /// @see - https://dom.spec.whatwg.org/#dom-processinginstruction-target
    KRYS_NODISCARD const DOMString &Target() const noexcept
    {
      return _target;
    }

#pragma endregion

#pragma region Node

    /// @see - https://dom.spec.whatwg.org/#dom-node-nodename
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