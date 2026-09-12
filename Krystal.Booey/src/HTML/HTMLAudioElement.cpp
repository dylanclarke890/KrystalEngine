#include "Krystal.Booey/HTML/HTMLAudioElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLAudioElement::HTMLAudioElement(Document &document) noexcept
      : HTMLMediaElement(document, HTMLElementInterface::Audio)
  {
  }
}
