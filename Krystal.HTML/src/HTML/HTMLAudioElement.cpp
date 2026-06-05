#include "Krystal.HTML/HTML/HTMLAudioElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLAudioElement::HTMLAudioElement(Document &document) noexcept
      : HTMLMediaElement(document, DOMInterface::Audio)
  {
  }
}
