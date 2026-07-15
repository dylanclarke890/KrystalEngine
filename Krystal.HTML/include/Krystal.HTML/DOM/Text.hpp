#pragma once

#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  class Document;
  class HTMLSlotElement;

  /// @see https://dom.spec.whatwg.org/#interface-text
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

  protected:
    struct CDATAConstructorTag
    {
    };

    Text(Document &document, DOMString &&data, CDATAConstructorTag) noexcept;

  public:
#pragma region Text - https://dom.spec.whatwg.org/#text

    /// @see https://dom.spec.whatwg.org/#dom-text-text
    Text(Document &document, DOMString &&data = u8"") noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-text-splittext
    KRYS_NODISCARD ExceptionOr<Ref<Text>> SplitText(size_t offset) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-text-wholetext
    KRYS_NODISCARD DOMString WholeText() const noexcept;

#pragma endregion

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept override
    {
      return u8"#text";
    }

#pragma endregion

#pragma region Slottable Mixin - https://dom.spec.whatwg.org/#slotable

    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
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