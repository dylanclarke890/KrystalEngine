#pragma once

#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Invalidator.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  /// @brief Invalidates the style that a change to a container's children affects (`:has()`, `:empty`,
  /// positional pseudo-classes and sibling combinators), before the change (when constructed) and after it
  /// (when destroyed).
  class ChildChangeInvalidation
  {
    KRYS_NON_COPY_MOVABLE(ChildChangeInvalidation);

    using MatchingHasSelectors = HashSet<const css::Selector *>;

    enum class ChangedElementRelation : uint8
    {
      SelfOrDescendant,
      Sibling
    };

  private:
    RefPtr<dom::Element> _parentElement;
    const dom::ContainerNode::ChildChange &_childChange;

    const bool _isEnabled;
    const bool _needsHasInvalidation;
    const bool _wasEmpty;

  public:
    ChildChangeInvalidation(dom::ContainerNode &container,
                            const dom::ContainerNode::ChildChange &childChange) noexcept;
    ~ChildChangeInvalidation() noexcept;

    static void InvalidateAfterFinishedParsingChildren(dom::Element &parent) noexcept;

  private:
    void InvalidateForHasBeforeMutation() noexcept;
    void InvalidateForHasAfterMutation() noexcept;
    void InvalidateAfterChange() noexcept;
    void CheckForSiblingStyleChanges() noexcept;
    void InvalidateForChangedElement(dom::Element &changedElement, MatchingHasSelectors &matchingHasSelectors,
                                     ChangedElementRelation changedElementRelation) noexcept;
    void InvalidateForChangeOutsideHasScope() noexcept;

    template <typename TFunction>
    void TraverseRemovedElements(TFunction &&function) noexcept;
    template <typename TFunction>
    void TraverseAddedElements(TFunction &&function) noexcept;
    template <typename TFunction>
    void TraverseRemainingExistingSiblings(TFunction &&function) noexcept;

    KRYS_NODISCARD dom::Element &ParentElement() noexcept
    {
      return *_parentElement;
    }
  };

  inline ChildChangeInvalidation::ChildChangeInvalidation(
    dom::ContainerNode &container, const dom::ContainerNode::ChildChange &childChange) noexcept
      : _parentElement(ShareRefPtr(DynamicDowncast<dom::Element>(container))), _childChange(childChange),
        _isEnabled(_parentElement && _parentElement->NeedsStyleInvalidation()),
        _needsHasInvalidation(_isEnabled && Scope::ForNode(*_parentElement).UsesHasPseudoClass()),
        _wasEmpty(!container.FirstChild())
  {
    if (!_isEnabled)
    {
      return;
    }

    if (_needsHasInvalidation)
    {
      InvalidateForHasBeforeMutation();
    }
  }

  inline ChildChangeInvalidation::~ChildChangeInvalidation() noexcept
  {
    if (!_isEnabled)
    {
      return;
    }

    if (_needsHasInvalidation)
    {
      InvalidateForHasAfterMutation();
    }

    InvalidateAfterChange();
  }
}
