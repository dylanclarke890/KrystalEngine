#include "Krystal.HTML/Element/Element.hpp"

namespace Krys::HTML
{
  DOMString Element::NodeName() const noexcept
  {
    // TODO(IMPL): Return the qualified name
    return u8"element";
  }
}