#include "Krystal.HTML/HTML/HTMLImageElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLImageElement::HTMLImageElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Image)
  {
  }

#pragma region HTMLImageElement

  DOMString HTMLImageElement::Alt() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLImageElement::Alt(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"alt", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLImageElement::Src() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src");
  }

  ExceptionOr<void> HTMLImageElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"src", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLImageElement::SrcSet() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset");
  }

  ExceptionOr<void> HTMLImageElement::SrcSet(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"srcset", Krys::Move(value));
  }

  DOMString HTMLImageElement::Sizes() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLImageElement::Sizes(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"sizes", Krys::Move(value));
  }

  Maybe<DOMString> HTMLImageElement::CrossOrigin() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): CrossOrigin() is limited to known values.
    return Attributes::Reflection::Reflect<Maybe<DOMString>>(*this, u8"crossorigin");
  }

  void HTMLImageElement::CrossOrigin(Maybe<DOMString> &&value) noexcept
  {
    Attributes::Reflection::Reflect<Maybe<DOMString>>(*this, u8"crossorigin", Krys::Move(value));
  }

  DOMString HTMLImageElement::UseMap() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLImageElement::UseMap(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"usemap", Krys::Move(value));
  }

  bool HTMLImageElement::IsMap() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"ismap");
  }

  void HTMLImageElement::IsMap(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"ismap", Krys::Move(value));
  }

  bool HTMLImageElement::Controls() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"controls");
  }

  void HTMLImageElement::Controls(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"controls", Krys::Move(value));
  }

  uint32 HTMLImageElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLImageElement::Width(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"width", Krys::Move(value));
  }

  uint32 HTMLImageElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLImageElement::Height(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"height", Krys::Move(value));
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

  DOMString HTMLImageElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): ReferrerPolicy() is limited to known values.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLImageElement::ReferrerPolicy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy", Krys::Move(value));
  }

  DOMString HTMLImageElement::Decoding() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Decoding() is limited to known values.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"decoding").value_or(u8"");
  }

  void HTMLImageElement::Decoding(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"decoding", Krys::Move(value));
  }

  DOMString HTMLImageElement::Loading() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): Loading() is limited to known values.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"loading").value_or(u8"");
  }

  void HTMLImageElement::Loading(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"loading", Krys::Move(value));
  }

  DOMString HTMLImageElement::FetchPriority() const noexcept
  {
    // TODO(HTMLIMAGEELEMENT, HTML): FetchPriority() is limited to known values.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLImageElement::FetchPriority(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"fetchpriority", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLImageElement Obsolete members

  DOMString HTMLImageElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLImageElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLImageElement::LowSrc() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"lowsrc");
  }

  ExceptionOr<void> HTMLImageElement::LowSrc(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"lowsrc", Krys::Move(value));
  }

  DOMString HTMLImageElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLImageElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  uint32 HTMLImageElement::HSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLImageElement::HSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"hspace", Krys::Move(value));
  }

  uint32 HTMLImageElement::VSpace() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLImageElement::VSpace(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"vspace", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLImageElement::LongDesc() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"longdesc");
  }

  ExceptionOr<void> HTMLImageElement::LongDesc(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"longdesc", Krys::Move(value));
  }

  DOMString HTMLImageElement::Border() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLImageElement::Border(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"border", Krys::Move(value));
  }

#pragma endregion
}
