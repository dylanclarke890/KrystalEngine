#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class ContainerNode;
  class Element;
  class HTMLSlotElement;
  class Node;
  class Text;
}

namespace krys::boo::dom
{
  class SlotAlgorithms
  {
  public:
#pragma region Slottables

    /// @see https://dom.spec.whatwg.org/#concept-slotable
    KRYS_NODISCARD static bool IsSlottable(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-slotable
    KRYS_NODISCARD static bool IsSlottable(const Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-slotable
    KRYS_NODISCARD static bool IsSlottable(const Element &node) noexcept;

    /// @brief The first slot in a shadow tree, in tree order, whose name is the empty string, is sometimes
    /// known as the "default slot".
    KRYS_NODISCARD static HTMLSlotElement *DefaultSlot(Node &node) noexcept;

    /// @brief The first slot in a shadow tree, in tree order, whose name is the empty string, is sometimes
    /// known as the "default slot".
    KRYS_NODISCARD static const HTMLSlotElement *DefaultSlot(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#slotable-assigned
    KRYS_NODISCARD static bool IsAssigned(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#slotable-assigned
    KRYS_NODISCARD static bool IsAssigned(const Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#slotable-assigned
    KRYS_NODISCARD static bool IsAssigned(const Element &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    KRYS_NODISCARD static HTMLSlotElement *GetAssignedSlot(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    KRYS_NODISCARD static HTMLSlotElement *GetAssignedSlot(const Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    KRYS_NODISCARD static HTMLSlotElement *GetAssignedSlot(const Element &node) noexcept;

    /// @brief The slot @p slottable is manually assigned to, if any.
    /// @see https://dom.spec.whatwg.org/#slottable-manual-slot-assignment
    KRYS_NODISCARD static HTMLSlotElement *GetManualSlotAssignment(const Node &slottable) noexcept;

    /// @brief Sets @p slottable's manual slot assignment to @p slot.
    /// @see https://dom.spec.whatwg.org/#slottable-manual-slot-assignment
    static void SetManualSlotAssignment(Node &slottable, HTMLSlotElement *slot) noexcept;

#pragma endregion

#pragma region Finding slots and slotted elements - https://dom.spec.whatwg.org/#finding-slots-and-slotables

    /// @see https://dom.spec.whatwg.org/#find-a-slot
    KRYS_NODISCARD static HTMLSlotElement *FindSlot(Node &slottable, bool open = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#find-slotables
    KRYS_NODISCARD static List<Ref<Node>> FindSlottables(HTMLSlotElement &slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#find-flattened-slotables
    KRYS_NODISCARD static List<Ref<Node>> FindFlattenedSlottables(HTMLSlotElement &slot) noexcept;

#pragma endregion

#pragma region Assigning slottables and slots https://dom.spec.whatwg.org/#assigning-slotables-and-slots

    /// @see https://dom.spec.whatwg.org/#assign-slotables
    static void AssignSlottables(HTMLSlotElement &slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#assign-slotables-for-a-tree
    static void AssignSlottablesForTree(Node &root) noexcept;

    /// @see https://dom.spec.whatwg.org/#assign-a-slot
    static void AssignSlot(Node &slottable) noexcept;

#pragma endregion

#pragma region Signaling slot change - https://dom.spec.whatwg.org/#signaling-slot-change

    /// @see https://dom.spec.whatwg.org/#signal-a-slot-change
    static void SignalSlotChange(HTMLSlotElement &slot) noexcept;

#pragma endregion
  };
}
