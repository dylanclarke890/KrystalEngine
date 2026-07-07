#include "Krystal.HTML/HTML/HTMLIFrameElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLIFrameElement::HTMLIFrameElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::IFrame)
  {
  }

#pragma region HTMLIFrameElement

  ExceptionOr<USVString> HTMLIFrameElement::Src() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src");
  }

  ExceptionOr<void> HTMLIFrameElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::SrcDoc() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"srcdoc").value_or(u8"");
  }

  void HTMLIFrameElement::SrcDoc(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"srcdoc", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::Sizes() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLIFrameElement::Sizes(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"sizes", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLIFrameElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMTokenList &HTMLIFrameElement::Sandbox() noexcept
  {
    if (!_sandbox)
    {
      _sandbox = ElementAlgorithms::CreateDOMTokenList(*this, u8"sandbox");
    }

    return *_sandbox;
  }

  DOMString HTMLIFrameElement::Allow() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"allow").value_or(u8"");
  }

  void HTMLIFrameElement::Allow(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"allow", Krys::Move(value));
  }

  bool HTMLIFrameElement::AllowFullscreen() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"allowfullscreen");
  }

  void HTMLIFrameElement::AllowFullscreen(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"allowfullscreen", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLIFrameElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLIFrameElement::Height(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"height", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::ReferrerPolicy() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLIFrameElement::ReferrerPolicy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::Loading() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"loading").value_or(u8"");
  }

  void HTMLIFrameElement::Loading(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"loading", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLIFrameElement Obsolete members

  DOMString HTMLIFrameElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLIFrameElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::Scrolling() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"scrolling").value_or(u8"");
  }

  void HTMLIFrameElement::Scrolling(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"scrolling", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::FrameBorder() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"frameborder").value_or(u8"");
  }

  void HTMLIFrameElement::FrameBorder(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"frameborder", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLIFrameElement::LongDesc() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"longdesc");
  }

  ExceptionOr<void> HTMLIFrameElement::LongDesc(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"longdesc", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::MarginWidth() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"marginwidth").value_or(u8"");
  }

  void HTMLIFrameElement::MarginWidth(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"marginwidth", Krys::Move(value));
  }

  DOMString HTMLIFrameElement::MarginHeight() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"marginheight").value_or(u8"");
  }

  void HTMLIFrameElement::MarginHeight(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"marginheight", Krys::Move(value));
  }

#pragma endregion
}
