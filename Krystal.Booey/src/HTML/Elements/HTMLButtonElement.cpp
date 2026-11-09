/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2018 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 * Copyright (C) 2007 Samuel Weinig (sam@webkit.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/CommandEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderButton.hpp"
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

  // https://html.spec.whatwg.org/#the-button-element:activation-behaviour
  void HTMLButtonElement::ActivationBehavior(dom::Event &event) noexcept
  {
    // 1. If element is disabled, then return.
    if (IsDisabledFormControl())
    {
      return;
    }

    // 2. If element's node document is not fully active, then return.
    if (!NodeDocument().IsFullyActive())
    {
      return;
    }

    // 3. If element has a form owner:
    if (Form())
    {
      // Update layout before processing form actions in case the style changes the form or button
      // relationships.
      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();
      if (RefPtr<HTMLFormElement> currentForm = ShareRefPtr(Form()))
      {
        // 3a. If element is a submit button, then submit element's form owner from element with
        //     userInvolvement set to event's user navigation involvement, and return.
        if (_type == ButtonType::Submit)
        {
          currentForm->SubmitIfPossible(&event, this);
        }
        // 3b. If element's type attribute is in the Reset Button state, then reset element's form owner, and
        //     return.
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

      // 3c. If element's type attribute is in the Auto state, then return.
      if (_type == ButtonType::Button
          && !text::IsASCIICaselessEqual(
            dom::DOMStringView(GetAttribute(u8"type").value_or(dom::DOMString())),
            dom::DOMStringView(u8"button")))
      {
        return;
      }
    }

    // 4. Let target be the result of running element's get the commandfor-associated element.
    // 5. If target is not null:
    if (CommandForElement())
    {
      HandleCommand();
      return;
    }

    // 6. Otherwise, run the popover target attribute activation behavior given element and event's target.
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

  void HTMLButtonElement::OnInsert() noexcept
  {
    HTMLFormControlElement::OnInsert();
    UpdateTypeForParent();
  }

  void HTMLButtonElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLFormControlElement::OnRemove(isSubtreeRoot, oldAncestor);
    UpdateTypeForParent();
  }

  void HTMLButtonElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLFormControlElement::OnMove(isSubtreeRoot, oldAncestor);
    UpdateTypeForParent();
  }

  void HTMLButtonElement::UpdateTypeForParent() noexcept
  {
    ComputeType(GetAttribute(u8"type").value_or(dom::DOMString()));
  }

  // https://html.spec.whatwg.org/#concept-submit-button
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
    else if (HasAttribute(u8"command") || HasAttribute(u8"commandfor") || Is<HTMLSelectElement>(ParentNode()))
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

  // https://html.spec.whatwg.org/#attr-button-command
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

    // A custom command keyword is a string that starts with "--".
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

    // 5a. Let command be element's command attribute.
    auto commandRaw = GetAttribute(u8"command");
    auto command = GetCommandType();

    // 5b. If the result of determining if a command is valid for a target given command and target is false,
    //     then return.
    // https://html.spec.whatwg.org/#determine-if-command-is-valid
    // 1. If command is in the Unknown state, then return false.
    if (command == dom::CommandType::Invalid)
    {
      return;
    }

    // 2. If command is in the Custom state, then return true.
    // 3. If target is not an HTML element, then return false.
    // 4. If command is in any of the following states: Toggle Popover; Show Popover; Hide Popover then return
    //    true.
    // 5. If this standard does not define is valid command steps for target's local name, then return false.
    // 6. Otherwise, return the result of running target's corresponding is valid command steps given command.
    // (Steps 3 to 6 are Element::IsValidCommandType and its overrides.)
    if (command != dom::CommandType::Custom && !invokee->IsValidCommandType(command))
    {
      return;
    }

    // 5c. Let continue be the result of firing an event named command at target, using CommandEvent, with its
    //     command attribute initialized to command, its source attribute initialized to element, and its
    //     cancelable attribute initialized to true.
    dom::CommandEventInit init;
    init.Bubbles = false;
    init.Cancelable = true;
    init.Source = ShareRefPtr<dom::Element>(this);
    init.Command = commandRaw.value_or(dom::DOMString());

    auto event = dom::CommandEvent::Create(dom::DOMStringAtom(EventNames::Command), init, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, invokee.get());

    // 5d. If continue is false, then return.
    // 5e. If target is not connected, then return.
    // 5f. If command is in the Custom state, then return.
    if (event->DefaultPrevented() || !invokee->IsConnected() || command == dom::CommandType::Custom)
    {
      return;
    }

    // 5g. If command is in the Hide Popover state: ...
    // 5h. Otherwise, if command is in the Toggle Popover state: ...
    // 5i. Otherwise, if command is in the Show Popover state: ...
    // 5j. Otherwise, if this standard defines command steps for target's local name, then run the
    //     corresponding command steps given target, element, and command.
    // (HTMLElement::HandleCommandInternal and its overrides; showing and hiding check the popover's
    // validity.)
    (void)invokee->HandleCommandInternal(*this, command);
  }

#pragma region HTMLButtonElement

  // https://html.spec.whatwg.org/#dom-button-command
  dom::DOMString HTMLButtonElement::Command() const noexcept
  {
    // 1. Let command be this's command attribute.
    auto command = GetCommandType();

    // 2. If command is in the Custom state, then return command's value.
    if (command == dom::CommandType::Custom)
    {
      return GetAttribute(u8"command").value_or(dom::DOMString());
    }

    // 3. If command is in the Unknown state, then return the empty string.
    // 4. Return the keyword corresponding to the value of command.
    for (const auto &[keyword, type] : CommandKeywords)
    {
      if (type == command)
      {
        return dom::DOMString(keyword);
      }
    }
    return {};
  }

  void HTMLButtonElement::Command(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"command", krys::move(value));
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
    return Reflect<bool>(*this, u8"disabled");
  }

  void HTMLButtonElement::Disabled(bool value) noexcept
  {
    Reflect<bool>(*this, u8"disabled", krys::move(value));
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

  void HTMLButtonElement::FormAction(dom::USVString value) noexcept
  {
    return ReflectSetter<dom::USVString>(*this, u8"formaction", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::EncType() const noexcept
  {
    return ReflectEnumerated<"formenctype">(*this);
  }

  void HTMLButtonElement::EncType(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"formenctype", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::FormMethod() const noexcept
  {
    return ReflectEnumerated<"formmethod">(*this);
  }

  void HTMLButtonElement::FormMethod(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"formmethod", krys::move(value));
  }

  void HTMLButtonElement::FormNoValidate(bool value) noexcept
  {
    Reflect<bool>(*this, u8"formnovalidate", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::FormTarget() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLButtonElement::FormTarget(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"formtarget", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLButtonElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-button-type
  dom::DOMString HTMLButtonElement::Type() const noexcept
  {
    // 1. If this is a submit button, then return "submit".
    // 2. Let state be this's type attribute.
    // 3. Assert: state is not in the Submit Button state.
    // 4. If state is in the Auto state, then return "button".
    // 5. Return the keyword value corresponding to state.
    return FormControlType();
  }

  void HTMLButtonElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Value() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLButtonElement::Value(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

#pragma endregion
}