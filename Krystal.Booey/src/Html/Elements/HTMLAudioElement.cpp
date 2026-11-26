#include "Krystal.Booey/Html/Elements/HTMLAudioElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLAudioElement::HTMLAudioElement(Document &document) noexcept
      : HTMLMediaElement(document, html::HTMLElementInterface::Audio)
  {
  }
}
