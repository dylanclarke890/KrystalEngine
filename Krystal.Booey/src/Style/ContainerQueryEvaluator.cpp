#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Update.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo::style
{
  ContainerQueryEvaluator::ContainerQueryEvaluator(const dom::Element &element, SelectionMode selectionMode,
                                                   ScopeOrdinal scopeOrdinal,
                                                   ContainerQueryEvaluationState *evaluationState) noexcept
      : _element(ShareRef(element)), _selectionMode(selectionMode), _scopeOrdinal(scopeOrdinal),
        _evaluationState(evaluationState)
  {
  }

  bool ContainerQueryEvaluator::Evaluate(const css::cq::ContainerQuery &containerQuery) const noexcept
  {
    auto context = FeatureEvaluationContextForQuery(containerQuery);
    if (!context)
    {
      return false;
    }

    if (containerQuery.Condition.Queries.empty() && !containerQuery.Name.View().empty())
    {
      return true;
    }

    return EvaluateCondition(containerQuery.Condition, *context) == css::mq::EvaluationResult::True;
  }

  static const RenderStyle *StyleForContainer(const dom::Element &container, css::cq::Axis requiredAxes,
                                              const ContainerQueryEvaluationState *evaluationState) noexcept
  {
    // Any element can be a style container and we haven't necessarily committed the style to render tree yet.
    // Look it up from the currently computed style update instead.
    if (requiredAxes == css::cq::Axis::None && evaluationState && evaluationState->StyleUpdate)
    {
      return evaluationState->StyleUpdate->ElementStyle(container);
    }

    return container.ExistingComputedStyle();
  }

  Maybe<css::mq::FeatureEvaluationContext> ContainerQueryEvaluator::FeatureEvaluationContextForQuery(
    const css::cq::ContainerQuery &containerQuery) const noexcept
  {
    // "For each element, the query container to be queried is selected from among the element’s
    // ancestor query containers that have a valid container-type for all the container features
    // in the <container-condition>. The optional <container-name> filters the set of query containers
    // considered to just those with a matching query container name."
    // https://drafts.csswg.org/css-contain-3/#container-rule

    // "If the <container-query> contains unknown or unsupported container features, no query container will
    // be selected."
    if (containerQuery.ContainsUnknownFeature == css::cq::ContainsUnknownFeature::Yes)
    {
      return null;
    }

    auto container =
      SelectContainer(containerQuery.RequiredAxes, css::CSSOMString(containerQuery.Name.View()), *_element,
                      _selectionMode, _scopeOrdinal, _evaluationState);
    if (!container)
    {
      return null;
    }

    auto *containerStyle = StyleForContainer(*container, containerQuery.RequiredAxes, _evaluationState);
    if (!containerStyle)
    {
      return null;
    }

    auto containerParent = ShareRefPtr<const dom::Element>(container->ParentElementInComposedTree());
    auto *containerParentStyle =
      containerParent ? StyleForContainer(*containerParent, containerQuery.RequiredAxes, _evaluationState)
                      : containerStyle;

    auto &document = _element->NodeDocument();
    auto *rootStyle = document.DocumentElement()->GetRenderStyle();

    // FIXME: The container's principal box (`container->renderer()`), once there's a render tree.
    return css::mq::FeatureEvaluationContext {
      WeakRef<const dom::Document>::WithRef(document),
      css::ToLengthConversionData {*containerStyle, rootStyle, containerParentStyle, RenderViewOf(document),
                                   container.get()},
      nullptr};
  }

  RefPtr<const dom::Element>
    ContainerQueryEvaluator::SelectContainer(css::cq::Axis requiredAxes, const css::CSSOMString &name,
                                             const dom::Element &element, SelectionMode selectionMode,
                                             ScopeOrdinal scopeOrdinal,
                                             const ContainerQueryEvaluationState *evaluationState) noexcept
  {
    // "For each element, the query container to be queried is selected from among the element’s
    // ancestor query containers that have a valid container-type for all the container features
    // in the <container-condition>. The optional <container-name> filters the set of query containers
    // considered to just those with a matching query container name."
    // https://drafts.csswg.org/css-contain-3/#container-rule

    auto isValidContainerForRequiredAxes = [&](ContainerType containerType, const RenderElement *principalBox)
    {
      // Any container is valid for style queries.
      if (requiredAxes == css::cq::Axis::None)
      {
        return true;
      }

      switch (containerType)
      {
        case ContainerType::Size: return true;
        case ContainerType::InlineSize:
          // Without a principal box the container matches but the query against it will evaluate to Unknown.
          if (!principalBox)
          {
            return true;
          }
          // FIXME: Check the principal box's writing mode (`isHorizontalWritingMode`), once there's a render
          // tree.
          krys_unreachable();
        case ContainerType::Normal: return false;
      }
      krys_unreachable();
    };

    auto isContainerForQuery =
      [&](const dom::Element &candidateElement, const dom::Element *originatingElement = nullptr)
    {
      auto *style = StyleForContainer(candidateElement, requiredAxes, evaluationState);
      if (!style)
      {
        return false;
      }
      // FIXME: The candidate's renderer, once there's a render tree.
      if (!isValidContainerForRequiredAxes(style->GetContainerType(), nullptr))
      {
        return false;
      }
      if (name.empty())
      {
        return true;
      }

      return std::ranges::any_of(style->GetContainerNames(),
                                 [&](auto &scopedName)
                                 {
                                   auto isNameFromAllowedScope = [&](auto &scopedName)
                                   {
                                     // Names from :host rules are allowed when the candidate is the host
                                     // element.
                                     auto *host = originatingElement
                                                    ? dom::TreeQueries::ShadowHost(*originatingElement)
                                                    : dom::TreeQueries::ShadowHost(element);
                                     auto isHost = host == &candidateElement;
                                     if (scopedName.ScopeOrdinal == ScopeOrdinal::Shadow && isHost)
                                     {
                                       return true;
                                     }
                                     // Otherwise names from the inner scopes are ignored.
                                     return scopedName.ScopeOrdinal <= ScopeOrdinal::Element;
                                   };
                                   return isNameFromAllowedScope(scopedName)
                                          && scopedName.Name.View() == name;
                                 });
    };

    auto findOriginatingElement = [&]() -> const dom::Element *
    {
      if (selectionMode == SelectionMode::PseudoElement)
      {
        return &element;
      }

      // ::part() selectors query the composed tree
      if (selectionMode == SelectionMode::PartPseudoElement)
      {
        return dom::SlotAlgorithms::GetAssignedSlot(element);
      }

      // ::slotted() selectors can query containers inside the shadow tree, including the slot itself.
      if (scopeOrdinal >= ScopeOrdinal::FirstSlot && scopeOrdinal <= ScopeOrdinal::SlotLimit)
      {
        return AssignedSlotForScopeOrdinal(element, scopeOrdinal);
      }

      if (scopeOrdinal == ScopeOrdinal::Element && dom::SlotAlgorithms::GetAssignedSlot(element))
      {
        return dom::SlotAlgorithms::GetAssignedSlot(element);
      }

      return nullptr;
    };

    if (auto originatingElement = ShareRefPtr(findOriginatingElement()))
    {
      // For the ::part() and ::slotted() pseudo-element selectors, which represent real elements in the DOM
      // tree, query containers can be established by flat tree ancestors of those elements. For other
      // pseudo-elements, query containers can be established by inclusive flat tree ancestors of their
      // originating element. https://drafts.csswg.org/css-conditional-5/#container-queries
      for (auto ancestor = originatingElement; ancestor;
           ancestor = ShareRefPtr<const dom::Element>(ancestor->ParentElementInComposedTree()))
      {
        if (isContainerForQuery(*ancestor, originatingElement.get()))
        {
          return ancestor;
        }
      }
      return nullptr;
    }

    if (evaluationState && requiredAxes != css::cq::Axis::None)
    {
      for (auto &container : evaluationState->SizeQueryContainers | std::views::reverse)
      {
        if (isContainerForQuery(*container))
        {
          return container;
        }
      }
      return {};
    }

    for (auto ancestor = ShareRefPtr<const dom::Element>(element.ParentElementInComposedTree()); ancestor;
         ancestor = ShareRefPtr<const dom::Element>(ancestor->ParentElementInComposedTree()))
    {
      if (isContainerForQuery(*ancestor))
      {
        return ancestor;
      }
    }
    return {};
  }
}
