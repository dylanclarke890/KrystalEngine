#pragma once

#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLSlotElement;
}

namespace krys::boo::dom
{
  class Document;

  /// @see https://dom.spec.whatwg.org/#interface-text
  class Text : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Text);

    friend class SlotAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#slotable-name
    DOMString _slottableName;
    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    RefPtr<html::HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slotable-manual-slot-assignment
    WeakPtr<html::HTMLSlotElement> _manuallyAssignedSlot;

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
    KRYS_NODISCARD RefPtr<html::HTMLSlotElement> AssignedSlot() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Text)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsTextNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();