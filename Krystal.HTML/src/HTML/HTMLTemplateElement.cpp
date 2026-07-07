#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTemplateElement::HTMLTemplateElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Template)
  {
  }

#pragma region HTMLTemplateElement

  RefPtr<DocumentFragment> HTMLTemplateElement::Content() const noexcept
  {
    assert(!Is<HTML::ShadowRoot>(_content.get()));
    return _content;
  }

  DOMString HTMLTemplateElement::ShadowRootMode() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"shadowrootmode").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootMode(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"shadowrootmode", Krys::Move(value));
  }

  bool HTMLTemplateElement::ShadowRootDelegatesFocus() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus");
  }

  void HTMLTemplateElement::ShadowRootDelegatesFocus(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus", Krys::Move(value));
  }

  DOMString HTMLTemplateElement::ShadowRootSlotAssignment() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"shadowrootslotassignment").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootSlotAssignment(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"shadowrootslotassignment", Krys::Move(value));
  }

  bool HTMLTemplateElement::ShadowRootClonable() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootclonable");
  }

  void HTMLTemplateElement::ShadowRootClonable(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootclonable", Krys::Move(value));
  }

  bool HTMLTemplateElement::ShadowRootSerializable() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootserializable");
  }

  void HTMLTemplateElement::ShadowRootSerializable(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootserializable", Krys::Move(value));
  }

  DOMString HTMLTemplateElement::ShadowRootCustomElementRegistry() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"shadowrootcustomelementregistry")
      .value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootCustomElementRegistry(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"shadowrootcustomelementregistry", Krys::Move(value));
  }

#pragma endregion
}