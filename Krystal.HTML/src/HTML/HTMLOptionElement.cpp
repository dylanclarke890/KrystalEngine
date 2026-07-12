#include "Krystal.HTML/HTML/HTMLOptionElement.hpp"
#include "Krystal.HTML/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLOptionElement::HTMLOptionElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Option)
  {
  }
#pragma region HTMLOptionElement

  bool HTMLOptionElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptionElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  DOMString HTMLOptionElement::Label() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLOptionElement::Label(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"label", Krys::Move(value));
  }

  bool HTMLOptionElement::DefaultSelected() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"selected");
  }

  void HTMLOptionElement::DefaultSelected(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"selected", Krys::Move(value));
  }

  bool HTMLOptionElement::Selected() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"selected");
  }

  void HTMLOptionElement::Selected(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"selected", Krys::Move(value));
  }

  DOMString HTMLOptionElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLOptionElement::Value(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"value", Krys::Move(value));
  }

  DOMString HTMLOptionElement::Text() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"text").value_or(u8"");
  }

  void HTMLOptionElement::Text(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"text", Krys::Move(value));
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