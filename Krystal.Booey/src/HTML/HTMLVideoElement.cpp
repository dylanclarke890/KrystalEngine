#include "Krystal.Booey/HTML/HTMLVideoElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

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
    // FIXME: Implement VideoWidth()
    return 0u;
  }

  uint32 HTMLVideoElement::VideoHeight() const noexcept
  {
    // FIXME: Implement VideoHeight()
    return 0u;
  }

  dom::USVString HTMLVideoElement::Poster() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"poster");
  }

  void HTMLVideoElement::Poster(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"poster", krys::move(value));
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

#pragma region Presentational hints

  bool HTMLVideoElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"width" || localName == u8"height"))
    {
      return true;
    }
    return HTMLMediaElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLVideoElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                dom::DOMStringAtom namespaceURI,
                                                                dom::DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLMediaElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(value, AttributeOrEmpty(u8"height"), style);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      ApplyAspectRatioFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"), value, style);
      return;
    }
    HTMLMediaElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
