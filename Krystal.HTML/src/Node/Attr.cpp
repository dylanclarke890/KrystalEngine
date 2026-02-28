#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"

namespace Krys::HTML
{
  DOMString Attr::NodeName() const noexcept
  {
    return _name.Name();
  }
}