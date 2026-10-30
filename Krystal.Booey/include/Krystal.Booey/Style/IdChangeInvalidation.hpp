#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Style/Invalidator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::style
{
  /// @brief Invalidates the style that a change to an element's ID affects, before the change (when
  /// constructed) and after it (when destroyed).
  class IdChangeInvalidation
  {
    KRYS_NON_COPY_MOVABLE(IdChangeInvalidation);

  private:
    const bool _isEnabled;
    const Ref<dom::Element> _element;

    dom::DOMStringAtom _newId {dom::DOMStringAtom::Null()};

    Invalidator::MatchElementRuleSets _matchElementRuleSets;

  public:
    IdChangeInvalidation(dom::Element &element, const dom::DOMStringAtom &oldId,
                         const dom::DOMStringAtom &newId) noexcept;
    ~IdChangeInvalidation() noexcept;

  private:
    void InvalidateStyle(const dom::DOMStringAtom &changedId) noexcept;
    void InvalidateStyleWithRuleSets() noexcept;
  };

  inline IdChangeInvalidation::IdChangeInvalidation(dom::Element &element, const dom::DOMStringAtom &oldId,
                                                    const dom::DOMStringAtom &newId) noexcept
      : _isEnabled(element.NeedsStyleInvalidation()), _element(ShareRef(element))
  {
    if (!_isEnabled)
    {
      return;
    }
    if (oldId == newId)
    {
      return;
    }
    _newId = newId;

    InvalidateStyle(oldId);
    InvalidateStyleWithRuleSets();
  }

  inline IdChangeInvalidation::~IdChangeInvalidation() noexcept
  {
    if (!_isEnabled)
    {
      return;
    }
    InvalidateStyle(_newId);
    InvalidateStyleWithRuleSets();
  }
}
