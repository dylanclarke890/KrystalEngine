#include "Krystal.HTML/Algorithms/SlotAssignmentAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
#pragma region Finding slots and slotted elements - https://dom.spec.whatwg.org/#finding-slots-and-slotables

  RawPtr<HTMLSlotElement> SlotAssignmentAlgorithms::FindSlot(Node &slottable, bool open) noexcept
  {
    RawPtr<Element> parent = slottable.ParentElement();

    if (parent == nullptr)
    {
      return nullptr;
    }

    RawPtr<ShadowRoot> shadow = parent->GetShadowRoot();

    if (shadow == nullptr || (open && shadow->Mode() != ShadowRootMode::Open))
    {
      return nullptr;
    }

    if (shadow->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      // TODO(impl):
      // return the slot in shadow’s descendants whose manually assigned nodes contains slottable, if any;
      // otherwise null.
      return nullptr;
    }
    else
    {
      // TODO(impl):
      // Return the first slot in tree order in shadow’s descendants whose name is slottable’s name, if any;
      // otherwise null.
      return nullptr;
    }
  }

  List<Ref<Node>> SlotAssignmentAlgorithms::FindSlottables(HTMLSlotElement &slot) noexcept
  {
    List<Ref<Node>> result;
    RawPtr<ShadowRoot> root = DynamicDowncast<ShadowRoot>(TreeQueries::Root(slot));

    if (root == nullptr)
    {
      return result;
    }

    RawPtr<Element> host = root->Host();
    if (root->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      // TODO(impl):
      // For each slottable slottable of slot’s manually assigned nodes, if slottable’s parent is host, append
      // slottable to result.
    }
    else
    {
      // TODO(impl):
      // or each slottable child slottable of host, in tree order:
      // Let foundSlot be the result of finding a slot given slottable.
      // If foundSlot is slot,
      // then append slottable to result.
    }

    return result;
  }

  List<Ref<Node>> SlotAssignmentAlgorithms::FindFlattenedSlottables(HTMLSlotElement &slot) noexcept
  {
    List<Ref<Node>> result;

    if (!TreeQueries::IsInShadowTree(slot))
    {
      return result;
    }

    auto slottables = FindSlottables(slot);
    if (slottables.empty())
    {
      for (auto &child : ChildNodeRange(slot))
      {
        if (child.IsTextNode() || child.IsElementNode())
        {
          slottables.push_back(ShareRef(child));
        }
      }
    }

    for (auto &node : slottables)
    {
      auto *slotNode = DynamicDowncast<HTMLSlotElement>(*node);
      auto *slotShadowRoot = slotNode ? DynamicDowncast<ShadowRoot>(TreeQueries::Root(*slotNode)) : nullptr;

      if (slotShadowRoot != nullptr)
      {
        result.append_range(FindFlattenedSlottables(*slotNode));
      }
      else
      {
        result.push_back(node);
      }
    }

    return result;
  }

#pragma endregion

#pragma region Assigning slottables and slots https://dom.spec.whatwg.org/#assigning-slotables-and-slots

  void SlotAssignmentAlgorithms::AssignSlottables(HTMLSlotElement &slot) noexcept
  {
    auto slottables = FindSlottables(slot);
    // TODO(impl):
    // if slottables and slot’s assigned nodes are not identical, then run signal a slot change for slot.
    // Set slot’s assigned nodes to slottables.
    // For each slottable of slottables: set slottable’s assigned slot to slot.
  }

  void SlotAssignmentAlgorithms::AssignSlottablesForTree(ContainerNode &root) noexcept
  {
    for (auto &inclusiveDescendant : InclusiveDescendantRange(root))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(inclusiveDescendant))
      {
        AssignSlottables(*slot);
      }
    }
  }

  void SlotAssignmentAlgorithms::AssignSlot(Node &slottable) noexcept
  {
    RawPtr<HTMLSlotElement> slot = FindSlot(slottable);
    if (slot != nullptr)
    {
      AssignSlottables(*slot);
    }
  }

#pragma endregion

#pragma region Signaling slot change - https://dom.spec.whatwg.org/#signaling-slot-change

  void SlotAssignmentAlgorithms::SignalSlotChange(HTMLSlotElement &slot) noexcept
  {
    // TODO(impl):
    // Append slot to slot’s relevant agent’s signal slots
    // Queue a mutation observer microtask.
  }

#pragma endregion
}