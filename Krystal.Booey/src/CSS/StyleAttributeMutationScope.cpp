#include "Krystal.Booey/CSS/StyleAttributeMutationScope.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
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

    bool shouldReadOldValue =
      dom::MutationObserverAlgorithms::IsAttributeOldValueRequested(*_element, Attributes::Style);

    if (_element->IsDefinedCustomElement()) KRYS_UNLIKELY
    {
      auto *reactionQueue = _element->ReactionQueue();
      if (reactionQueue && reactionQueue->ObservesStyleAttribute())
      {
        _isCustomElement = true;
        shouldReadOldValue = true;
      }
    }

    if (shouldReadOldValue)
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
      dom::MutationRecordType::Attributes, _element, Attributes::Style, dom::DOMStringAtom::Null(),
      _oldValue ? dom::DOMStringAtom(*_oldValue) : dom::DOMStringAtom::Null(), {}, {}, nullptr, nullptr);

    if (_isCustomElement)
    {
      auto newValue = _element->GetAttribute(Attributes::Style);
      html::CustomElementReactionQueue::EnqueueAttributeChangedCallbackIfNeeded(
        *_element,
        dom::QualifiedName(dom::DOMStringAtom::Null(), dom::DOMStringAtom::Null(), Attributes::Style),
        _oldValue ? dom::DOMStringAtom(*_oldValue) : dom::DOMStringAtom::Null(),
        newValue ? dom::DOMStringAtom(*newValue) : dom::DOMStringAtom::Null());
    }
  }

  void StyleAttributeMutationScope::EnqueueMutationRecord() noexcept
  {
    currentScope->_shouldDeliver = true;
  }
}
