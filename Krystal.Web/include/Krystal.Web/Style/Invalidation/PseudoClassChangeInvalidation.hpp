/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Web/Css/Selectors/SelectorEnums.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Style/Invalidation/Invalidator.hpp"
#include "Krystal.Web/Style/Validity.hpp"
#include <initializer_list>

namespace krys::web::style
{
  /// @brief Invalidates the style that a change to one of an element's pseudo-class states affects, before
  /// the change (when constructed) and after it (when destroyed).
  class PseudoClassChangeInvalidation
  {
    KRYS_NON_COPY_MOVABLE(PseudoClassChangeInvalidation);

  public:
    enum AnyValueTag
    {
      AnyValue,
    };

  private:
    enum class Value : uint8
    {
      False,
      True,
      Any,
    };

    const bool _isEnabled;
    Element &_element;

    Invalidator::MatchElementRuleSets _beforeChangeRuleSets;
    Invalidator::MatchElementRuleSets _afterChangeRuleSets;

  public:
    /// @param value The state after the change.
    PseudoClassChangeInvalidation(Element &element, css::SelectorPseudoClass pseudoClass, bool value,
                                  InvalidationScope invalidationScope = InvalidationScope::All) noexcept;

    /// @brief For a change whose state after the change isn't known: invalidates both ways.
    PseudoClassChangeInvalidation(Element &element, css::SelectorPseudoClass pseudoClass,
                                  AnyValueTag) noexcept;

    PseudoClassChangeInvalidation(
      Element &element, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> pseudoClasses) noexcept;

    ~PseudoClassChangeInvalidation() noexcept;

  private:
    void ComputeInvalidation(css::SelectorPseudoClass pseudoClass, Value value,
                             InvalidationScope invalidationScope) noexcept;

    void CollectRuleSets(const PseudoClassInvalidationKey &key, Value value,
                         InvalidationScope invalidationScope) noexcept;

    void InvalidateBeforeChange() noexcept;

    void InvalidateAfterChange() noexcept;
  };

  /// @brief The keys of the invalidation rule sets for @p pseudoClass that may match @p element.
  KRYS_NODISCARD SmallList<PseudoClassInvalidationKey, 4>
    MakePseudoClassInvalidationKeys(css::SelectorPseudoClass pseudoClass, const Element &element) noexcept;

  inline PseudoClassChangeInvalidation::PseudoClassChangeInvalidation(
    Element &element, css::SelectorPseudoClass pseudoClass, bool value,
    InvalidationScope invalidationScope) noexcept
      : _isEnabled(element.NeedsStyleInvalidation()), _element(element)
  {
    if (!_isEnabled)
    {
      return;
    }
    ComputeInvalidation(pseudoClass, value ? Value::True : Value::False, invalidationScope);
    InvalidateBeforeChange();
  }

  inline PseudoClassChangeInvalidation::PseudoClassChangeInvalidation(Element &element,
                                                                      css::SelectorPseudoClass pseudoClass,
                                                                      AnyValueTag) noexcept
      : _isEnabled(element.NeedsStyleInvalidation()), _element(element)
  {
    if (!_isEnabled)
    {
      return;
    }
    ComputeInvalidation(pseudoClass, Value::Any, InvalidationScope::All);
    InvalidateBeforeChange();
  }

  inline PseudoClassChangeInvalidation::PseudoClassChangeInvalidation(
    Element &element, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> pseudoClasses) noexcept
      : _isEnabled(element.NeedsStyleInvalidation()), _element(element)
  {
    if (!_isEnabled)
    {
      return;
    }
    for (auto &pseudoClass : pseudoClasses)
    {
      ComputeInvalidation(pseudoClass.first, pseudoClass.second ? Value::True : Value::False,
                          InvalidationScope::All);
    }
    InvalidateBeforeChange();
  }

  inline PseudoClassChangeInvalidation::~PseudoClassChangeInvalidation() noexcept
  {
    if (!_isEnabled)
    {
      return;
    }
    InvalidateAfterChange();
  }
}
