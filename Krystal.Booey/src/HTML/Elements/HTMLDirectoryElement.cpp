#include "Krystal.Booey/HTML/Elements/HTMLDirectoryElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLDirectoryElement::HTMLDirectoryElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Directory)
  {
  }

  bool HTMLDirectoryElement::Compact() const noexcept
  {
    return Reflect<bool>(*this, u8"compact");
  }

  void HTMLDirectoryElement::Compact(bool value) noexcept
  {
    Reflect<bool>(*this, u8"compact", krys::move(value));
  }
}
