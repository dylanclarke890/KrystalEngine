#pragma once

#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ProcessingInstruction)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsProcessingInstructionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()