#include "Krystal.Booey/HTML/HTMLVideoElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLVideoElement::HTMLVideoElement(dom::Document &document) noexcept
      : HTMLMediaElement(document, HTMLElementInterface::Video)
  {
  }

#pragma region HTMLVideoElement

  uint32 HTMLVideoElement::Width() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLVideoElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLVideoElement::Height() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLVideoElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  uint32 HTMLVideoElement::VideoWidth() const noexcept
  {
    // TODO(HTMLVideoElement, HTML): Implement VideoWidth()
    return 0u;
  }

  uint32 HTMLVideoElement::VideoHeight() const noexcept
  {
    // TODO(HTMLVideoElement, HTML): Implement VideoHeight()
    return 0u;
  }

  dom::ExceptionOr<dom::USVString> HTMLVideoElement::Poster() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"poster");
  }

  dom::ExceptionOr<void> HTMLVideoElement::Poster(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"poster", krys::move(value));
  }

  bool HTMLVideoElement::PlaysInline() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"playsinline");
  }

  void HTMLVideoElement::PlaysInline(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"playsinline", krys::move(value));
  }

#pragma endregion
}
