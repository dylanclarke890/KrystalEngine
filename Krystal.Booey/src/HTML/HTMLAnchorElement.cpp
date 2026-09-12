#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLAnchorElement::HTMLAnchorElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Anchor)
  {
  }

#pragma region HTMLAnchorElement

  dom::DOMString HTMLAnchorElement::Download() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAnchorElement::Download(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"download", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLAnchorElement::Ping() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping");
  }

  dom::ExceptionOr<void> HTMLAnchorElement::Ping(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAnchorElement::Rel(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  DOMTokenList &HTMLAnchorElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  dom::DOMString HTMLAnchorElement::Text() const noexcept
  {
    return TextAlgorithms::DescendantTextContent(*this);
  }

  dom::ExceptionOr<void> HTMLAnchorElement::Text(dom::DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  dom::DOMString HTMLAnchorElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLANCHORELEMENT, HTML): ReferrerPolicy should be limited to known values
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLAnchorElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members

  dom::DOMString HTMLAnchorElement::Coords() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAnchorElement::Coords(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"coords", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Charset() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLAnchorElement::Charset(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLAnchorElement::Name(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Rev() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLAnchorElement::Rev(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rev", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Shape() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAnchorElement::Shape(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shape", krys::move(value));
  }

#pragma endregion
}