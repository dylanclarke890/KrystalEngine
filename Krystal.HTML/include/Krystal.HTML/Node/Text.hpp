#pragma once

#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  class Document;
  class HTMLSlotElement;

  class Text : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Text);

    friend class SlotAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#slotable-name
    DOMString _slottableName;
    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    RefPtr<HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slotable-manual-slot-assignment
    WeakPtr<HTMLSlotElement> _manuallyAssignedSlot;

  public:
    Text(Document &document, DOMString &&data = u8"", HTML::NodeType type = NodeType::TEXT_NODE,
         NodeFlags flags = NodeFlags::None) noexcept;

#pragma region Text

    /// @see https://dom.spec.whatwg.org/#dom-text-splittext
    KRYS_NODISCARD ExceptionOr<Ref<Text>> SplitText(size_t offset) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-text-wholetext
    KRYS_NODISCARD DOMString WholeText() const noexcept;

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept override
    {
      return u8"#text";
    }

#pragma endregion

#pragma region Slottable Mixin - https://dom.spec.whatwg.org/#slotable

    KRYS_NODISCARD RefPtr<HTMLSlotElement> AssignedSlot() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Text)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsTextNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();