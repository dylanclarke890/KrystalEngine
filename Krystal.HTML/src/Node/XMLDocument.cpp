#include "Krystal.HTML/Node/XMLDocument.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"

namespace Krys::HTML
{
  XMLDocument::XMLDocument() noexcept : Document(Type::XML)
  {
  }
}
