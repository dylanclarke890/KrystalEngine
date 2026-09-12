#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  // TODO(HTMLTemplateElement): Implement the HTMLTemplateElement constructor in a spec compliant way.

  HTMLTemplateElement::HTMLTemplateElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Template), _content(CreateRef<DocumentFragment>(document))
  {
  }

#pragma region HTMLTemplateElement

  RefPtr<DocumentFragment> HTMLTemplateElement::Content() const noexcept
  {
    assert(!Is<HTML::ShadowRoot>(_content.get()));
    return _content;
  }

  dom::DOMString HTMLTemplateElement::ShadowRootMode() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootmode").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootMode(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootmode", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootDelegatesFocus() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus");
  }

  void HTMLTemplateElement::ShadowRootDelegatesFocus(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootSlotAssignment() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootSlotAssignment(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootClonable() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootclonable");
  }

  void HTMLTemplateElement::ShadowRootClonable(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootclonable", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootSerializable() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootserializable");
  }

  void HTMLTemplateElement::ShadowRootSerializable(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"shadowrootserializable", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootCustomElementRegistry() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry")
      .value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootCustomElementRegistry(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry", krys::move(value));
  }

#pragma endregion
}