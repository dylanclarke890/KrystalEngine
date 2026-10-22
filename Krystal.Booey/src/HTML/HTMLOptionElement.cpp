#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  HTMLOptionElement::HTMLOptionElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Option)
  {
  }

  bool HTMLOptionElement::IsDisabledFormControl() const noexcept
  {
    return FormControlAlgorithms::IsDisabled(*this);
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
    return _selectedness;
  }

  void HTMLOptionElement::Selected(bool value) noexcept
  {
    // The selected IDL attribute, on setting, must set the element's selectedness to the new value, set its
    // dirtiness to true, and then cause the element to ask for a reset.
    _dirtiness = true;
    SetSelectedState(value);
    if (auto *select = FormControlAlgorithms::NearestAncestorSelect(*this))
    {
      FormControlAlgorithms::SelectednessSettingAlgorithm(*select);
    }
  }

  dom::DOMString HTMLOptionElement::Value() const noexcept
  {
    // The value of an option element is the value of the value content attribute, if there is one, or, if
    // there is not, the value of the element's text IDL attribute.
    if (auto value = GetAttribute(u8"value"))
    {
      return *value;
    }
    return Text();
  }

  void HTMLOptionElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

  dom::DOMString HTMLOptionElement::Text() const noexcept
  {
    // The text IDL attribute, on getting, must return the result of collecting option text given this and
    // false, then stripping and collapsing ASCII whitespace.
    return infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(
      FormControlAlgorithms::CollectOptionText(*this, false));
  }

  void HTMLOptionElement::Text(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"text", krys::move(value));
  }

#pragma endregion

  void HTMLOptionElement::SetSelectedState(bool selected) noexcept
  {
    if (_selectedness == selected)
    {
      return;
    }

    style::PseudoClassChangeInvalidation checkedInvalidation(*this, css::SelectorPseudoClass::Checked,
                                                             selected);
    _selectedness = selected;

    if (auto *select = FormControlAlgorithms::NearestAncestorSelect(*this))
    {
      select->UpdateValidity();
    }
  }

  void HTMLOptionElement::ResetSelectedness() noexcept
  {
    _dirtiness = false;
    SetSelectedState(HasAttribute(u8"selected"));
  }

#pragma region Extension Hooks

  void HTMLOptionElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null() || localName != u8"selected"
        || oldValue.has_value() == value.has_value())
    {
      return;
    }

    style::PseudoClassChangeInvalidation defaultInvalidation(*this, css::SelectorPseudoClass::Default,
                                                             value.has_value());

    // Whenever an option element's selected attribute is added or removed, if its dirtiness is false, its
    // selectedness must follow it.
    if (!_dirtiness)
    {
      SetSelectedState(value.has_value());
      if (auto *select = FormControlAlgorithms::NearestAncestorSelect(*this))
      {
        FormControlAlgorithms::SelectednessSettingAlgorithm(*select);
      }
    }
  }

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