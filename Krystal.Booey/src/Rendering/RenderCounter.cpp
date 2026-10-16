#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLOListElement.hpp"
#include "Krystal.Booey/Rendering/CounterNode.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  namespace
  {
    using CounterMap = HashMap<css::CSSOMStringAtom, Ref<CounterNode>>;
    // Entries are removed when their renderer is destroyed (RenderElement::WillBeDestroyed).
    using CounterMaps = HashMap<const RenderElement *, UniquePtr<CounterMap>>;

    KRYS_NODISCARD CounterNode *MakeCounterNode(RenderElement &renderer,
                                                const css::CSSOMStringAtom &identifier,
                                                bool alwaysCreateCounter) noexcept;

    KRYS_NODISCARD CounterMaps &GetCounterMaps() noexcept
    {
      static CounterMaps staticCounterMaps;
      return staticCounterMaps;
    }

    KRYS_NODISCARD CounterNode *CounterFor(const RenderElement &renderer,
                                           const css::CSSOMStringAtom &identifier) noexcept
    {
      auto &maps = GetCounterMaps();
      auto map = maps.find(&renderer);
      if (map == maps.end())
      {
        return nullptr;
      }
      auto node = map->second->find(identifier);
      return node != map->second->end() ? node->second.get() : nullptr;
    }

    KRYS_NODISCARD CounterDirectives DirectivesFor(const RenderStyle &style,
                                                   const css::CSSOMStringAtom &identifier) noexcept
    {
      auto &map = style.GetCounterDirectives().Map;
      auto it = map.find(identifier);
      return it != map.end() ? it->second : CounterDirectives {};
    }

    KRYS_NODISCARD dom::Element *AncestorStyleContainmentObject(const dom::Element &element) noexcept
    {
      auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(element);
      dom::Element *ancestor = pseudoElement ? pseudoElement->HostElement() : element.ParentElement();
      while (ancestor)
      {
        if (auto *style = ancestor->ExistingComputedStyle())
        {
          if (style->UsedContain().Contains(style::ContainValue::Style))
          {
            break;
          }
        }
        // FIXME: this should use parentInComposedTree but for now matches the rest of RenderCounter.
        ancestor = ancestor->ParentElement();
      }
      return ancestor;
    }

    // This function processes the renderer tree in the order of the DOM tree
    // including pseudo elements as defined in CSS 2.1. This method will always
    // return either a previous element within the same contain: style scope or nullptr.
    KRYS_NODISCARD RenderElement *
      PreviousInPreOrderRespectingContainment(const RenderElement &renderer) noexcept
    {
      krys_debug_assert(renderer.GetElement());
      dom::Element *previous = dom::TreeQueries::PreviousElementIncludingPseudo(*renderer.GetElement());
      dom::Element *styleContainmentAncestor = AncestorStyleContainmentObject(*renderer.GetElement());

      while (previous)
      {
        while (previous && !previous->Renderer())
        {
          previous = dom::TreeQueries::PreviousElementIncludingPseudo(*previous, styleContainmentAncestor);
        }
        if (!previous)
        {
          break;
        }
        auto *previousStyleContainmentAncestor = AncestorStyleContainmentObject(*previous);
        // If the candidate's containment ancestor is the same as elements, then
        // that's a valid candidate.
        if (previousStyleContainmentAncestor == styleContainmentAncestor)
        {
          return previous->Renderer();
        }

        // If the candidate does have a containment ancestor, it could be
        // that we entered a new sub-containment. Try again starting from the
        // contain ancestor.
        previous = previousStyleContainmentAncestor;
      }
      return nullptr;
    }

    KRYS_NODISCARD dom::Element *ParentOrPseudoHostElement(const RenderElement &renderer) noexcept
    {
      if (renderer.IsPseudoElement())
      {
        return renderer.GeneratingElement();
      }
      return renderer.GetElement() ? renderer.GetElement()->ParentElement() : nullptr;
    }

    KRYS_NODISCARD dom::Element *PreviousSiblingOrParentElement(const dom::Element &element) noexcept
    {
      if (auto *previous = dom::TreeQueries::PseudoAwarePreviousElementSibling(element))
      {
        while (previous && !previous->Renderer())
        {
          previous = dom::TreeQueries::PseudoAwarePreviousElementSibling(*previous);
        }

        if (previous)
        {
          return previous;
        }
      }

      if (auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(element))
      {
        auto *hostElement = pseudoElement->HostElement();
        krys_debug_assert(hostElement);
        if (hostElement->Renderer())
        {
          return hostElement;
        }
        return PreviousSiblingOrParentElement(*hostElement);
      }

      auto *parent = element.ParentElement();
      if (parent && !parent->Renderer())
      {
        parent = PreviousSiblingOrParentElement(*parent);
      }
      if (parent && parent->Renderer()
          && parent->Renderer()->Style().UsedContain().Contains(style::ContainValue::Style))
      {
        return nullptr;
      }
      return parent;
    }

    // This function processes the renderer tree in the order of the DOM tree
    // including pseudo elements as defined in CSS 2.1.
    KRYS_NODISCARD RenderElement *PreviousSiblingOrParent(const RenderElement &renderer) noexcept
    {
      krys_debug_assert(renderer.GetElement());

      auto *previous = PreviousSiblingOrParentElement(*renderer.GetElement());
      return previous ? previous->Renderer() : nullptr;
    }

    KRYS_NODISCARD bool AreRenderersElementsSiblings(const RenderElement &first,
                                                     const RenderElement &second) noexcept
    {
      return ParentOrPseudoHostElement(first) == ParentOrPseudoHostElement(second);
    }

    // This function processes the renderer tree in the order of the DOM tree
    // including pseudo elements as defined in CSS 2.1.
    KRYS_NODISCARD RenderElement *NextInPreOrder(const RenderElement &renderer,
                                                 const dom::Element *stayWithin,
                                                 bool skipDescendants = false) noexcept
    {
      krys_debug_assert(renderer.GetElement());
      dom::Element &self = *renderer.GetElement();
      auto advance = [&](const dom::Element &current)
      {
        return skipDescendants
                 ? dom::TreeQueries::NextElementIncludingPseudoSkippingChildren(current, stayWithin)
                 : dom::TreeQueries::NextElementIncludingPseudo(current, stayWithin);
      };
      dom::Element *next = advance(self);
      while (next && !next->Renderer())
      {
        next = advance(*next);
      }
      return next ? next->Renderer() : nullptr;
    }

    KRYS_NODISCARD CounterDirectives ListItemCounterDirectives(RenderElement &renderer) noexcept
    {
      if (auto *item = DynamicDowncast<RenderListItem>(renderer))
      {
        return {
          .ResetValue = null,
          .IncrementValue = item->IsInReversedOrderedList() ? -1 : 1,
          .SetValue = null,
        };
      }
      if (auto *element = renderer.GetElement())
      {
        auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element);
        if (auto *list = htmlElement ? DynamicDowncast<html::HTMLOListElement>(*htmlElement) : nullptr)
        {
          return {
            .ResetValue = list->StartValue(),
            .IncrementValue = list->IsReversed() ? 1 : -1,
            .SetValue = null,
          };
        }
        if (IsHTMLListElement(*element))
        {
          return {
            .ResetValue = 0,
            .IncrementValue = null,
            .SetValue = null,
          };
        }
      }
      return {};
    }

    struct CounterPlan
    {
      EnumSet<CounterNode::Type> Type;
      int32 Value;
    };

    KRYS_NODISCARD Maybe<CounterPlan> PlanCounter(RenderElement &renderer,
                                                  const css::CSSOMStringAtom &identifier) noexcept
    {
      // We must have a generating node or else we cannot have a counter.
      dom::Element *generatingElement = renderer.GeneratingElement();
      if (!generatingElement)
      {
        return null;
      }

      const auto &style = renderer.Style();

      if (auto pseudoElementType = style.GetPseudoElementType())
      {
        switch (*pseudoElementType)
        {
          case PseudoElementType::Before:
          case PseudoElementType::After:  break;
          default:                        return null; // Counters are forbidden from all other pseudo elements.
        }
      }
      else
      {
        // Sometimes elements have more then one renderer. Only the first one gets the counter
        // LayoutTests/http/tests/css/counter-crash.html
        if (generatingElement->Renderer() != &renderer)
        {
          return null;
        }
      }

      auto directives = DirectivesFor(style, identifier);

      if (identifier == u8"list-item")
      {
        auto itemDirectives = ListItemCounterDirectives(renderer);
        if (!directives.ResetValue)
        {
          directives.ResetValue = itemDirectives.ResetValue;
        }
        if (!directives.IncrementValue)
        {
          directives.IncrementValue = itemDirectives.IncrementValue;
        }
        if (!directives.SetValue)
        {
          directives.SetValue = itemDirectives.SetValue;
        }
      }

      EnumSet<CounterNode::Type> type;

      if (directives.SetValue)
      {
        type.Add(CounterNode::Type::Set);
      }
      if (directives.ResetValue)
      {
        type.Add(CounterNode::Type::Reset);
      }
      if (directives.IncrementValue)
      {
        type.Add(CounterNode::Type::Increment);
      }

      if (directives.SetValue)
      {
        return CounterPlan {type, *directives.SetValue};
      }
      if (directives.ResetValue)
      {
        return CounterPlan {
          type, SaturatedSum<int32>(*directives.ResetValue, directives.IncrementValue.value_or(0))};
      }
      if (directives.IncrementValue)
      {
        return CounterPlan {type, *directives.IncrementValue};
      }
      return null;
    }

    // - Finds the insertion point for the counter described by counterOwner, isReset and
    // identifier in the CounterNode tree for identifier and sets parent and
    // previousSibling accordingly.
    // - The function returns true if the counter whose insertion point is searched is NOT
    // the root of the tree.
    // - The root of the tree is a counter reference that is not in the scope of any other
    // counter with the same identifier.
    // - All the counter references with the same identifier as this one that are in
    // children or subsequent siblings of the renderer that owns the root of the tree
    // form the rest of of the nodes of the tree.
    // - The root of the tree is always a reset type reference.
    // - A subtree rooted at any reset node in the tree is equivalent to all counter
    // references that are in the scope of the counter or nested counter defined by that
    // reset node.
    // - Non-reset CounterNodes cannot have descendants.

    struct CounterInsertionPoint
    {
      RefPtr<CounterNode> Parent;
      RefPtr<CounterNode> PreviousSibling;
    };

    KRYS_NODISCARD CounterInsertionPoint FindPlaceForCounter(RenderElement &counterOwner,
                                                             const css::CSSOMStringAtom &identifier,
                                                             EnumSet<CounterNode::Type> type) noexcept
    {
      // We cannot stop searching for counters with the same identifier before we also
      // check this renderer, because it may affect the positioning in the tree of our counter.
      RenderElement *searchEndRenderer = PreviousSiblingOrParent(counterOwner);
      // We check renderers in preOrder from the renderer that our counter is attached to
      // towards the begining of the document for counters with the same identifier as the one
      // we are trying to find a place for. This is the next renderer to be checked.
      RenderElement *currentRenderer = PreviousInPreOrderRespectingContainment(counterOwner);
      RefPtr<CounterNode> previousSibling;

      // Establish counter nodes previous to currentRenderer in order that calling
      // makeCounterNode on currentRenderer does not recurse into this function.
      if (currentRenderer && !currentRenderer->HasCounterNodeMap())
      {
        List<RenderElement *> previousRenderers;
        RenderElement *current = currentRenderer;
        while (current && !current->HasCounterNodeMap())
        {
          if (!current->Style().GetCounterDirectives().Map.empty())
          {
            previousRenderers.push_back(current);
          }
          current = PreviousInPreOrderRespectingContainment(*current);
        }
        while (!previousRenderers.empty())
        {
          auto *previousRenderer = previousRenderers.back();
          previousRenderers.pop_back();
          (void)MakeCounterNode(*previousRenderer, identifier, false);
        }
      }

      bool isReset = type.Contains(CounterNode::Type::Reset);
      while (currentRenderer)
      {
        auto currentCounter = ShareRefPtr(MakeCounterNode(*currentRenderer, identifier, false));
        if (searchEndRenderer == currentRenderer)
        {
          // We may be at the end of our search.
          if (currentCounter)
          {
            // We have a suitable counter on the EndSearchRenderer.
            if (previousSibling)
            {
              // But we already found another counter that we come after.
              if (currentCounter->ActsAsReset())
              {
                // We found a reset counter that is on a renderer that is a sibling of ours or a parent.
                if (isReset && AreRenderersElementsSiblings(*currentRenderer, counterOwner))
                {
                  // We are also a reset counter and the previous reset was on a sibling renderer
                  // hence we are the next sibling of that counter if that reset is not a root or
                  // we are a root node if that reset is a root.
                  auto parent = ShareRefPtr(currentCounter->Parent());
                  auto sibling = parent ? currentCounter : nullptr;
                  return {krys::move(parent), krys::move(sibling)};
                }
                // We are not a reset node or the previous reset must be on an ancestor of our owner renderer
                // hence we must be a child of that reset counter.
                // In some cases renders can be reparented (ex. nodes inside a table but not in a column or
                // row). In these cases the identified previousSibling will be invalid as its parent is
                // different from our identified parent.
                if (previousSibling->Parent() != currentCounter.get())
                {
                  previousSibling = nullptr;
                }
                return {currentCounter, krys::move(previousSibling)};
              }
              // CurrentCounter, the counter at the EndSearchRenderer, is not reset.
              if (!isReset || !AreRenderersElementsSiblings(*currentRenderer, counterOwner))
              {
                // If the node we are placing is not reset or we have found a counter that is attached
                // to an ancestor of the placed counter's owner renderer we know we are a sibling of that
                // node.
                if (currentCounter->Parent() != previousSibling->Parent())
                {
                  return {};
                }
                return {ShareRefPtr(currentCounter->Parent()), krys::move(previousSibling)};
              }
            }
            else
            {
              // We are at the potential end of the search, but we had no previous sibling candidate
              // In this case we follow pretty much the same logic as above but no ASSERTs about
              // previousSibling, and when we are a sibling of the end counter we must set previousSibling
              // to currentCounter.
              if (currentCounter->ActsAsReset())
              {
                if (isReset && AreRenderersElementsSiblings(*currentRenderer, counterOwner))
                {
                  return {ShareRefPtr(currentCounter->Parent()), currentCounter};
                }
                return {currentCounter, krys::move(previousSibling)};
              }
              if (!isReset || !AreRenderersElementsSiblings(*currentRenderer, counterOwner))
              {
                return {ShareRefPtr(currentCounter->Parent()), currentCounter};
              }
              previousSibling = currentCounter;
            }
          }
          // We come here if the previous sibling or parent of our owner renderer had no
          // good counter, or we are a reset node and the counter on the previous sibling
          // of our owner renderer was not a reset counter.
          // Set a new goal for the end of the search.
          searchEndRenderer = PreviousSiblingOrParent(*currentRenderer);
        }
        else
        {
          // We are searching descendants of a previous sibling of the renderer that the
          // counter being placed is attached to.
          if (currentCounter)
          {
            // We found a suitable counter.
            if (previousSibling)
            {
              // Since we had a suitable previous counter before, we should only consider this one as our
              // previousSibling if it is a reset counter and hence the current previousSibling is its child.
              if (currentCounter->ActsAsReset())
              {
                previousSibling = currentCounter;
                // We are no longer interested in previous siblings of the currentRenderer or their children
                // as counters they may have attached cannot be the previous sibling of the counter we are
                // placing.
                auto *parent = ParentOrPseudoHostElement(*currentRenderer);
                currentRenderer = parent ? parent->Renderer() : nullptr;
                continue;
              }
            }
            else
            {
              previousSibling = currentCounter;
            }
            currentRenderer = PreviousSiblingOrParent(*currentRenderer);
            continue;
          }
        }
        // This function is designed so that the same test is not done twice in an iteration, except for this
        // one which may be done twice in some cases. Rearranging the decision points though, to accommodate
        // this performance improvement would create more code duplication than is worthwhile in my oppinion
        // and may further impede the readability of this already complex algorithm.
        if (previousSibling)
        {
          currentRenderer = PreviousSiblingOrParent(*currentRenderer);
        }
        else
        {
          currentRenderer = PreviousInPreOrderRespectingContainment(*currentRenderer);
        }
      }
      return {};
    }

    CounterNode *MakeCounterNode(RenderElement &renderer, const css::CSSOMStringAtom &identifier,
                                 bool alwaysCreateCounter) noexcept
    {
      if (renderer.HasCounterNodeMap())
      {
        krys_debug_assert(GetCounterMaps().contains(&renderer));
        if (auto *node = CounterFor(renderer, identifier))
        {
          return node;
        }
      }

      auto plan = PlanCounter(renderer, identifier);
      if (!plan && !alwaysCreateCounter)
      {
        return nullptr;
      }

      auto &maps = GetCounterMaps();

      auto newNode = CounterNode::Create(renderer, plan ? plan->Type : EnumSet<CounterNode::Type> {},
                                         plan ? plan->Value : 0);

      auto place =
        FindPlaceForCounter(renderer, identifier, plan ? plan->Type : EnumSet<CounterNode::Type> {});
      if (place.Parent)
      {
        place.Parent->InsertAfter(*newNode, place.PreviousSibling.get(), identifier);
      }

      auto &map = maps[&renderer];
      if (!map)
      {
        map = CreateUnique<CounterMap>();
      }
      map->insert_or_assign(identifier, newNode);
      renderer.SetHasCounterNodeMap(true);

      if (newNode->Parent() || renderer.ShouldApplyStyleContainment())
      {
        return newNode.get();
      }

      // Check if some nodes that were previously root nodes should become children of this node now.
      auto *currentRenderer = &renderer;
      auto *stayWithin = ParentOrPseudoHostElement(renderer);
      bool skipDescendants = false;
      while ((currentRenderer = NextInPreOrder(*currentRenderer, stayWithin, skipDescendants)))
      {
        skipDescendants = currentRenderer->ShouldApplyStyleContainment();
        if (!currentRenderer->HasCounterNodeMap())
        {
          continue;
        }
        auto currentCounter = ShareRefPtr(CounterFor(*currentRenderer, identifier));
        if (!currentCounter)
        {
          continue;
        }
        skipDescendants = true;
        if (currentCounter->Parent())
        {
          continue;
        }
        if (stayWithin == ParentOrPseudoHostElement(*currentRenderer) && currentCounter->HasResetType())
        {
          break;
        }
        newNode->InsertAfter(*currentCounter, newNode->LastChild(), identifier);
      }

      return newNode.get();
    }

    void DestroyCounterNodeWithoutMapRemoval(const css::CSSOMStringAtom &identifier,
                                             CounterNode &node) noexcept
    {
      RefPtr<CounterNode> previous;
      for (auto child = ShareRefPtr(node.LastDescendant()); child && child != &node;
           child = krys::move(previous))
      {
        previous = ShareRefPtr(child->PreviousInPreOrder());
        child->Parent()->RemoveChild(*child);
        auto &childMap = *GetCounterMaps().find(&child->Owner())->second;
        krys_debug_assert(childMap.find(identifier)->second.get() == child.get());
        childMap.erase(identifier);
      }
      if (auto *parent = node.Parent())
      {
        parent->RemoveChild(node);
      }
    }
  }

  RenderCounter::RenderCounter(dom::Document &document, const style::Content::Counter &counter) noexcept
      : RenderText(Type::Counter, document, u8""), _counter(counter)
  {
    krys_debug_assert(IsRenderCounter());
    View().AddCounterNeedingUpdate(*this);
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderCounter::~RenderCounter() noexcept = default;

  void RenderCounter::WillBeDestroyed() noexcept
  {
    if (_counterNode)
    {
      _counterNode->RemoveRenderer(*this);
      krys_debug_assert(!_counterNode);
    }

    RenderText::WillBeDestroyed();
  }

  utf8_stringview RenderCounter::RenderName() const noexcept
  {
    return u8"RenderCounter";
  }

  utf8_string RenderCounter::OriginalText() const noexcept
  {
    if (!_counterNode)
    {
      return {};
    }

    auto child = ShareRefPtr(_counterNode.get());
    int32 value = child->ActsAsReset() ? child->Value() : child->CountInParent();

    auto counterText = [&](int32 value)
    {
      return GetCounterStyle()->Text(value, Style().GetWritingMode());
    };
    auto text = counterText(value);
    if (_counter.Separator != css::CSSOMStringAtom::Null())
    {
      if (!child->ActsAsReset())
      {
        child = ShareRefPtr(child->Parent());
      }
      while (CounterNode *parent = child->Parent())
      {
        StringBuilder builder;
        builder.Append(counterText(child->CountInParent()), _counter.Separator, text);
        text = krys::move(builder).ToString();
        child = ShareRefPtr(parent);
      }
    }

    return text;
  }

  void RenderCounter::UpdateCounter() noexcept
  {
    if (!_counterNode)
    {
      RenderElement *beforeAfterContainer = Parent();
      while (true)
      {
        if (!beforeAfterContainer)
        {
          return;
        }
        if (!beforeAfterContainer->IsAnonymous() && !beforeAfterContainer->IsPseudoElement())
        {
          return;
        }
        auto containerStyle = beforeAfterContainer->Style().GetPseudoElementType();
        if (containerStyle == PseudoElementType::Before || containerStyle == PseudoElementType::After)
        {
          break;
        }
        beforeAfterContainer = beforeAfterContainer->Parent();
      }
      MakeCounterNode(*beforeAfterContainer, _counter.Identifier, true)->AddRenderer(*this);
    }

    SetText(OriginalText(), true);
  }

  void RenderCounter::DestroyCounterNodes(RenderElement &owner) noexcept
  {
    krys_debug_assert(owner.HasCounterNodeMap());
    auto &maps = GetCounterMaps();
    auto it = maps.find(&owner);
    krys_debug_assert(it != maps.end());
    auto counterMap = krys::move(it->second);
    maps.erase(it);
    for (auto &[identifier, node] : *counterMap)
    {
      DestroyCounterNodeWithoutMapRemoval(identifier, *node);
    }
    owner.SetHasCounterNodeMap(false);
  }

  void RenderCounter::DestroyCounterNode(RenderElement &owner,
                                         const css::CSSOMStringAtom &identifier) noexcept
  {
    auto &maps = GetCounterMaps();
    auto map = maps.find(&owner);
    if (map == maps.end())
    {
      return;
    }
    auto entry = map->second->find(identifier);
    if (entry == map->second->end())
    {
      return;
    }
    auto node = krys::move(entry->second);
    map->second->erase(entry);
    DestroyCounterNodeWithoutMapRemoval(identifier, *node);
    // We do not delete the map here even if it is now empty because we expect to
    // reuse it. In order for a renderer to lose all its counters permanently,
    // a style change for the renderer involving removal of all counter
    // directives must occur, in which case, RenderCounter::destroyCounterNodes()
    // will be called.
  }

  void RenderCounter::RendererStyleChanged(RenderElement &renderer, const RenderStyle *oldStyle,
                                           const RenderStyle &newStyle) noexcept
  {
    if ((!oldStyle || oldStyle->GetCounterDirectives().Map.empty())
        && newStyle.GetCounterDirectives().Map.empty())
    {
      return;
    }

    RendererStyleChangedSlowCase(renderer, oldStyle, newStyle);
  }

  void RenderCounter::RendererStyleChangedSlowCase(RenderElement &renderer, const RenderStyle *oldStyle,
                                                   const RenderStyle &newStyle) noexcept
  {
    dom::Element *element = renderer.GeneratingElement();
    if (!element || !element->Renderer())
    {
      return; // cannot have generated content or if it can have, it will be handled during attaching
    }

    const CounterDirectiveMap *oldCounterDirectives;
    if (oldStyle && !(oldCounterDirectives = &oldStyle->GetCounterDirectives())->Map.empty())
    {
      if (auto &newCounterDirectives = newStyle.GetCounterDirectives().Map; !newCounterDirectives.empty())
      {
        for (auto &[key, value] : newCounterDirectives)
        {
          auto existingEntry = oldCounterDirectives->Map.find(key);
          if (existingEntry != oldCounterDirectives->Map.end())
          {
            if (existingEntry->second == value)
            {
              continue;
            }
            RenderCounter::DestroyCounterNode(renderer, key);
          }
          // We must create this node here, because the changed node may be a node with no display such as
          // as those created by the increment or reset directives and the re-layout that will happen will
          // not catch the change if the node had no children.
          (void)MakeCounterNode(renderer, key, false);
        }
        // Destroying old counters that do not exist in the new counterDirective map.
        for (auto &[key, value] : oldCounterDirectives->Map)
        {
          if (!newCounterDirectives.contains(key))
          {
            RenderCounter::DestroyCounterNode(renderer, key);
          }
        }
      }
      else
      {
        if (renderer.HasCounterNodeMap())
        {
          RenderCounter::DestroyCounterNodes(renderer);
        }
      }
    }
    else
    {
      if (renderer.HasCounterNodeMap())
      {
        RenderCounter::DestroyCounterNodes(renderer);
      }

      for (auto &[key, value] : newStyle.GetCounterDirectives().Map)
      {
        // We must create this node here, because the added node may be a node with no display such as
        // as those created by the increment or reset directives and the re-layout that will happen will
        // not catch the change if the node had no children.
        (void)MakeCounterNode(renderer, key, false);
      }
    }
  }

  Ref<css::CounterStyle> RenderCounter::GetCounterStyle() const noexcept
  {
    return GetDocument().StyleScope().GetCounterStyleRegistry().ResolvedCounterStyle(_counter.Style);
  }
}
