#include "Krystal.HTML/HTML/HTMLLinkElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLLinkElement::HTMLLinkElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Link)
  {
  }

#pragma region HTMLLinkElement

  ExceptionOr<USVString> HTMLLinkElement::Href() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"href");
  }

  ExceptionOr<void> HTMLLinkElement::Href(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"href", Krys::Move(value));
  }

  Maybe<DOMString> HTMLLinkElement::CrossOrigin() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Attributes::Reflection::Reflect<Maybe<DOMString>>(*this, u8"crossorigin");
  }

  void HTMLLinkElement::CrossOrigin(Maybe<DOMString> &&value) noexcept
  {
    Attributes::Reflection::Reflect<Maybe<DOMString>>(*this, u8"crossorigin", Krys::Move(value));
  }

  DOMString HTMLLinkElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLLinkElement::Rel(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rel", Krys::Move(value));
  }

  DOMString HTMLLinkElement::As() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"as").value_or(u8"");
  }

  void HTMLLinkElement::As(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"as", Krys::Move(value));
  }

  KRYS_NODISCARD DOMTokenList &HTMLLinkElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = ElementAlgorithms::CreateDOMTokenList(*this, u8"rellist");
    }

    return *_relList;
  }

  DOMString HTMLLinkElement::Media() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLLinkElement::Media(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"media", Krys::Move(value));
  }

  DOMString HTMLLinkElement::Integrity() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLLinkElement::Integrity(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"integrity", Krys::Move(value));
  }

  DOMString HTMLLinkElement::HrefLang() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLLinkElement::HrefLang(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"hreflang", Krys::Move(value));
  }

  DOMString HTMLLinkElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLinkElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

  KRYS_NODISCARD DOMTokenList &HTMLLinkElement::Sizes() noexcept
  {
    if (!_sizes)
    {
      _sizes = ElementAlgorithms::CreateDOMTokenList(*this, u8"sizes");
    }

    return *_sizes;
  }

  ExceptionOr<USVString> HTMLLinkElement::ImageSrcset() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"imagesrcset");
  }

  ExceptionOr<void> HTMLLinkElement::ImageSrcset(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"imagesrcset", Krys::Move(value));
  }

  DOMString HTMLLinkElement::ImageSizes() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"imagesizes").value_or(u8"");
  }

  void HTMLLinkElement::ImageSizes(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"imagesizes", Krys::Move(value));
  }

  DOMString HTMLLinkElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLLinkElement::ReferrerPolicy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy", Krys::Move(value));
  }

  KRYS_NODISCARD DOMTokenList &HTMLLinkElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

  bool HTMLLinkElement::Disabled() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): Whenever the disabled attribute is removed, set the link element's
    // explicitly enabled attribute to true. This is more than just changing this method because the attribute
    // can be set in other ways.
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLLinkElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  DOMString HTMLLinkElement::FetchPriority() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLLinkElement::FetchPriority(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"fetchpriority", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLLinkElement Obsolete members

  DOMString HTMLLinkElement::Charset() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLLinkElement::Charset(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charset", Krys::Move(value));
  }

  DOMString HTMLLinkElement::Rev() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLLinkElement::Rev(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rev", Krys::Move(value));
  }

  DOMString HTMLLinkElement::Target() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLLinkElement::Target(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"target", Krys::Move(value));
  }

#pragma endregion
}
