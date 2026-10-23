#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Algorithms/FormControlAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"

namespace krys::boo::html
{
  HTMLSelectElement::HTMLSelectElement(dom::Document &document) noexcept
      : HTMLFormControlElement(document, HTMLElementInterface::Select)
  {
  }

  dom::DOMString HTMLSelectElement::FormControlType() const noexcept
  {
    return HasAttribute(u8"multiple") ? u8"select-multiple" : u8"select-one";
  }

  int32 HTMLSelectElement::SelectedIndex() const noexcept
  {
    int32 index = 0;
    for (auto &option : FormControlAlgorithms::ListOfOptions(*this))
    {
      if (option->Selected())
      {
        return index;
      }
      ++index;
    }
    return -1;
  }

  bool HTMLSelectElement::HasPlaceholderLabelOption() const noexcept
  {
    // The select element has no placeholder label option if it has an attribute "multiple" specified or a
    // display size of non-1.
    if (HasAttribute(u8"multiple") || FormControlAlgorithms::DisplaySize(*this) > 1)
    {
      return false;
    }

    // The first option of the list of options is a placeholder if its value is empty and its parent is the
    // select.
    auto options = FormControlAlgorithms::ListOfOptions(*this);
    if (options.empty())
    {
      return false;
    }
    auto &option = options.front();
    return option->ParentNode() == this && option->Value().empty();
  }

  bool HTMLSelectElement::ValueMissing() const noexcept
  {
    if (!WillValidate())
    {
      return false;
    }

    if (!IsRequired())
    {
      return false;
    }

    int32 firstSelectionIndex = SelectedIndex();

    // If a non-placeholder label option is selected (firstSelectionIndex > 0), it's not value-missing.
    return firstSelectionIndex < 0 || (!firstSelectionIndex && HasPlaceholderLabelOption());
  }

  bool HTMLSelectElement::AppendFormData(DOMFormData &formData) noexcept
  {
    auto name = Name();
    if (name.empty())
    {
      return false;
    }

    bool successful = false;
    for (auto &option : FormControlAlgorithms::ListOfOptions(*this))
    {
      if (option->Selected() && !option->IsDisabledFormControl())
      {
        formData.Append(name, option->Value());
        successful = true;
      }
    }
    return successful;
  }

  void HTMLSelectElement::Reset() noexcept
  {
    // The reset algorithm for a select element selectedOption is to go through all the option elements in the
    // element's list of options, set their selectedness to true if the option element has a selected
    // attribute, and false otherwise, set their dirtiness to false, and then run the selectedness setting
    // algorithm.
    for (auto &option : FormControlAlgorithms::ListOfOptions(*this))
    {
      option->ResetSelectedness();
    }
    FormControlAlgorithms::SelectednessSettingAlgorithm(*this);

    InvalidateStyleForSubtree();
    UpdateValidity();
    SetInteractedWithSinceLastFormSubmitEvent(false);
  }

#pragma region HTMLSelectElement

  dom::DOMString HTMLSelectElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLSelectElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  bool HTMLSelectElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLSelectElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  bool HTMLSelectElement::UsesMenuList() const noexcept
  {
    if (RenderTheme::Singleton().DelegatesMenuListRendering())
    {
      return true;
    }

    return !Multiple() && Size() <= 1;
  }

  bool HTMLSelectElement::Multiple() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"multiple");
  }

  void HTMLSelectElement::Multiple(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"multiple", krys::move(value));
  }

  dom::DOMString HTMLSelectElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSelectElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLSelectElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLSelectElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLSelectElement::Size() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"size");
  }

  void HTMLSelectElement::Size(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"size", krys::move(value));
  }

#pragma endregion
}