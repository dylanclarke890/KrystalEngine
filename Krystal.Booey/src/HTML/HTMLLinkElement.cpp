#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLLinkElement::HTMLLinkElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Link)
  {
  }

#pragma region HTMLLinkElement

  dom::ExceptionOr<dom::USVString> HTMLLinkElement::Href() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"href");
  }

  dom::ExceptionOr<void> HTMLLinkElement::Href(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLLinkElement::CrossOrigin() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin");
  }

  void HTMLLinkElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Rel() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLLinkElement::Rel(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::As() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<dom::DOMString>(*this, u8"as").value_or(u8"");
  }

  void HTMLLinkElement::As(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"as", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"rellist");
    }

    return *_relList;
  }

  dom::DOMString HTMLLinkElement::Media() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLLinkElement::Media(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Integrity() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLLinkElement::Integrity(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"integrity", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::HrefLang() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLLinkElement::HrefLang(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"hreflang", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLinkElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::Sizes() noexcept
  {
    if (!_sizes)
    {
      _sizes = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"sizes");
    }

    return *_sizes;
  }

  dom::ExceptionOr<dom::USVString> HTMLLinkElement::ImageSrcset() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"imagesrcset");
  }

  dom::ExceptionOr<void> HTMLLinkElement::ImageSrcset(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"imagesrcset", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::ImageSizes() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"imagesizes").value_or(u8"");
  }

  void HTMLLinkElement::ImageSizes(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"imagesizes", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLLinkElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  KRYS_NODISCARD dom::DOMTokenList &HTMLLinkElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

  bool HTMLLinkElement::Disabled() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): Whenever the disabled attribute is removed, set the link element's
    // explicitly enabled attribute to true. This is more than just changing this method because the attribute
    // can be set in other ways.
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLLinkElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::FetchPriority() const noexcept
  {
    // TODO(HTMLLINKELEMENT, HTML): This is an enumerated attribute but I don't know the states/keywords.
    return Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLLinkElement::FetchPriority(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLinkElement Obsolete members

  dom::DOMString HTMLLinkElement::Charset() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLLinkElement::Charset(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Rev() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLLinkElement::Rev(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rev", krys::move(value));
  }

  dom::DOMString HTMLLinkElement::Target() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLLinkElement::Target(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

#pragma endregion
}
