#include "Krystal.HTML/HTML/HTMLVideoElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLVideoElement::HTMLVideoElement(Document &document) noexcept
      : HTMLMediaElement(document, HTMLElementInterface::Video)
  {
  }

#pragma region HTMLVideoElement

  uint32 HTMLVideoElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLVideoElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", Krys::Move(value));
  }

  uint32 HTMLVideoElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLVideoElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", Krys::Move(value));
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

  ExceptionOr<USVString> HTMLVideoElement::Poster() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"poster");
  }

  ExceptionOr<void> HTMLVideoElement::Poster(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"poster", Krys::Move(value));
  }

  bool HTMLVideoElement::PlaysInline() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"playsinline");
  }

  void HTMLVideoElement::PlaysInline(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"playsinline", Krys::Move(value));
  }

#pragma endregion
}
