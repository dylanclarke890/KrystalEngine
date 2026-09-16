#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::dom
{
  DOMTokenList::DOMTokenList(Element &element, DOMStringAtom attributeName,
                             IsSupportedTokenFunction &&isSupportedToken) noexcept
      : _attributeName {attributeName}, _element {ShareCheckedRef(element)},
        _isSupportedToken {krys::move(isSupportedToken)}
  {
    auto value = ElementAlgorithms::GetAttributeValue(*_element, _attributeName);
    auto tokens = OrderedSet::Parser(value);
    _tokens.append(tokens.begin(), tokens.end());
  }

  void DOMTokenList::Value(DOMString &&value) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*_element, _attributeName, krys::move(value));
  }

  ExceptionOr<bool> DOMTokenList::ValidationSteps(DOMStringView token) const noexcept
  {
    if (!_isSupportedToken)
    {
      return Exception {ExceptionCode::TypeError};
    }

    auto lowercaseToken = krys::text::ToASCIILower(token);

    return _isSupportedToken(_element->NodeDocument(), lowercaseToken);
  }

  void DOMTokenList::UpdateSteps() noexcept
  {
    auto attr = ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom::Null(), _attributeName, *_element);
    if (attr == nullptr && _tokens.empty())
    {
      return;
    }

    ElementAlgorithms::SetAttributeValue(*_element, _attributeName, OrderedSet::Serializer(_tokens));
  }

  DOMString DOMTokenList::SerializeSteps() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*_element, _attributeName);
  }
}
