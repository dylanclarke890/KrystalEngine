#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Element/Element.hpp"

namespace Krys::HTML
{
  DOMString Attr::NodeName() const noexcept
  {
    return _name.Name();
  }
}