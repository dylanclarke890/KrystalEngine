#include "Krystal.Booey/CSS/StyleAttributeMutationScope.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  namespace
  {
    uint32 scopeCount = 0;
    StyleAttributeMutationScope *currentScope = nullptr;
  }

  StyleAttributeMutationScope::StyleAttributeMutationScope(dom::Element *element) noexcept
      : _element(ShareRefPtr(element))
  {
    ++scopeCount;

    if (scopeCount != 1)
    {
      return;
    }

    krys_debug_assert(!currentScope);
    currentScope = this;

    if (!_element)
    {
      return;
    }

    // TODO(impl): CUSTOM-ELEMENT - also read the old value when the element is a defined custom element that
    // observes the style attribute, and enqueue its attributeChangedCallback when the scope ends.
    if (dom::MutationObserverAlgorithms::IsAttributeOldValueRequested(*_element, Attributes::Style))
    {
      _oldValue = _element->GetAttribute(Attributes::Style);
    }
  }

  StyleAttributeMutationScope::~StyleAttributeMutationScope() noexcept
  {
    --scopeCount;
    if (scopeCount)
    {
      return;
    }
    krys_debug_assert(currentScope == this);
    currentScope = nullptr;

    if (!_shouldDeliver || !_element)
    {
      return;
    }

    dom::MutationObserverAlgorithms::QueueMutationRecord(
      dom::MutationRecordType::Attributes, ShareRef(*_element), Attributes::Style, dom::DOMStringAtom::Null(),
      _oldValue ? dom::DOMStringAtom(*_oldValue) : dom::DOMStringAtom::Null(), {}, {}, nullptr, nullptr);
  }

  void StyleAttributeMutationScope::EnqueueMutationRecord() noexcept
  {
    currentScope->_shouldDeliver = true;
  }
}
