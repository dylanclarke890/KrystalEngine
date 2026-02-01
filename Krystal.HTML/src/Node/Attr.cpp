#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Document/Document.hpp"

namespace Krys::HTML
{
  DOMString Attr::NodeName() const noexcept
  {
    // TODO (IMPL): qualified name
    return u8"";
  }
}