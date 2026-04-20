#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  void Attr::SetExistingAttributeValue(Attr &attribute, DOMString &&value) noexcept
  {
    auto element = attribute._ownerElement.lock();

    if (!element)
    {
      attribute._value = Krys::Move(value);
      return;
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with attribute’s local name, attribute’s namespace, element, and value.

    // TODO(IMPL): Change attribute to verifiedValue (implement Element first).
  }
}