#include "Krystal.Web/Html/Elements/HTMLAudioElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
{
  HTMLAudioElement::HTMLAudioElement(Document &document) noexcept
      : HTMLMediaElement(document, html::HTMLElementInterface::Audio)
  {
  }
}
