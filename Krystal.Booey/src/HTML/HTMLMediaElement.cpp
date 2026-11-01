#include "Krystal.Booey/HTML/HTMLMediaElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLMediaElement::HTMLMediaElement(dom::Document &document, HTMLElementInterface interface) noexcept
      : HTMLElement(document, interface, HTMLElementFlags::IsMediaElement)
  {
  }

  HTMLMediaElement::HTMLMediaElement(dom::Document &document) noexcept
      : HTMLMediaElement(document, HTMLElementInterface::Media)
  {
  }
}
