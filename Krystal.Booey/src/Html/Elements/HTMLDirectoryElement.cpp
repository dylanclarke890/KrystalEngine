#include "Krystal.Booey/Html/Elements/HTMLDirectoryElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLDirectoryElement::HTMLDirectoryElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Directory)
  {
  }

  bool HTMLDirectoryElement::Compact() const noexcept
  {
    return html::Reflect<bool>(*this, u8"compact");
  }

  void HTMLDirectoryElement::Compact(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"compact", krys::move(value));
  }
}
