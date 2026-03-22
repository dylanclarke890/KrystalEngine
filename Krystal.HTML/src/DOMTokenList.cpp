#include "Krystal.HTML/DOMTokenList.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  DOMTokenList::DOMTokenList(Element &element, const QualifiedName &attributeName,
                             IsSupportedTokenFunction &&isSupportedToken) noexcept
      : _attributeName {attributeName}, _element {ShareCheckedRef(element)},
        _isSupportedToken {Krys::Move(isSupportedToken)}
  {
  }

  void DOMTokenList::Value(const DOMString &value) noexcept
  {
    // TODO(impl): the value setter steps are to set an attribute value for this’s element using this’s
    // attribute name and the given value.
  }

  ExceptionOr<bool> DOMTokenList::ValidationSteps(DOMStringView token) const noexcept
  {
    if (!_isSupportedToken)
    {
      return Exception {ExceptionCode::TypeError};
    }

    // TODO(fix): Let lowercaseToken be token, in ASCII lowercase.

    return _isSupportedToken(_element->NodeDocument(), token);
  }
}
