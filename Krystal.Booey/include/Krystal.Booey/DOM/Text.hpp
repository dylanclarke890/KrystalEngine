#pragma once

#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
  class RenderText;
}

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
    WeakPtr<html::HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slotable-manual-slot-assignment
    WeakPtr<html::HTMLSlotElement> _manuallyAssignedSlot;

  protected:
    struct CDATAConstructorTag
    {
    };

    Text(Document &document, DOMString &&data, CDATAConstructorTag) noexcept;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return true;
    }

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

    KRYS_NODISCARD RenderText *Renderer() const noexcept;

    /// @brief Creates the renderer of this text, whose parent renderer has @p style.
    KRYS_NODISCARD RenderPtr<RenderText> CreateTextRenderer(const RenderStyle &style) noexcept;

    /// @brief Schedules an update of the renderer after @p lengthOfReplacedData bytes from
    /// @p offsetOfReplacedData were replaced.
    void UpdateRendererAfterContentChange(size_t offsetOfReplacedData, size_t lengthOfReplacedData) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Text)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsTextNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();