#include "Krystal.Booey/HTML/Elements/HTMLAudioElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLAudioElement::HTMLAudioElement(dom::Document &document) noexcept
      : HTMLMediaElement(document, HTMLElementInterface::Audio)
  {
  }
}
