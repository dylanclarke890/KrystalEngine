#include "Krystal.HTML/DOMTokenList.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/StringAlgorithms.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
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
}
