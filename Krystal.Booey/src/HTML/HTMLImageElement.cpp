#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLImageElement::HTMLImageElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Image)
  {
  }

#pragma region HTMLImageElement

  dom::DOMString HTMLImageElement::Alt() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLImageElement::Alt(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLImageElement::Src() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src");
  }

  dom::ExceptionOr<void> HTMLImageElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLImageElement::SrcSet() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset");
  }

  dom::ExceptionOr<void> HTMLImageElement::SrcSet(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Sizes() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLImageElement::Sizes(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLImageElement::CrossOrigin() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): CrossOrigin() is limited to known values.
    return Attributes::Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin");
  }

  void HTMLImageElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    Attributes::Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLImageElement::UseMap() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLImageElement::UseMap(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

  bool HTMLImageElement::IsMap() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"ismap");
  }

  void HTMLImageElement::IsMap(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"ismap", krys::move(value));
  }

  bool HTMLImageElement::Controls() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"controls");
  }

  void HTMLImageElement::Controls(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"controls", krys::move(value));
  }

  uint32 HTMLImageElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLImageElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  uint32 HTMLImageElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLImageElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  uint32 HTMLImageElement::NaturalWidth() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): NaturalWidth().
    return 0u;
  }

  uint32 HTMLImageElement::NaturalHeight() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): NaturalHeight().
    return 0u;
  }

  bool HTMLImageElement::Complete() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Complete().
    return false;
  }

  USVString HTMLImageElement::CurrentSrc() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): CurrentSrc().
    return {};
  }

  dom::DOMString HTMLImageElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): ReferrerPolicy() is limited to known values.
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLImageElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Decoding() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Decoding() is limited to known values.
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"decoding").value_or(u8"");
  }

  void HTMLImageElement::Decoding(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"decoding", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Loading() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Loading() is limited to known values.
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"loading").value_or(u8"");
  }

  void HTMLImageElement::Loading(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"loading", krys::move(value));
  }

  dom::DOMString HTMLImageElement::FetchPriority() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): FetchPriority() is limited to known values.
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLImageElement::FetchPriority(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLImageElement Obsolete members

  dom::DOMString HTMLImageElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLImageElement::Name(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLImageElement::LowSrc() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"lowsrc");
  }

  dom::ExceptionOr<void> HTMLImageElement::LowSrc(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"lowsrc", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLImageElement::Align(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  uint32 HTMLImageElement::HSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLImageElement::HSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  uint32 HTMLImageElement::VSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLImageElement::VSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLImageElement::LongDesc() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"longdesc");
  }

  dom::ExceptionOr<void> HTMLImageElement::LongDesc(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"longdesc", krys::move(value));
  }

  dom::DOMString HTMLImageElement::Border() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLImageElement::Border(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
  }

#pragma endregion
}
