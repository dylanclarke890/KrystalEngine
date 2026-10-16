#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Invalidator.hpp"
#include "Krystal.Booey/Style/Validity.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <initializer_list>

namespace krys::boo::style
{
  /// @brief Invalidates the style that a change to one of an element's pseudo-class states affects, before
  /// the change (when constructed) and after it (when destroyed).
  class PseudoClassChangeInvalidation
  {
    KRYS_NON_COPY_MOVABLE(PseudoClassChangeInvalidation);

  public:
    enum AnyValueTag
    {
      AnyValue
    };

  private:
    enum class Value : uint8
    {
      False,
      True,
      Any
    };

    const bool _isEnabled;
    dom::Element &_element;

    Invalidator::MatchElementRuleSets _beforeChangeRuleSets;
    Invalidator::MatchElementRuleSets _afterChangeRuleSets;

  public:
    /// @param value The state after the change.
    PseudoClassChangeInvalidation(dom::Element &element, css::SelectorPseudoClass pseudoClass, bool value,
                                  InvalidationScope invalidationScope = InvalidationScope::All) noexcept;
    /// @brief For a change whose state after the change isn't known: invalidates both ways.
    PseudoClassChangeInvalidation(dom::Element &element, css::SelectorPseudoClass pseudoClass,
                                  AnyValueTag) noexcept;
    PseudoClassChangeInvalidation(
      dom::Element &element,
      std::initializer_list<Pair<css::SelectorPseudoClass, bool>> pseudoClasses) noexcept;

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
    MakePseudoClassInvalidationKeys(css::SelectorPseudoClass pseudoClass,
                                    const dom::Element &element) noexcept;

  inline PseudoClassChangeInvalidation::PseudoClassChangeInvalidation(
    dom::Element &element, css::SelectorPseudoClass pseudoClass, bool value,
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

  inline PseudoClassChangeInvalidation::PseudoClassChangeInvalidation(dom::Element &element,
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
    dom::Element &element, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> pseudoClasses) noexcept
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
