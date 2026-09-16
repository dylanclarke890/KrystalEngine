#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLOptionElement::HTMLOptionElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Option)
  {
  }
#pragma region HTMLOptionElement

  bool HTMLOptionElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptionElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLOptionElement::Label() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLOptionElement::Label(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

  bool HTMLOptionElement::DefaultSelected() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"selected");
  }

  void HTMLOptionElement::DefaultSelected(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"selected", krys::move(value));
  }

  bool HTMLOptionElement::Selected() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"selected");
  }

  void HTMLOptionElement::Selected(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"selected", krys::move(value));
  }

  dom::DOMString HTMLOptionElement::Value() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLOptionElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

  dom::DOMString HTMLOptionElement::Text() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"text").value_or(u8"");
  }

  void HTMLOptionElement::Text(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"text", krys::move(value));
  }

#pragma endregion

#pragma region Extension Hooks

  void HTMLOptionElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    FormControlAlgorithms::UpdateNearestAncestorSelect(*this);
  }

  void HTMLOptionElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    FormControlAlgorithms::UpdateNearestAncestorSelect(*this);
  }

  void HTMLOptionElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);
    FormControlAlgorithms::UpdateNearestAncestorSelect(*this);
  }

#pragma endregion
}