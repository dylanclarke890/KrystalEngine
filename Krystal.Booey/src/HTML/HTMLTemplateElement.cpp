#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  // TODO(HTMLTemplateElement): Implement the HTMLTemplateElement constructor in a spec compliant way.

  HTMLTemplateElement::HTMLTemplateElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Template),
        _content(CreateRef<dom::DocumentFragment>(document))
  {
  }

#pragma region HTMLTemplateElement

  RefPtr<dom::DocumentFragment> HTMLTemplateElement::Content() const noexcept
  {
    assert(!Is<dom::ShadowRoot>(_content.get()));
    return _content;
  }

  dom::DOMString HTMLTemplateElement::ShadowRootMode() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootmode").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootMode(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootmode", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootDelegatesFocus() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus");
  }

  void HTMLTemplateElement::ShadowRootDelegatesFocus(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"shadowrootdelegatesfocus", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootSlotAssignment() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment").value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootSlotAssignment(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootslotassignment", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootClonable() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"shadowrootclonable");
  }

  void HTMLTemplateElement::ShadowRootClonable(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"shadowrootclonable", krys::move(value));
  }

  bool HTMLTemplateElement::ShadowRootSerializable() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"shadowrootserializable");
  }

  void HTMLTemplateElement::ShadowRootSerializable(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"shadowrootserializable", krys::move(value));
  }

  dom::DOMString HTMLTemplateElement::ShadowRootCustomElementRegistry() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry")
      .value_or(u8"");
  }

  void HTMLTemplateElement::ShadowRootCustomElementRegistry(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shadowrootcustomelementregistry", krys::move(value));
  }

#pragma endregion
}