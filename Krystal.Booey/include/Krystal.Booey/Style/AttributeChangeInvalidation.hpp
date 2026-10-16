#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Invalidator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  /// @brief Invalidates the style that a change to one of an element's attributes affects, before the change
  /// (when constructed) and after it (when destroyed).
  class AttributeChangeInvalidation
  {
    KRYS_NON_COPY_MOVABLE(AttributeChangeInvalidation);

  private:
    const bool _isEnabled;
    const Ref<dom::Element> _element;

    Invalidator::MatchElementRuleSets _matchElementRuleSets;

  public:
    /// @param oldValue The value before the change, or null if the attribute is being added.
    /// @param newValue The value after the change, or null if the attribute is being removed.
    AttributeChangeInvalidation(dom::Element &element, const dom::QualifiedName &attributeName,
                                Maybe<dom::DOMStringView> oldValue,
                                Maybe<dom::DOMStringView> newValue) noexcept;
    ~AttributeChangeInvalidation() noexcept;

  private:
    void InvalidateStyle(const dom::QualifiedName &attributeName, Maybe<dom::DOMStringView> oldValue,
                         Maybe<dom::DOMStringView> newValue) noexcept;
    void InvalidateStyleWithRuleSets() noexcept;
  };

  inline AttributeChangeInvalidation::AttributeChangeInvalidation(dom::Element &element,
                                                                  const dom::QualifiedName &attributeName,
                                                                  Maybe<dom::DOMStringView> oldValue,
                                                                  Maybe<dom::DOMStringView> newValue) noexcept
      : _isEnabled(element.NeedsStyleInvalidation()), _element(ShareRef(element))
  {
    if (!_isEnabled)
    {
      return;
    }
    InvalidateStyle(attributeName, oldValue, newValue);
    InvalidateStyleWithRuleSets();
  }

  inline AttributeChangeInvalidation::~AttributeChangeInvalidation() noexcept
  {
    if (!_isEnabled)
    {
      return;
    }
    InvalidateStyleWithRuleSets();
  }
}
