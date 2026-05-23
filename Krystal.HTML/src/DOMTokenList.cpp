#include "Krystal.HTML/DOMTokenList.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/Algorithms/StringAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  DOMTokenList::DOMTokenList(Element &element, DOMStringAtom attributeName,
                             IsSupportedTokenFunction &&isSupportedToken) noexcept
      : _attributeName {attributeName}, _element {ShareCheckedRef(element)},
        _isSupportedToken {Krys::Move(isSupportedToken)}
  {
    auto value = ElementAlgorithms::GetAttributeValue(*_element, _attributeName);
    auto tokens = OrderedSet::Parser(value);
    _tokens.append(tokens.begin(), tokens.end());
  }

  void DOMTokenList::Value(DOMString &&value) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*_element, _attributeName, Krys::Move(value));
  }

  ExceptionOr<bool> DOMTokenList::ValidationSteps(DOMStringView token) const noexcept
  {
    if (!_isSupportedToken)
    {
      return Exception {ExceptionCode::TypeError};
    }

    auto lowercaseToken = Krys::Text::ToASCIILowercase(token);

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
