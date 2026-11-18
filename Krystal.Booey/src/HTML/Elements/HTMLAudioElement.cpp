#include "Krystal.Booey/HTML/Elements/HTMLAudioElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLAudioElement::HTMLAudioElement(Document &document) noexcept
      : HTMLMediaElement(document, html::HTMLElementInterface::Audio)
  {
  }
}
