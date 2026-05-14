#include "Krystal.HTML/HTMLElement/HTMLPreElement.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLPreElement::HTMLPreElement(Document &document, const QualifiedName &name) noexcept
      : HTMLElement(document, name)
  {
  }
}