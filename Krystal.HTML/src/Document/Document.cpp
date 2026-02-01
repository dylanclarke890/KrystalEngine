#include "Krystal.HTML/Document/Document.hpp"

namespace Krys::HTML
{
  DOMString Document::NodeName() const noexcept
  {
    return u8"#document";
  }
}