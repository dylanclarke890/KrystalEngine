#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class HTMLSlotElement;
  class Element;
  class Text;
  class ContainerNode;
  class Node;

  class SlotAssignmentAlgorithms
  {
  public:
#pragma region Finding slots and slotted elements - https://dom.spec.whatwg.org/#finding-slots-and-slotables

    /// @see https://dom.spec.whatwg.org/#find-a-slot
    KRYS_NODISCARD static RawPtr<HTMLSlotElement> FindSlot(Node &slottable, bool open = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#find-slotables
    KRYS_NODISCARD static List<Ref<Node>> FindSlottables(HTMLSlotElement &slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#find-flattened-slotables
    KRYS_NODISCARD static List<Ref<Node>> FindFlattenedSlottables(HTMLSlotElement &slot) noexcept;

#pragma endregion

#pragma region Assigning slottables and slots https://dom.spec.whatwg.org/#assigning-slotables-and-slots

    /// @see https://dom.spec.whatwg.org/#assign-slotables
    static void AssignSlottables(HTMLSlotElement &slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#assign-slotables-for-a-tree
    static void AssignSlottablesForTree(ContainerNode &root) noexcept;

    /// @see https://dom.spec.whatwg.org/#assign-a-slot
    static void AssignSlot(Node &slottable) noexcept;

#pragma endregion

#pragma region Signaling slot change - https://dom.spec.whatwg.org/#signaling-slot-change

    /// @see https://dom.spec.whatwg.org/#signal-a-slot-change
    static void SignalSlotChange(HTMLSlotElement &slot) noexcept;

#pragma endregion
  };
}