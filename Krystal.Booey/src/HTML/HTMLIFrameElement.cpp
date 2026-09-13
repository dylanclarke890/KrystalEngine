#include "Krystal.Booey/HTML/HTMLIFrameElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLIFrameElement::HTMLIFrameElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::IFrame)
  {
  }

#pragma region HTMLIFrameElement

  dom::ExceptionOr<dom::USVString> HTMLIFrameElement::Src() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"src");
  }

  dom::ExceptionOr<void> HTMLIFrameElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::SrcDoc() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"srcdoc").value_or(u8"");
  }

  void HTMLIFrameElement::SrcDoc(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"srcdoc", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::Sizes() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"sizes").value_or(u8"");
  }

  void HTMLIFrameElement::Sizes(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"sizes", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLIFrameElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMTokenList &HTMLIFrameElement::Sandbox() noexcept
  {
    if (!_sandbox)
    {
      _sandbox = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"sandbox");
    }

    return *_sandbox;
  }

  dom::DOMString HTMLIFrameElement::Allow() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"allow").value_or(u8"");
  }

  void HTMLIFrameElement::Allow(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"allow", krys::move(value));
  }

  bool HTMLIFrameElement::AllowFullscreen() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"allowfullscreen");
  }

  void HTMLIFrameElement::AllowFullscreen(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"allowfullscreen", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLIFrameElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::Height() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLIFrameElement::Height(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::ReferrerPolicy() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLIFrameElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::Loading() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"loading").value_or(u8"");
  }

  void HTMLIFrameElement::Loading(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"loading", krys::move(value));
  }

#pragma endregion

#pragma region HTMLIFrameElement Obsolete members

  dom::DOMString HTMLIFrameElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLIFrameElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::Scrolling() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"scrolling").value_or(u8"");
  }

  void HTMLIFrameElement::Scrolling(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"scrolling", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::FrameBorder() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"frameborder").value_or(u8"");
  }

  void HTMLIFrameElement::FrameBorder(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"frameborder", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLIFrameElement::LongDesc() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"longdesc");
  }

  dom::ExceptionOr<void> HTMLIFrameElement::LongDesc(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"longdesc", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::MarginWidth() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"marginwidth").value_or(u8"");
  }

  void HTMLIFrameElement::MarginWidth(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"marginwidth", krys::move(value));
  }

  dom::DOMString HTMLIFrameElement::MarginHeight() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"marginheight").value_or(u8"");
  }

  void HTMLIFrameElement::MarginHeight(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"marginheight", krys::move(value));
  }

#pragma endregion
}
