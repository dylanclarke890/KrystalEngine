#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Web/Rendering/RenderPtr.hpp"

namespace krys::web::dom
{
  class SlotAlgorithms;
}

namespace krys::web
{
  class Document;
  class HTMLSlotElement;
  class RenderStyle;
  class RenderText;

  /// @brief A node of text: an exclusive Text node, or the CDATASection that derives from it.
  /// @see https://dom.spec.whatwg.org/#interface-text
  class Text : public CharacterData
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Text);

    friend class dom::SlotAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#slotable-name
    DOMString _slottableName;
    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    WeakPtr<HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slottable-manual-slot-assignment
    WeakPtr<HTMLSlotElement> _manuallyAssignedSlot;

  protected:
    /// @brief Picks the constructor `CDATASection` uses, which gives it its own node type.
    struct CDATAConstructorTag
    {
    };

    Text(Document &document, DOMString data, CDATAConstructorTag) noexcept;

  public:
    /// @brief A range's boundary point can be in a text node, at an offset into its data.
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return true;
    }

#pragma region Text - https://dom.spec.whatwg.org/#text

    /// @brief Creates a text node in @p document with the data.
    /// @see https://dom.spec.whatwg.org/#dom-text-text
    /// @see https://dom.spec.whatwg.org/#create-a-text-node
    Text(Document &document, DOMString data = u8"") noexcept;

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

    /// @brief The node's text renderer, if it's rendered.
    KRYS_NODISCARD RenderText *Renderer() const noexcept;

    /// @brief Creates the renderer of this text, whose parent renderer has @p style.
    KRYS_NODISCARD RenderPtr<RenderText> CreateTextRenderer(const RenderStyle &style) noexcept;

    /// @brief Schedules an update of the renderer after @p lengthOfReplacedData bytes from
    /// @p offsetOfReplacedData were replaced.
    void UpdateRendererAfterContentChange(size_t offsetOfReplacedData, size_t lengthOfReplacedData) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::Text)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &node) noexcept
  {
    return node.IsTextNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
