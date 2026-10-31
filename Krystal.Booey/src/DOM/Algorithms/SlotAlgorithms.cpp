#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include <algorithm>

namespace krys::boo::dom
{
#pragma region Slottables

  bool SlotAlgorithms::IsSlottable(const Node &node) noexcept
  {
    return Is<Element>(node) || Is<Text>(node);
  }

  bool SlotAlgorithms::IsSlottable(const Text &) noexcept
  {
    return true;
  }

  bool SlotAlgorithms::IsSlottable(const Element &) noexcept
  {
    return true;
  }

  html::HTMLSlotElement *SlotAlgorithms::DefaultSlot(Node &node) noexcept
  {
    krys_debug_assert(Is<ShadowRoot>(TreeQueries::Root(node)));

    for (auto &descendant : DescendantRange(TreeQueries::Root(node)))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant))
      {
        if (slot->Name().empty())
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  const html::HTMLSlotElement *SlotAlgorithms::DefaultSlot(const Node &node) noexcept
  {
    krys_debug_assert(Is<ShadowRoot>(TreeQueries::Root(node)));

    for (auto &descendant : ConstDescendantRange(TreeQueries::Root(node)))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant))
      {
        if (slot->Name().empty())
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  bool SlotAlgorithms::IsAssigned(const Node &node) noexcept
  {
    if (Is<Element>(node))
    {
      return IsAssigned(Downcast<Element>(node));
    }

    if (Is<Text>(node))
    {
      return IsAssigned(Downcast<Text>(node));
    }

    return false;
  }

  bool SlotAlgorithms::IsAssigned(const Text &node) noexcept
  {
    return node._assignedSlot != nullptr;
  }

  bool SlotAlgorithms::IsAssigned(const Element &node) noexcept
  {
    return node._assignedSlot != nullptr;
  }

  html::HTMLSlotElement *SlotAlgorithms::GetAssignedSlot(const Node &node) noexcept
  {
    if (Is<Element>(node))
    {
      return GetAssignedSlot(Downcast<Element>(node));
    }

    if (Is<Text>(node))
    {
      return GetAssignedSlot(Downcast<Text>(node));
    }

    return nullptr;
  }

  html::HTMLSlotElement *SlotAlgorithms::GetAssignedSlot(const Text &node) noexcept
  {
    return node._assignedSlot.get();
  }

  html::HTMLSlotElement *SlotAlgorithms::GetAssignedSlot(const Element &node) noexcept
  {
    return node._assignedSlot.get();
  }

  html::HTMLSlotElement *SlotAlgorithms::GetManualSlotAssignment(const Node &slottable) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(slottable))
    {
      return element->_manuallyAssignedSlot.get();
    }
    if (auto *text = DynamicDowncast<Text>(slottable))
    {
      return text->_manuallyAssignedSlot.get();
    }
    return nullptr;
  }

  void SlotAlgorithms::SetManualSlotAssignment(Node &slottable, html::HTMLSlotElement *slot) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(slottable))
    {
      element->_manuallyAssignedSlot = CreateWeakPtr(slot);
    }
    else if (auto *text = DynamicDowncast<Text>(slottable))
    {
      text->_manuallyAssignedSlot = CreateWeakPtr(slot);
    }
  }

#pragma endregion

#pragma region Finding slots and slotted elements

  html::HTMLSlotElement *SlotAlgorithms::FindSlot(Node &slottable, bool open) noexcept
  {
    krys_debug_assert(IsSlottable(slottable));

    // 1. If slottable's parent is null, then return null.
    // Only an element parent can have a shadow root.
    Element *parent = slottable.ParentElement();
    if (parent == nullptr)
    {
      return nullptr;
    }

    // 2. Let shadow be slottable's parent's shadow root.
    RefPtr<ShadowRoot> shadow = ShareRefPtr(TreeQueries::GetShadowRoot(*parent));

    // 3. If shadow is null, then return null.
    if (shadow == nullptr)
    {
      return nullptr;
    }

    // 4. If open is true and shadow's mode is not "open", then return null.
    if (open && shadow->Mode() != ShadowRootMode::Open)
    {
      return nullptr;
    }

    // A details element's first summary child takes its summary slot, and its other children its content
    // slot.
    if (auto *details = DynamicDowncast<html::HTMLDetailsElement>(*parent);
        details && shadow->Mode() == ShadowRootMode::UserAgent)
    {
      return details->SlotForHostChild(slottable);
    }

    // 5. If shadow's slot assignment is "manual", then return the slot in shadow's descendants whose manually
    //    assigned nodes contains slottable, if any; otherwise null.
    if (shadow->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      for (auto &descendant : DescendantRange(*shadow))
      {
        auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant);
        if (slot == nullptr)
        {
          continue;
        }

        for (auto &manuallyAssignedNode : slot->_manuallyAssignedNodes)
        {
          if (manuallyAssignedNode == &slottable)
          {
            return slot;
          }
        }
      }

      return nullptr;
    }

    // 6. Return the first slot in tree order in shadow's descendants whose name is slottable's name, if any;
    //    otherwise null.
    auto &slottableName = Is<Element>(slottable) ? Downcast<Element>(slottable)._slottableName
                                                 : Downcast<Text>(slottable)._slottableName;

    for (auto &descendant : DescendantRange(*shadow))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(descendant))
      {
        if (slot->Name() == slottableName)
        {
          return slot;
        }
      }
    }

    return nullptr;
  }

  List<Ref<Node>> SlotAlgorithms::FindSlottables(html::HTMLSlotElement &slot) noexcept
  {
    // 1. Let result be « ».
    List<Ref<Node>> result;

    // 2. Let root be slot's root.
    ShadowRoot *root = DynamicDowncast<ShadowRoot>(TreeQueries::Root(slot));

    // 3. If root is not a shadow root, then return result.
    if (root == nullptr)
    {
      return result;
    }

    // 4. Let host be root's host.
    Element *host = root->Host();

    // 5. If root's slot assignment is "manual":
    if (root->SlotAssignment() == SlotAssignmentMode::Manual)
    {
      // 5a. For each slottable slottable of slot's manually assigned nodes, if slottable's parent is host,
      //     append slottable to result.
      for (auto &manuallyAssignedNode : slot._manuallyAssignedNodes)
      {
        if (auto slottable = manuallyAssignedNode.lock())
        {
          if (slottable->ParentNode() == host)
          {
            result.push_back(ShareRef(*slottable));
          }
        }
      }
    }
    // 6. Otherwise, for each slottable child slottable of host, in tree order:
    else
    {
      for (auto &slottable : ChildNodeRange(host))
      {
        if (IsSlottable(slottable))
        {
          // 6a. Let foundSlot be the result of finding a slot given slottable.
          auto *foundSlot = FindSlot(slottable);

          // 6b. If foundSlot is slot, then append slottable to result.
          if (foundSlot == &slot)
          {
            result.push_back(ShareRef(slottable));
          }
        }
      }
    }

    // 7. Return result.
    return result;
  }

  List<Ref<Node>> SlotAlgorithms::FindFlattenedSlottables(html::HTMLSlotElement &slot) noexcept
  {
    // 1. Let result be « ».
    List<Ref<Node>> result;

    // 2. If slot's root is not a shadow root, then return result.
    if (!TreeQueries::IsInShadowTree(slot))
    {
      return result;
    }

    // 3. Let slottables be the result of finding slottables given slot.
    auto slottables = FindSlottables(slot);

    // 4. If slottables is the empty list, then append each slottable child of slot, in tree order, to
    //    slottables.
    if (slottables.empty())
    {
      for (auto &child : ChildNodeRange(slot))
      {
        if (IsSlottable(child))
        {
          slottables.push_back(ShareRef(child));
        }
      }
    }

    // 5. For each node of slottables:
    for (auto &node : slottables)
    {
      // 5a. If node is a slot whose root is a shadow root:
      if (Is<html::HTMLSlotElement>(node) && TreeQueries::IsInShadowTree(*node))
      {
        // 5a.i. Let temporaryResult be the result of finding flattened slottables given node.
        auto temporaryResult = FindFlattenedSlottables(Downcast<html::HTMLSlotElement>(*node));

        // 5a.ii. Append each slottable in temporaryResult, in order, to result.
        result.append_range(std::move(temporaryResult));
      }
      // 5b. Otherwise, append node to result.
      else
      {
        result.push_back(node);
      }
    }

    // 6. Return result.
    return result;
  }

#pragma endregion

#pragma region Assigning slottables and slots

  void SlotAlgorithms::AssignSlottables(html::HTMLSlotElement &slot) noexcept
  {
    // 1. Let slottables be the result of finding slottables for slot.
    auto slottables = FindSlottables(slot);

    // 2. If slottables and slot's assigned nodes are not identical, then run signal a slot change for slot.
    bool assignedNodesChanged = !std::ranges::equal(
      slottables, slot._assignedNodes, [](const auto &a, const auto &b) { return a.get() == b.get(); });
    if (assignedNodesChanged)
    {
      SignalSlotChange(slot);

      // The composed tree changed under the host.
      if (auto *shadowRoot = TreeQueries::ContainingShadowRoot(slot))
      {
        if (auto host = ShareRefPtr(shadowRoot->Host()))
        {
          RenderTreeUpdater::TearDownRenderersAfterSlotChange(*host);
          host->InvalidateStyleForSubtree();
        }
      }
    }

    // A slottable that's no longer assigned to the slot no longer has it as its assigned slot.
    for (auto &weakNode : slot._assignedNodes)
    {
      auto *node = weakNode.get();
      if (node && GetAssignedSlot(*node) == &slot
          && !std::ranges::any_of(slottables, [&](const auto &slottable) { return slottable.get() == node; }))
      {
        if (auto *element = DynamicDowncast<Element>(*node))
        {
          element->_assignedSlot = nullptr;
        }
        else if (auto *text = DynamicDowncast<Text>(*node))
        {
          text->_assignedSlot = nullptr;
        }
      }
    }

    // 3. Set slot's assigned nodes to slottables.
    // 4. For each slottable of slottables: set slottable's assigned slot to slot.
    slot._assignedNodes.clear();
    slot._assignedNodes.reserve(slottables.size());
    for (auto &slottable : slottables)
    {
      slot._assignedNodes.push_back(CreateWeakRef<Node>(*slottable));
      if (auto *element = DynamicDowncast<Element>(*slottable))
      {
        element->_assignedSlot = CreateWeakPtr(&slot);
      }
      else if (auto *text = DynamicDowncast<Text>(*slottable))
      {
        text->_assignedSlot = CreateWeakPtr(&slot);
      }
    }

    if (assignedNodesChanged && slot.SelfOrPrecedingNodesAffectDirAuto())
    {
      slot.UpdateEffectiveTextDirection();
    }
  }

  void SlotAlgorithms::AssignSlottablesForTree(Node &root) noexcept
  {
    for (auto &inclusiveDescendant : InclusiveDescendantRange(root))
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(inclusiveDescendant))
      {
        AssignSlottables(*slot);
      }
    }
  }

  void SlotAlgorithms::AssignSlot(Node &slottable) noexcept
  {
    // 1. Let slot be the result of finding a slot with slottable.
    html::HTMLSlotElement *slot = FindSlot(slottable);

    // 2. If slot is non-null, then run assign slottables for slot.
    if (slot != nullptr)
    {
      AssignSlottables(*slot);
    }
  }

#pragma endregion

#pragma region Signaling slot change

  void SlotAlgorithms::SignalSlotChange(html::HTMLSlotElement &slot) noexcept
  {
    // 1. Append slot to slot's relevant agent's signal slots.
    MutationObserverAlgorithms::AppendSignalSlot(slot);

    // 2. Queue a mutation observer microtask.
    MutationObserverAlgorithms::QueueMutationObserverMicrotask();
  }

#pragma endregion
}