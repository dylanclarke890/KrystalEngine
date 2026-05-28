#include "Krystal.HTML/DOM/XMLDocument.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"

namespace Krys::HTML
{
  XMLDocument::XMLDocument(DocumentFlags flags) noexcept : Document(flags | DocumentFlags::IsXMLDocument)
  {
  }

  XMLDocument::XMLDocument() noexcept : Document(DocumentFlags::IsXMLDocument)
  {
  }
}
