#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Invalidator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::style
{
  /// @brief Invalidates the style that a change to an element's classes affects, before the change (when
  /// constructed) and after it (when destroyed).
  class ClassChangeInvalidation
  {
    KRYS_NON_COPY_MOVABLE(ClassChangeInvalidation);

  private:
    const bool _isEnabled;
    const Ref<dom::Element> _element;

    Invalidator::MatchElementRuleSets _beforeChangeRuleSets;
    Invalidator::MatchElementRuleSets _afterChangeRuleSets;

  public:
    ClassChangeInvalidation(dom::Element &element, Span<const dom::DOMStringAtom> oldClasses,
                            Span<const dom::DOMStringAtom> newClasses) noexcept;
    ~ClassChangeInvalidation() noexcept;

  private:
    void ComputeInvalidation(Span<const dom::DOMStringAtom> oldClasses,
                             Span<const dom::DOMStringAtom> newClasses) noexcept;
    void InvalidateBeforeChange() noexcept;
    void InvalidateAfterChange() noexcept;
  };

  inline ClassChangeInvalidation::ClassChangeInvalidation(dom::Element &element,
                                                          Span<const dom::DOMStringAtom> oldClasses,
                                                          Span<const dom::DOMStringAtom> newClasses) noexcept
      : _isEnabled(element.NeedsStyleInvalidation()), _element(ShareRef(element))
  {
    if (!_isEnabled)
    {
      return;
    }
    ComputeInvalidation(oldClasses, newClasses);
    InvalidateBeforeChange();
  }

  inline ClassChangeInvalidation::~ClassChangeInvalidation() noexcept
  {
    if (!_isEnabled)
    {
      return;
    }
    InvalidateAfterChange();
  }
}
