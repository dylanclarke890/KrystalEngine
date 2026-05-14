#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  HTMLElement::HTMLElement(Document &document, const QualifiedName &name, NodeFlag flags) noexcept
      : Element(document, name, flags | NodeFlag::IsHTMLElement)
  {
  }

  HTMLElement::HTMLElement(Document &document, const QualifiedName &name) noexcept
      : Element(document, name, NodeFlag::IsHTMLElement)
  {
  }
}