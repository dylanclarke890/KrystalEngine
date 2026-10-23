#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Rendering/RenderButton.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLButtonElement::HTMLButtonElement(dom::Document &document) noexcept
      : HTMLFormControlElement(document, HTMLElementInterface::Button)
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
  }

  RenderPtr<RenderElement> HTMLButtonElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    // https://html.spec.whatwg.org/multipage/rendering.html#button-layout
    if (style.IsDisplayFlexibleOrGridFormattingContextBox())
    {
      return HTMLFormControlElement::CreateElementRenderer(krys::move(style));
    }
    return CreateRenderer<RenderButton>(*this, krys::move(style));
  }

  dom::DOMString HTMLButtonElement::FormControlType() const noexcept
  {
    switch (_type)
    {
      case ButtonType::Submit: return u8"submit";
      case ButtonType::Button: return u8"button";
      case ButtonType::Reset:  return u8"reset";
    }
    krys_unreachable();
  }

  void HTMLButtonElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"type")
    {
      ComputeType(value.value_or(dom::DOMStringView()));
    }
    // FIXME: The command and commandfor attributes (Settings::commandAttributesEnabled), once there are
    // commands.
  }

  void HTMLButtonElement::ActivationBehavior(dom::Event &event) noexcept
  {
    if (IsDisabledFormControl())
    {
      return;
    }

    if (Form())
    {
      // Update layout before processing form actions in case the style changes the form or button
      // relationships.
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();
      if (RefPtr<HTMLFormElement> currentForm = ShareRefPtr(Form()))
      {
        if (_type == ButtonType::Submit)
        {
          currentForm->SubmitIfPossible(&event, this);
        }
        if (_type == ButtonType::Reset)
        {
          currentForm->Reset();
        }
      }
      if (_type == ButtonType::Submit || _type == ButtonType::Reset)
      {
        event.SetDefaultHandled();
        return;
      }
    }

    // FIXME: The button's command (handleCommand) and popover target (handlePopoverTargetAction), once there
    // are commands and popovers.
  }

  void HTMLButtonElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event))
    {
      if (keyboardEvent->Type() == EventNames::KeyDown && keyboardEvent->KeyIdentifier() == u8"U+0020")
      {
        SetActive(true);
        // No SetDefaultHandled(): a keypress follows.
        return;
      }
      if (keyboardEvent->Type() == EventNames::KeyPress)
      {
        switch (keyboardEvent->CharCode())
        {
          case '\r':
            (void)DispatchSimulatedClick(keyboardEvent);
            keyboardEvent->SetDefaultHandled();
            return;
          case ' ':
            // Prevent scrolling down the page.
            keyboardEvent->SetDefaultHandled();
            return;
          default: break;
        }
      }
      if (keyboardEvent->Type() == EventNames::KeyUp && keyboardEvent->KeyIdentifier() == u8"U+0020")
      {
        if (Active())
        {
          (void)DispatchSimulatedClick(keyboardEvent);
        }
        keyboardEvent->SetDefaultHandled();
        return;
      }
    }

    HTMLFormControlElement::DefaultEventHandler(event);
  }

  bool HTMLButtonElement::IsSuccessfulSubmitButton() const noexcept
  {
    // HTML spec says that buttons must have names to be considered successful. However, other browsers do not
    // impose this constraint.
    return _type == ButtonType::Submit;
  }

  bool HTMLButtonElement::MatchesDefaultPseudoClass() const noexcept
  {
    auto *form = Form();
    return IsSuccessfulSubmitButton() && form && form->DefaultButton() == this;
  }

  bool HTMLButtonElement::IsActivatedSubmit() const noexcept
  {
    return _isActivatedSubmit;
  }

  void HTMLButtonElement::SetActivatedSubmit(bool flag) noexcept
  {
    _isActivatedSubmit = flag;
  }

  bool HTMLButtonElement::AppendFormData(DOMFormData &formData) noexcept
  {
    if (_type != ButtonType::Submit || Name().empty() || !_isActivatedSubmit)
    {
      return false;
    }
    formData.Append(Name(), Value());
    return true;
  }

  bool HTMLButtonElement::ComputeWillValidate() const noexcept
  {
    return _type == ButtonType::Submit && HTMLFormControlElement::ComputeWillValidate();
  }

  bool HTMLButtonElement::IsSubmitButton() const noexcept
  {
    return _type == ButtonType::Submit;
  }

  bool HTMLButtonElement::IsExplicitlySetSubmitButton() const noexcept
  {
    return IsSubmitButton() && HasAttribute(u8"type");
  }

  void HTMLButtonElement::ComputeType(dom::DOMStringView typeAttributeValue) noexcept
  {
    auto oldType = _type;
    if (text::IsASCIICaselessEqual(typeAttributeValue, dom::DOMStringView(u8"reset")))
    {
      _type = ButtonType::Reset;
    }
    else if (text::IsASCIICaselessEqual(typeAttributeValue, dom::DOMStringView(u8"button")))
    {
      _type = ButtonType::Button;
    }
    else
    {
      _type = ButtonType::Submit;
    }

    if (oldType != _type)
    {
      UpdateWillValidateAndValidity();
      if (auto *currentForm = Form();
          currentForm && (oldType == ButtonType::Submit || _type == ButtonType::Submit))
      {
        currentForm->ResetDefaultButton();
      }
    }
  }

#pragma region HTMLButtonElement

  dom::DOMString HTMLButtonElement::Command() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"command").value_or(u8"");
  }

  void HTMLButtonElement::Command(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"command", krys::move(value));
  }

  bool HTMLButtonElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLButtonElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLButtonElement::FormAction() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"formaction");
  }

  dom::ExceptionOr<void> HTMLButtonElement::FormAction(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectSetter<dom::USVString>(*this, u8"formaction", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::EncType() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formenctype").value_or(u8"");
  }

  void HTMLButtonElement::EncType(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formenctype", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::FormMethod() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formmethod").value_or(u8"");
  }

  void HTMLButtonElement::FormMethod(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formmethod", krys::move(value));
  }

  void HTMLButtonElement::FormNoValidate(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"formnovalidate", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::FormTarget() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLButtonElement::FormTarget(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formtarget", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLButtonElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLButtonElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Value() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLButtonElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

#pragma endregion
}