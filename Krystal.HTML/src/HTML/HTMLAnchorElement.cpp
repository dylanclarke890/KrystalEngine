#include "Krystal.HTML/HTML/HTMLAnchorElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLAnchorElement::HTMLAnchorElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Anchor)
  {
  }

#pragma region HTMLAnchorElement

  DOMString HTMLAnchorElement::Download() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAnchorElement::Download(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"download", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLAnchorElement::Ping() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping");
  }

  ExceptionOr<void> HTMLAnchorElement::Ping(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping", Krys::Move(value));
  }

  DOMString HTMLAnchorElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAnchorElement::Rel(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rel", Krys::Move(value));
  }

  DOMTokenList &HTMLAnchorElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  DOMString HTMLAnchorElement::Text() const noexcept
  {
    return TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> HTMLAnchorElement::Text(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(Krys::Move(value), *this);
  }

  DOMString HTMLAnchorElement::ReferrerPolicy() const noexcept
  {
    // TODO(HTMLANCHORELEMENT, HTML): ReferrerPolicy should be limited to known values
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLAnchorElement::ReferrerPolicy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members

  DOMString HTMLAnchorElement::Coords() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAnchorElement::Coords(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"coords", Krys::Move(value));
  }

  DOMString HTMLAnchorElement::Charset() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLAnchorElement::Charset(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charset", Krys::Move(value));
  }

  DOMString HTMLAnchorElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLAnchorElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMString HTMLAnchorElement::Rev() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLAnchorElement::Rev(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rev", Krys::Move(value));
  }

  DOMString HTMLAnchorElement::Shape() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAnchorElement::Shape(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"shape", Krys::Move(value));
  }

#pragma endregion
}