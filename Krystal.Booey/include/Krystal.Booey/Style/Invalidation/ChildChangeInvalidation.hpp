/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Style/Invalidation/Invalidator.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
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
