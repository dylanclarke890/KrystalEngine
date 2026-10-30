#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/CommandEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/Rendering/RenderButton.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo::html
{
  namespace
  {
    struct CommandKeyword
    {
      utf8_stringview Keyword;
      dom::CommandType Type;
    };

    /// @brief The built-in commands' keywords.
    constexpr Array<CommandKeyword, 6> CommandKeywords = {{
      {u8"toggle-popover", dom::CommandType::TogglePopover},
      {u8"show-popover", dom::CommandType::ShowPopover},
      {u8"hide-popover", dom::CommandType::HidePopover},
      {u8"show-modal", dom::CommandType::ShowModal},
      {u8"close", dom::CommandType::Close},
      {u8"request-close", dom::CommandType::RequestClose},
    }};
  }

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
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }
    if (localName == u8"type")
    {
      ComputeType(value.value_or(dom::DOMStringView()));
    }
    else if (localName == u8"command" || localName == u8"commandfor")
    {
      ComputeType(GetAttribute(u8"type").value_or(dom::DOMString()));
    }
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

      if (_type == ButtonType::Button
          && !text::IsASCIICaselessEqual(
            dom::DOMStringView(GetAttribute(u8"type").value_or(dom::DOMString())),
            dom::DOMStringView(u8"button")))
      {
        return;
      }
    }

    if (CommandForElement())
    {
      HandleCommand();
      return;
    }

    HandlePopoverTargetAction(event.Target().get());
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
    else if (text::IsASCIICaselessEqual(typeAttributeValue, dom::DOMStringView(u8"submit")))
    {
      _type = ButtonType::Submit;
    }
    else if (HasAttribute(u8"command") || HasAttribute(u8"commandfor"))
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

  RefPtr<dom::Element> HTMLButtonElement::CommandForElement() const noexcept
  {
    return ElementForAttributeInternal(dom::DOMStringAtom(u8"commandfor"));
  }

  dom::CommandType HTMLButtonElement::GetCommandType() const noexcept
  {
    auto action = GetAttribute(u8"command");
    if (!action || action->empty())
    {
      return dom::CommandType::Invalid;
    }

    for (const auto &[keyword, type] : CommandKeywords)
    {
      if (text::IsASCIICaselessEqual(dom::DOMStringView(*action), dom::DOMStringView(keyword)))
      {
        return type;
      }
    }

    if (action->starts_with(u8"--"))
    {
      return dom::CommandType::Custom;
    }

    return dom::CommandType::Invalid;
  }

  void HTMLButtonElement::HandleCommand() noexcept
  {
    RefPtr<dom::Element> invokee = CommandForElement();
    if (!invokee)
    {
      return;
    }

    auto commandRaw = GetAttribute(u8"command");
    auto command = GetCommandType();

    if (command == dom::CommandType::Invalid)
    {
      return;
    }

    if (command != dom::CommandType::Custom && !invokee->IsValidCommandType(command))
    {
      return;
    }

    dom::CommandEventInit init;
    init.Bubbles = false;
    init.Cancelable = true;
    init.Source = ShareRefPtr<dom::Element>(this);
    init.Command = commandRaw.value_or(dom::DOMString());

    auto event = dom::CommandEvent::Create(dom::DOMStringAtom(EventNames::Command), init, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, invokee.get());

    if (!event->DefaultPrevented() && command != dom::CommandType::Custom)
    {
      (void)invokee->HandleCommandInternal(*this, command);
    }
  }

#pragma region HTMLButtonElement

  dom::DOMString HTMLButtonElement::Command() const noexcept
  {
    auto command = GetCommandType();
    if (command == dom::CommandType::Custom)
    {
      return GetAttribute(u8"command").value_or(dom::DOMString());
    }
    for (const auto &[keyword, type] : CommandKeywords)
    {
      if (type == command)
      {
        return dom::DOMString(keyword);
      }
    }
    return {};
  }

  void HTMLButtonElement::Command(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"command", krys::move(value));
  }

  RefPtr<dom::Element> HTMLButtonElement::CommandForElementForBindings() const noexcept
  {
    // FIXME: Retargeting against the button's tree, once there are reference targets.
    return CommandForElement();
  }

  void HTMLButtonElement::CommandForElementForBindings(dom::Element *element) noexcept
  {
    CEReactionsScope ceReactions;
    SetElementAttribute(dom::DOMStringAtom(u8"commandfor"), element);
  }

  bool HTMLButtonElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLButtonElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::USVString HTMLButtonElement::FormAction() const noexcept
  {
    // https://html.spec.whatwg.org/#dom-fs-formaction
    // Let attributeValue be the result of running get an attribute value given this and formaction. If
    // attributeValue is null or the empty string, return this's node document's URL, serialized.
    auto attributeValue = GetAttribute(u8"formaction");
    if (!attributeValue || attributeValue->empty())
    {
      return dom::USVString(NodeDocument().GetURL().String());
    }
    // Let urlString be the result of encoding-parsing-and-serializing a URL given attributeValue, relative to
    // this's node document. If urlString is not failure, return urlString.
    if (auto url = NodeDocument().CompleteURL(*attributeValue); url.IsValid())
    {
      return dom::USVString(url.String());
    }
    // Return attributeValue, converted to a scalar value string.
    return dom::USVString(krys::move(*attributeValue));
  }

  void HTMLButtonElement::FormAction(dom::USVString &&value) noexcept
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