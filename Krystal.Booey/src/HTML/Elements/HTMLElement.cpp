/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2021-2024 Google Inc. All rights reserved.
 * Copyright (C) 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2011 Motorola Mobility. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/SimulatedClick.hpp"
#include "Krystal.Booey/DOM/Events/ToggleEvent.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Nodes/TextDirectionState.hpp"
#include "Krystal.Booey/DOM/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/Collections/LabelsNodeList.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Forms/FormAssociatedElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Dimensions.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include <initializer_list>

namespace krys::boo::html
{
  HTMLElement::HTMLElement(dom::Document &document, HTMLElementInterface interface, HTMLElementFlags flags,
                           dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document, dom::NullQualifiedName(), nodeFlags | dom::NodeFlags::IsHTMLElement),
        _flags(flags), _interface(interface)
  {
  }

  HTMLElement::HTMLElement(dom::Document &document, HTMLElementInterface interface,
                           dom::DOMStringAtom localName, HTMLElementFlags flags,
                           dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document,
                      dom::QualifiedName(dom::DOMStringAtom(infra::Namespaces::HTML),
                                         dom::DOMStringAtom::Null(), localName),
                      nodeFlags | dom::NodeFlags::IsHTMLElement),
        _flags(flags), _interface(interface)
  {
  }

  HTMLElement::HTMLElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::None)
  {
  }

#pragma region HTMLElement

  dom::DOMString HTMLElement::Title() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"title").value_or(u8"");
  }

  void HTMLElement::Title(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"title", krys::move(value));
  }

  dom::DOMString HTMLElement::Lang() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"lang").value_or(u8"");
  }

  void HTMLElement::Lang(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"lang", krys::move(value));
  }

  bool HTMLElement::Translate() const noexcept
  {
    // The translate IDL attribute must, on getting, return true if the element's translation mode is
    // translate-enabled, and false otherwise.
    return IsTranslateEnabled(*this);
  }

  bool HTMLElement::IsTranslateEnabled(const dom::Element &element) noexcept
  {
    for (auto *ancestor = &element; ancestor; ancestor = ancestor->ParentElement())
    {
      // If an HTML element's translate attribute is in the Yes state, then the element's translation mode is
      // in the translate-enabled state; otherwise, if the element's translate attribute is in the No state,
      // then the element's translation mode is in the no-translate state.
      if (auto *htmlElement = DynamicDowncast<HTMLElement>(*ancestor))
      {
        auto state = EnumeratedAttribute<"translate", HTMLElement>::ResolveState(
          Reflect<dom::DOMString>(*htmlElement, u8"translate"));
        if (state == TranslateAttributeState::Yes)
        {
          return true;
        }
        if (state == TranslateAttributeState::No)
        {
          return false;
        }
      }

      // Otherwise, either the element's translate attribute is in the Inherit state, or the element is not
      // an HTML element and thus does not have a translate attribute; in either case, the element's
      // translation mode is in the same state as its parent element's, if any, or in the translate-enabled
      // state, if the element's parent element is null.
    }
    return true;
  }

  void HTMLElement::Translate(bool value) noexcept
  {
    // On setting, it must set the content attribute's value to "yes" if the new value is true, and set the
    // content attribute's value to "no" otherwise.
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"translate", value ? u8"yes" : u8"no");
  }

  dom::DOMString HTMLElement::Dir() const noexcept
  {
    return ReflectEnumerated<"dir">(*this);
  }

  void HTMLElement::Dir(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"dir", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-hidden
  BoolOr<dom::DOMString> HTMLElement::Hidden() const noexcept
  {
    auto value = Reflect<dom::DOMString>(*this, u8"hidden");

    // 1. If the hidden attribute is in the Hidden Until Found state, then return "until-found".
    auto state = EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(value);
    if (state == HiddenAttributeState::UntilFound)
    {
      return u8"until-found";
    }

    // 2. If the hidden attribute is set, then return true.
    // 3. Return false.
    // (Any other value, and the missing value, is in the Hidden state.)
    return state == HiddenAttributeState::Hidden;
  }

  bool HTMLElement::IsHiddenUntilFound() const noexcept
  {
    return EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(GetAttribute(u8"hidden"))
           == HiddenAttributeState::UntilFound;
  }

  // https://html.spec.whatwg.org/#dom-hidden
  // (The setter's steps are split between the overloads by the given value's type; a null value is the
  // empty string's overload.)
  void HTMLElement::Hidden(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    // 1. If the given value is a string that is an ASCII case-insensitive match for "until-found", then set
    //    the hidden attribute to "until-found".
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"until-found"))
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"until-found");
    }
    // 3. Otherwise, if the given value is the empty string, then remove the hidden attribute.
    else if (value.empty())
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    // 7. Otherwise, set the hidden attribute to the empty string.
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  // https://html.spec.whatwg.org/#dom-hidden
  void HTMLElement::Hidden(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    // 7. Otherwise, set the hidden attribute to the empty string.
    if (value)
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
    // 2. Otherwise, if the given value is false, then remove the hidden attribute.
    else
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
  }

  // https://html.spec.whatwg.org/#dom-hidden
  void HTMLElement::Hidden(double value) noexcept
  {
    CEReactionsScope ceReactions;
    // 5. Otherwise, if the given value is 0, then remove the hidden attribute.
    if (value == 0)
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    // 6. Otherwise, if the given value is NaN, then remove the hidden attribute.
    else if (std::isnan(value))
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    // 7. Otherwise, set the hidden attribute to the empty string.
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  bool HTMLElement::Inert() const noexcept
  {
    return Reflect<bool>(*this, u8"inert");
  }

  void HTMLElement::Inert(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<bool>(*this, u8"inert", krys::move(value));
  }

  bool HTMLElement::CanBeActuallyDisabled() const noexcept
  {
    if (auto *customElement = DynamicDowncast<HTMLMaybeFormAssociatedCustomElement>(*this))
    {
      return customElement->IsFormAssociatedCustomElement();
    }
    return IsHTMLButtonElement() || IsHTMLInputElement() || IsHTMLSelectElement() || IsHTMLTextAreaElement()
           || IsHTMLOptGroupElement() || IsHTMLOptionElement() || IsHTMLFieldSetElement();
  }

  // https://html.spec.whatwg.org/#concept-element-disabled
  bool HTMLElement::IsActuallyDisabled() const noexcept
  {
    // An element is said to be actually disabled if it is one of the following: a button, input, select or
    // textarea element that is disabled; an optgroup element that has a disabled attribute or whose nearest
    // ancestor select is disabled; an option element that is disabled or whose nearest ancestor select is
    // disabled; a fieldset element that is a disabled fieldset (HTMLFieldSetElement's); a form-associated
    // custom element that is disabled.
    if (!CanBeActuallyDisabled())
    {
      return false;
    }
    if (IsDisabledFormControl())
    {
      return true;
    }
    if (IsHTMLOptGroupElement() || IsHTMLOptionElement())
    {
      for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (auto *select = DynamicDowncast<HTMLSelectElement>(*ancestor))
        {
          return select->IsDisabledFormControl();
        }
      }
    }
    return false;
  }

  bool HTMLElement::MatchesReadWritePseudoClass() const noexcept
  {
    return EditabilityFromContentEditableAttr(*this) != dom::Node::Editability::ReadOnly;
  }

  RefPtr<dom::NodeList> HTMLElement::Labels() noexcept
  {
    if (!IsLabelable())
    {
      return nullptr;
    }

    auto &cachedLabels = EnsureNodeRareData().LabelsNodeList();
    if (auto labels = cachedLabels.lock())
    {
      return labels;
    }
    Ref<dom::NodeList> labels = CreateRef<LabelsNodeList>(*this);
    cachedLabels = CreateWeakPtr<dom::NodeList>(labels.get());
    return labels;
  }

  // https://html.spec.whatwg.org/#dom-click
  // 1. If this element is a form control that is disabled, then return.
  // 2. If this element's click in progress flag is set, then return.
  // 3. Set this element's click in progress flag.
  // 4. Fire a synthetic pointer event named click at this element, with the not trusted flag set.
  // 5. Unset this element's click in progress flag.
  // (SimulateClick does each, with a set of the elements being clicked for the flags.)
  void HTMLElement::Click() noexcept
  {
    (void)dom::SimulateClick(*this, nullptr, dom::SimulatedClickMouseEventOptions::SendNoEvents,
                             dom::SimulatedClickVisualOptions::DoNotShowPressedLook,
                             dom::SimulatedClickSource::Bindings);
  }

  dom::DOMString HTMLElement::AccessKey() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"accesskey").value_or(u8"");
  }

  void HTMLElement::AccessKey(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"accesskey", krys::move(value));
  }

  bool HTMLElement::AccessKeyAction(bool sendMouseEvents) noexcept
  {
    (void)StyledElement::AccessKeyAction(sendMouseEvents);
    return DispatchSimulatedClick(nullptr, sendMouseEvents
                                             ? dom::SimulatedClickMouseEventOptions::SendMouseUpDownEvents
                                             : dom::SimulatedClickMouseEventOptions::SendNoEvents);
  }

  dom::DOMString HTMLElement::AccessKeyLabel() const noexcept
  {
    // The modifier is the one EventHandler::HandleAccessKey looks for.
    auto key = AssignedAccessKey();
    return key.empty() ? key : dom::DOMString(u8"Alt+") + key;
  }

  bool HTMLElement::Draggable() const noexcept
  {
    auto value = Reflect<dom::DOMString>(*this, u8"draggable");
    switch (EnumeratedAttribute<"draggable", HTMLElement>::ResolveState(value))
    {
      case DraggableAttributeState::True:  return true;
      case DraggableAttributeState::False: return false;
      case DraggableAttributeState::Auto:
      {
        // If the element is an img element, an object element that represents an image, or an a element with
        // an href content attribute, the draggable IDL attribute must return true; otherwise, the draggable
        // IDL attribute must return false.
        // FIXME: An object element that represents an image, once objects show images.
        return IsHTMLImageElement() || (IsHTMLAnchorElement() && HasAttribute(u8"href"));
      }
    }

    return false;
  }

  void HTMLElement::Draggable(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"draggable", value ? u8"true" : u8"false");
  }

  namespace
  {
    /// @brief The form owner of @p element if it's an autocapitalize-and-autocorrect inheriting element.
    /// @see https://html.spec.whatwg.org/#category-autocapitalize
    KRYS_NODISCARD HTMLFormElement *AutocapitalizeAndAutocorrectFormOwner(const HTMLElement &element) noexcept
    {
      switch (element.TagQualifiedName().TagName())
      {
        case TagName::button:
        case TagName::fieldset:
        case TagName::input:
        case TagName::output:
        case TagName::select:
        case TagName::textarea: break;
        default:                return nullptr;
      }
      auto *formAssociated = const_cast<HTMLElement &>(element).AsFormAssociatedElement();
      return formAssociated ? formAssociated->Form() : nullptr;
    }

    /// @see https://html.spec.whatwg.org/#own-autocapitalization-hint
    KRYS_NODISCARD AutocapitalizeAttributeState OwnAutocapitalizationHint(const HTMLElement &element) noexcept
    {
      // 1. If the autocapitalize content attribute is present on element, and its value is not the empty
      //    string, return the state of the attribute.
      auto value = element.GetAttribute(u8"autocapitalize");
      if (value && !value->empty())
      {
        return EnumeratedAttribute<"autocapitalize", HTMLElement>::ResolveState(value);
      }
      // 2. If element is an autocapitalize-and-autocorrect inheriting element and has a non-null form owner,
      //    return the own autocapitalization hint of element's form owner.
      if (auto *form = AutocapitalizeAndAutocorrectFormOwner(element))
      {
        return OwnAutocapitalizationHint(*form);
      }
      // 3. Return Default.
      return AutocapitalizeAttributeState::Default;
    }

    /// @see https://html.spec.whatwg.org/#used-autocorrection-state
    KRYS_NODISCARD AutocorrectAttributeState UsedAutocorrectionState(const HTMLElement &element) noexcept
    {
      // 1. If element is an input element whose type attribute is in one of the URL, Email, or Password
      //    states, then return Off.
      if (auto *input = DynamicDowncast<HTMLInputElement>(element);
          input && (input->IsURLField() || input->IsEmailField() || input->IsPasswordField()))
      {
        return AutocorrectAttributeState::Off;
      }
      // 2. If the autocorrect content attribute is present on element, then return the state of the
      //    attribute.
      if (auto value = element.GetAttribute(u8"autocorrect"))
      {
        return EnumeratedAttribute<"autocorrect", HTMLElement>::ResolveState(value);
      }
      // 3. If element is an autocapitalize-and-autocorrect inheriting element and has a non-null form owner,
      //    then return the state of element's form owner's autocorrect attribute.
      if (auto *form = AutocapitalizeAndAutocorrectFormOwner(element))
      {
        return EnumeratedAttribute<"autocorrect", HTMLElement>::ResolveState(
          form->GetAttribute(u8"autocorrect"));
      }
      // 4. Return On.
      return AutocorrectAttributeState::On;
    }
  }

  // https://html.spec.whatwg.org/#dom-spellcheck
  bool HTMLElement::SpellCheck() const noexcept
  {
    // The spellcheck IDL attribute, on getting, must return true if the element's spellcheck content
    // attribute is in the True state, or if the element's spellcheck content attribute is in the Default
    // state and the element's default behavior is true-by-default, or if the element's spellcheck content
    // attribute is in the Default state and the element's default behavior is inherit-by-default and the
    // element's parent element's spellcheck IDL attribute would return true; otherwise, if none of those
    // conditions applies, then the attribute must instead return false. (Every element's default behavior is
    // inherit-by-default, but true-by-default without a parent element.)
    for (const dom::Element *element = this; element; element = element->ParentElement())
    {
      switch (
        EnumeratedAttribute<"spellcheck", HTMLElement>::ResolveState(element->GetAttribute(u8"spellcheck")))
      {
        case SpellcheckAttributeState::True:    return true;
        case SpellcheckAttributeState::False:   return false;
        case SpellcheckAttributeState::Default: break;
      }
    }
    return true;
  }

  // https://html.spec.whatwg.org/#dom-spellcheck
  void HTMLElement::SpellCheck(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    // On setting, if the new value is true, then the element's spellcheck content attribute must be set to
    // "true", otherwise it must be set to "false".
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"spellcheck", value ? u8"true" : u8"false");
  }

  // https://html.spec.whatwg.org/#dom-writingsuggestions
  dom::DOMString HTMLElement::WritingSuggestions() const noexcept
  {
    // Return this's computed writing suggestions value.
    // https://html.spec.whatwg.org/#computed-writing-suggestions-value
    for (const dom::Element *element = this; element; element = element->ParentElement())
    {
      switch (EnumeratedAttribute<"writingsuggestions", HTMLElement>::ResolveState(
        element->GetAttribute(u8"writingsuggestions")))
      {
        // 1. If element's writingsuggestions content attribute is in the False state, return "false".
        case WritingSuggestionsAttributeState::False:   return u8"false";
        // 2. If element's writingsuggestions content attribute is in the Default state, element has a parent
        //    element, and the computed writing suggestions value of element's parent element is "false", then
        //    return "false".
        case WritingSuggestionsAttributeState::Default: break;
        // 3. Return "true".
        case WritingSuggestionsAttributeState::True:    return u8"true";
      }
    }
    return u8"true";
  }

  void HTMLElement::WritingSuggestions(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    ReflectSetter(*this, u8"writingsuggestions", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-autocapitalize
  dom::DOMString HTMLElement::Autocapitalize() const noexcept
  {
    // 1. Let state be the own autocapitalization hint of this.
    switch (OwnAutocapitalizationHint(*this))
    {
      // 2. If state is Default, then return the empty string.
      case AutocapitalizeAttributeState::Default:    return {};
      // 3. If state is None, then return "none".
      case AutocapitalizeAttributeState::None:       return u8"none";
      // 4. If state is Sentences, then return "sentences".
      case AutocapitalizeAttributeState::Sentences:  return u8"sentences";
      // 5. Return the keyword value corresponding to state.
      case AutocapitalizeAttributeState::Words:      return u8"words";
      case AutocapitalizeAttributeState::Characters: return u8"characters";
    }
    krys_unreachable();
  }

  void HTMLElement::Autocapitalize(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    ReflectSetter(*this, u8"autocapitalize", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-autocorrect
  bool HTMLElement::Autocorrect() const noexcept
  {
    // The autocorrect getter steps are: return true if the element's used autocorrection state is On and
    // false if the element's used autocorrection state is Off.
    return UsedAutocorrectionState(*this) == AutocorrectAttributeState::On;
  }

  // https://html.spec.whatwg.org/#dom-autocorrect
  void HTMLElement::Autocorrect(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    // The setter steps are: if the given value is true, then the element's autocorrect attribute must be set
    // to "on"; otherwise it must be set to "off".
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"autocorrect", value ? u8"on" : u8"off");
  }

  // https://html.spec.whatwg.org/#dom-inputmode
  // The inputMode IDL attribute must reflect the inputmode content attribute, limited to only known values.
  dom::DOMString HTMLElement::InputMode() const noexcept
  {
    return ReflectEnumerated<"inputmode">(*this);
  }

  // https://html.spec.whatwg.org/#dom-inputmode
  void HTMLElement::InputMode(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"inputmode", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-enterkeyhint
  // The enterKeyHint IDL attribute must reflect the enterkeyhint content attribute, limited to only known
  // values.
  dom::DOMString HTMLElement::EnterKeyHint() const noexcept
  {
    return ReflectEnumerated<"enterkeyhint">(*this);
  }

  // https://html.spec.whatwg.org/#dom-enterkeyhint
  void HTMLElement::EnterKeyHint(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"enterkeyhint", krys::move(value));
  }

  namespace
  {
    /// @see https://html.spec.whatwg.org/#rendered-text-fragment
    KRYS_NODISCARD Ref<dom::DocumentFragment> RenderedTextFragment(dom::DOMStringView input,
                                                                   dom::Document &document) noexcept
    {
      // 1. Let fragment be the result of creating a document fragment given document.
      auto fragment = document.CreateDocumentFragment();

      // 2. Let position be a position variable for input, initially pointing at the start of input.
      size_t position = 0;

      // 3. Let text be the empty string.
      // 4. While position is not past the end of input:
      while (position < input.size())
      {
        // 4a. Collect a sequence of code points that are not U+000A LF or U+000D CR from input given
        // position,
        //     and set text to the result.
        auto end = std::min(input.find_first_of(u8"\r\n", position), input.size());
        auto text = input.substr(position, end - position);
        position = end;

        // 4b. If text is not the empty string, then append a new Text node whose data is text and node
        // document
        //     is document to fragment.
        if (!text.empty())
        {
          (void)fragment->AppendChild(*document.CreateTextNode(dom::DOMString(text)));
        }

        // 4c. While position is not past the end of input, and the code point at position is either U+000A LF
        //     or U+000D CR:
        while (position < input.size() && (input[position] == u8'\n' || input[position] == u8'\r'))
        {
          // 4c.i. If the code point at position is U+000D CR and the next code point is U+000A LF, then
          // advance
          //       position to the next code point in input.
          if (input[position] == u8'\r' && position + 1 < input.size() && input[position + 1] == u8'\n')
          {
            ++position;
          }

          // 4c.ii. Advance position to the next code point in input.
          ++position;

          // 4c.iii. Append the result of creating an element given document, "br", and the HTML namespace to
          //         fragment.
          (void)fragment->AppendChild(
            *dom::ElementFactory::CreateHTMLElement<HTMLBRElement>(document, u8"br"));
        }
      }

      // 5. Return fragment.
      return fragment;
    }

    /// @see https://html.spec.whatwg.org/#merge-with-the-next-text-node
    void MergeWithNextTextNode(dom::Text &node) noexcept
    {
      // 1. Let next be node's next sibling.
      // 2. If next is not a Text node, then return.
      RefPtr<dom::Text> next = ShareRefPtr(DynamicDowncast<dom::Text>(node.NextSibling()));
      if (!next)
      {
        return;
      }

      // 3. Replace data with node, node's data's length, 0, and next's data.
      (void)node.ReplaceData(node.Length(), 0, next->Data());

      // 4. Remove next.
      (void)next->Remove();
    }
  }

  dom::DOMString HTMLElement::InnerText() const noexcept
  {
    // The innerText and outerText getter steps are to return the result of running get the text steps with
    // this.
    auto &self = const_cast<HTMLElement &>(*this);

    // The rendered text comes from the line boxes, so layout must be up to date.
    self.NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    // 1. If element is not being rendered or if the user agent is a non-CSS user agent, then return element's
    //    descendant text content.
    if (!Renderer())
    {
      return TextContent().value_or(dom::DOMString {});
    }

    // Skipped content (content-visibility) isn't rendered, but it isn't the descendant text content either.
    if (Renderer()->IsSkippedContent())
    {
      return {};
    }

    // 2. Let results be a new empty list.
    // 3. For each child node node of element:
    // 3a. Let current be the list resulting from running the rendered text collection steps with node.
    // 3b. For each item item in current, append item to results.
    // 4. Remove any items from results that are the empty string.
    // 5. Remove any runs of consecutive required line break count items at the start or end of results.
    // 6. Replace each remaining run of consecutive required line break count items with a string consisting
    // of
    //    as many U+000A LF code points as the maximum of the values in the required line break count items.
    // 7. Return the concatenation of the string items in results.
    // (The text iterator walks the rendered text, approximating the rendered text collection steps.)
    return PlainText(dom::MakeRangeSelectingNodeContents(self));
  }

  void HTMLElement::InnerText(dom::DOMString value) noexcept
  {
    // The innerText setter steps are to run set the inner text steps with this and the given value.
    CEReactionsScope ceReactions;

    // A text control's inner element holds its value as one text node, with its line breaks normalised.
    if (IsConnected() && IsTextControlInnerTextElement())
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(infra::StringAlgorithms::NormalizeNewlines(value), *this);
      return;
    }

    // 1. Let fragment be the rendered text fragment for value given element's node document.
    auto fragment = RenderedTextFragment(value, NodeDocument());

    // 2. Replace all with fragment within element.
    (void)dom::MutationAlgorithms::ReplaceAll(fragment.get(), *this);
  }

  dom::DOMString HTMLElement::OuterText() const noexcept
  {
    return InnerText();
  }

  dom::ExceptionOr<void> HTMLElement::OuterText(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;

    // 1. If this's parent is null, then throw a "NoModificationAllowedError" DOMException.
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(ParentNode());
    if (!parent)
    {
      return dom::ExceptionCode::NoModificationAllowedError;
    }

    // 2. Let next be this's next sibling.
    RefPtr<dom::Node> next = ShareRefPtr(NextSibling());

    // 3. Let previous be this's previous sibling.
    RefPtr<dom::Node> previous = ShareRefPtr(PreviousSibling());

    // 4. Let fragment be the rendered text fragment for the given value given this's node document.
    auto fragment = RenderedTextFragment(value, NodeDocument());

    // 5. If fragment has no children, then append a new Text node whose data is the empty string and node
    //    document is this's node document to fragment.
    if (!fragment->HasChildNodes())
    {
      (void)fragment->AppendChild(*NodeDocument().CreateTextNode(dom::DOMString()));
    }

    // 6. Replace this with fragment within this's parent.
    if (auto replace = dom::MutationAlgorithms::Replace(*this, *fragment, *parent); replace.HasException())
    {
      return replace.ReleaseException();
    }

    // 7. If next is non-null and next's previous sibling is a Text node, then merge with the next text node
    //    given next's previous sibling.
    if (next)
    {
      if (auto *text = DynamicDowncast<dom::Text>(next->PreviousSibling()))
      {
        MergeWithNextTextNode(*text);
      }
    }

    // 8. If previous is a Text node, then merge with the next text node given previous.
    if (auto *text = DynamicDowncast<dom::Text>(previous.get()))
    {
      MergeWithNextTextNode(*text);
    }

    return {};
  }

  // https://html.spec.whatwg.org/#dom-attachinternals
  dom::ExceptionOr<Ref<ElementInternals>> HTMLElement::AttachInternals() noexcept
  {
    // 1. If this's is value is not null, then throw a "NotSupportedError" DOMException.
    if (IsValue() != dom::DOMStringAtom::Null())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 2. Let definition be the result of looking up a custom element definition given this's custom element
    //    registry, this's namespace, this's local name, and null.
    // 3. If definition is null, then throw a "NotSupportedError" DOMException.
    // (The definition is the reaction queue's, which only an element with one has.)
    auto *queue = ReactionQueue();
    if (!queue)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 4. If definition's disable internals is true, then throw a "NotSupportedError" DOMException.
    if (queue->IsElementInternalsDisabled())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 5. If this's attached internals is non-null, then throw a "NotSupportedError" DOMException.
    if (queue->AttachedInternals())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 6. If this's custom element state is not "precustomized" or "custom", then throw a "NotSupportedError"
    //    DOMException.
    if (!IsPrecustomizedOrDefinedCustomElement())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // 7. Set this's attached internals to a new ElementInternals instance whose target element is this.
    // 8. Return this's attached internals.
    auto internals = ElementInternals::Create(*this);
    queue->SetAttachedInternals(internals);
    return internals;
  }

  // https://html.spec.whatwg.org/#dom-showpopover
  dom::ExceptionOr<void> HTMLElement::ShowPopover(const ShowPopoverOptions &options) noexcept
  {
    // 1. Let source be options["source"] if it exists; otherwise, null.
    // 2. Run show popover given this, true, and source.
    return ShowPopoverInternal(options.Source.get());
  }

  // https://html.spec.whatwg.org/#dom-hidepopover
  // The hidePopover() method steps are: Run the hide popover algorithm given this, true, true, true, and
  // null.
  dom::ExceptionOr<void> HTMLElement::HidePopover() noexcept
  {
    return HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, true, nullptr);
  }

  // https://html.spec.whatwg.org/#dom-togglepopover
  dom::ExceptionOr<bool> HTMLElement::TogglePopover(const TogglePopoverOptionsOrBool &options) noexcept
  {
    // 1. Let force be null.
    Maybe<bool> force;
    HTMLElement *source = nullptr;
    // 2. If options is a boolean, set force to options.
    if (auto *value = std::get_if<bool>(&options))
    {
      force = *value;
    }
    // 3. Otherwise, if options["force"] exists, set force to options["force"].
    // 4. Let source be options["source"] if it exists; otherwise, null.
    else
    {
      auto &dictionary = std::get<TogglePopoverOptions>(options);
      force = dictionary.Force;
      source = dictionary.Source.get();
    }

    // 5. If this's popover visibility state is showing, and force is null or false, then run the hide popover
    //    algorithm given this, true, true, true, and null.
    if (IsPopoverShowing() && !force.value_or(false))
    {
      if (auto result =
            HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, true, nullptr);
          result.HasException())
      {
        return result.ReleaseException();
      }
    }
    // 6. Otherwise, if force is null or true, then run show popover given this, true, and source.
    else if (force.value_or(true))
    {
      if (auto result = ShowPopoverInternal(source); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    // 7. Otherwise:
    // 7a. Let expectedToBeShowing be true if this's popover visibility state is showing; otherwise false.
    // 7b. Run check popover validity given this, expectedToBeShowing, and null.
    else
    {
      if (auto check = CheckPopoverValidity(IsPopoverShowing(), nullptr); check.HasException())
      {
        return check.ReleaseException();
      }
    }
    // 8. Return true if this's popover visibility state is showing; otherwise false.
    return IsPopoverShowing();
  }

  Maybe<dom::DOMString> HTMLElement::Popover() const noexcept
  {
    return ReflectEnumerated<"popover", HTMLElement, Maybe<dom::DOMString>>(*this);
  }

  void HTMLElement::Popover(Maybe<dom::DOMString> value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<Maybe<dom::DOMString>>(*this, u8"popover", krys::move(value));
  }

  uint32 HTMLElement::HeadingOffset() const noexcept
  {
    return Reflect<uint32, ReflectRange<uint32>(0u, 8u)>(*this, u8"headingoffset");
  }

  void HTMLElement::HeadingOffset(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"headingoffset", krys::move(value));
  }

  bool HTMLElement::HeadingReset() const noexcept
  {
    return Reflect<bool>(*this, u8"headingreset");
  }

  void HTMLElement::HeadingReset(bool value) noexcept
  {
    Reflect<bool>(*this, u8"headingreset", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  namespace
  {
    KRYS_NODISCARD css::ValueId UnicodeBidiAttributeForDirAuto(const HTMLElement &element) noexcept
    {
      auto tagName = element.TagQualifiedName().TagName();
      if (tagName == TagName::pre || tagName == TagName::textarea)
      {
        return css::ValueId::Plaintext;
      }
      return css::ValueId::Isolate;
    }
  }

#pragma region Popovers

  namespace
  {
    /// @brief What a popover algorithm returns once @p validity says the popover can't be shown or hidden.
    KRYS_NODISCARD dom::ExceptionOr<void> InvalidPopoverResult(dom::ExceptionOr<bool> &&validity,
                                                               bool throwExceptions) noexcept
    {
      if (throwExceptions && validity.HasException())
      {
        return validity.ReleaseException();
      }
      return {};
    }

    KRYS_NODISCARD bool IsValid(const dom::ExceptionOr<bool> &validity) noexcept
    {
      return validity.HasValue() && validity.Value();
    }

    KRYS_NODISCARD bool ListContains(const List<Ref<HTMLElement>> &list, const HTMLElement &element) noexcept
    {
      return std::ranges::find(list, &element, [](auto &item) { return item.get(); }) != list.end();
    }
  }

  // https://html.spec.whatwg.org/#show-popover
  dom::ExceptionOr<void> HTMLElement::ShowPopoverInternal(HTMLElement *source, bool throwExceptions) noexcept
  {
    // 1. Let document be element's node document.
    Ref<dom::Document> document = ShareRef(NodeDocument());

    // 2. If document's showing popover is true, or document's hiding popover nesting count is not 0:
    // 2a. If throwExceptions is true, then throw a "InvalidStateError" DOMException.
    // 2b. Return.
    if (document->IsShowingPopover() || document->HidingPopoverNestingCount())
    {
      if (throwExceptions)
      {
        return dom::Exception {dom::ExceptionCode::InvalidStateError,
                               u8"Cannot show a popover while showing or hiding a popover."};
      }
      return {};
    }

    // 3. Let validityResult be the result of running check popover validity given element, false, and null.
    // 4. If validityResult is not true: if throwExceptions is true and validityResult is a DOMException, then
    //    throw validityResult; return.
    auto validity = CheckPopoverValidity(false, nullptr);
    if (!IsValid(validity))
    {
      return InvalidPopoverResult(krys::move(validity), throwExceptions);
    }

    // 5. Assert: element's popover trigger is null.
    // 6. Assert: element is not in document's top layer.
    // (It can be pending removal from the top layer, with its overlay transition.)
    krys_debug_assert(!GetPopoverData()->Invoker());
    krys_debug_assert(!IsInTopLayer() || document->IsPendingTopLayerRemoval(*this));

    Ref<HTMLElement> protectedThis = ShareRef(*this);
    // 7. Set document's showing popover to true.
    document->SetIsShowingPopover(true);
    // 8. Let cleanupShowingSteps be the following steps:
    // 8a. Set document's showing popover to false.
    auto cleanupShowingSteps = [&]
    {
      document->SetIsShowingPopover(false);
    };

    // 9. If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable
    // attribute
    //    initialized to true, the oldState attribute initialized to "closed", the newState attribute
    //    initialized to "open", and the source attribute initialized to source at element is false, then run
    //    cleanupShowingSteps and return.
    if (!FirePopoverBeforeToggleEvent(dom::ToggleState::Closed, dom::ToggleState::Open, true, source))
    {
      cleanupShowingSteps();
      return {};
    }

    // 10. Set validityResult to the result of running check popover validity given element, false, and
    //     document.
    // 11. If validityResult is not true: run cleanupShowingSteps; if throwExceptions is true and
    //     validityResult is a DOMException, then throw validityResult; return.
    validity = CheckPopoverValidity(false, document.get());
    if (!IsValid(validity))
    {
      cleanupShowingSteps();
      return InvalidPopoverResult(krys::move(validity), throwExceptions);
    }

    // 12. Let shouldRestoreFocus be false.
    bool shouldRestoreFocus = false;
    // 13. Let originalType be the current state of element's popover attribute.
    auto originalType = GetPopoverState();
    // 14. Let effectiveType be originalType.
    auto effectiveType = originalType;
    RefPtr<HTMLElement> ancestor;
    // 15. If originalType is Auto or Hint:
    if (originalType == dom::PopoverState::Auto || originalType == dom::PopoverState::Hint)
    {
      // 15a. Let ancestor be the result of running the topmost popover ancestor algorithm given element,
      //      source, and true.
      ancestor = TopmostPopoverAncestor(source, true);

      // 15b. If all of the following are true: ancestor is not null; ancestor's opened in popover mode is
      //      "hint"; and effectiveType is the Auto state, then set effectiveType to the Hint state.
      if (ancestor && ancestor->GetPopoverData()->OpenedInPopoverMode() == dom::PopoverState::Hint
          && effectiveType == dom::PopoverState::Auto)
      {
        effectiveType = dom::PopoverState::Hint;
      }

      // 15c. Run hide popover stack until given document, ancestor, Hint, shouldRestoreFocus, and true.
      document->HidePopoverStackUntil(ancestor.get(), dom::PopoverState::Hint, dom::FocusPreviousElement::No,
                                      dom::FireEvents::Yes);
      // 15d. If effectiveType is the Auto state, then run hide popover stack until given document, ancestor,
      //      Auto, shouldRestoreFocus, and true.
      if (effectiveType == dom::PopoverState::Auto)
      {
        document->HidePopoverStackUntil(ancestor.get(), dom::PopoverState::Auto,
                                        dom::FocusPreviousElement::No, dom::FireEvents::Yes);
      }

      // 15e. If originalType is not equal to the value of element's popover attribute: run
      //      cleanupShowingSteps; if throwExceptions is true, then throw an "InvalidStateError" DOMException;
      //      return.
      if (originalType != GetPopoverState())
      {
        cleanupShowingSteps();
        if (throwExceptions)
        {
          return dom::Exception {
            dom::ExceptionCode::InvalidStateError,
            u8"The value of the popover attribute was changed while hiding the popover."};
        }
        return {};
      }

      // 15f. Set validityResult to the result of running check popover validity given element, false, and
      //      document.
      // 15g. If validityResult is not true: run cleanupShowingSteps; if throwExceptions is true and
      //      validityResult is a DOMException, then throw validityResult; return.
      validity = CheckPopoverValidity(false, document.get());
      if (!IsValid(validity))
      {
        cleanupShowingSteps();
        return InvalidPopoverResult(krys::move(validity), throwExceptions);
      }

      // 15h. If the result of running topmost auto or hint popover on document is null, then set
      //      shouldRestoreFocus to true.
      if (!document->TopmostAutoOrHintPopover())
      {
        shouldRestoreFocus = true;
      }

      // 15i. If effectiveType is Auto: set element's opened in popover mode to "auto".
      // 15j. Otherwise: set element's opened in popover mode to "hint".
      GetPopoverData()->SetOpenedInPopoverMode(effectiveType);
      // 15k. Set element's popover close watcher to the result of establishing a close watcher given
      // element's
      //      relevant global object, with: cancelAction being to return true; closeAction being to hide a
      //      popover given element, true, true, false, and null; getEnabledState being to return true.
      if (document->Window())
      {
        auto weakThis = CreateWeakPtr(this);
        GetPopoverData()->SetCloseWatcher(CloseWatcher::Establish(
          *document, [](bool) { return true; },
          [weakThis]
          {
            if (RefPtr<HTMLElement> popover = weakThis.lock())
            {
              (void)popover->HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, false,
                                                 nullptr);
            }
          },
          [] { return true; }));
      }
    }

    // 16. Set element's previously focused element to null.
    GetPopoverData()->SetPreviouslyFocusedElement(nullptr);
    // 17. Let originallyFocusedElement be document's focused area of the document's DOM anchor.
    RefPtr<dom::Element> originallyFocusedElement = ShareRefPtr(document->FocusedElement());

    // 18. Add an element to the top layer given element.
    AddToTopLayer();

    // 19. If effectiveType is Hint and ancestor's opened in popover mode is "auto", then set document's hint
    //     stack parent to ancestor.
    if (effectiveType == dom::PopoverState::Hint && ancestor
        && ancestor->GetPopoverData()->OpenedInPopoverMode() == dom::PopoverState::Auto)
    {
      document->SetHintStackParent(ancestor.get());
    }

    // 20. Set element's popover visibility state to showing.
    {
      style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::PopoverOpen,
                                                             true);
      GetPopoverData()->SetVisibilityState(dom::PopoverVisibilityState::Showing);
    }

    // 21. Set element's popover trigger to source.
    // 22. Set element's implicit anchor element to source.
    // (The trigger is also the popover's implicit anchor element.)
    SetInvoker(source);

    // 23. Run the popover focusing steps given element.
    RunPopoverFocusingSteps();

    // 24. If shouldRestoreFocus is true and element's popover attribute is not in the No Popover state, then
    //     set element's previously focused element to originallyFocusedElement.
    // (The popover data goes with the popover attribute.)
    if (shouldRestoreFocus && GetPopoverData())
    {
      GetPopoverData()->SetPreviouslyFocusedElement(originallyFocusedElement.get());
    }

    // 25. Run cleanupShowingSteps.
    cleanupShowingSteps();
    // 26. Queue a popover toggle event task given element, "closed", "open", and source.
    if (GetPopoverData())
    {
      QueuePopoverToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open, source);
    }
    // FIXME: Telling the popover's invokers they expanded (onPopoverToggle), with accessibility relations.
    return {};
  }

  // https://html.spec.whatwg.org/#hide-popover-algorithm
  dom::ExceptionOr<void> HTMLElement::HidePopoverInternal(dom::FocusPreviousElement focusPreviousElement,
                                                          dom::FireEvents fireEvents, bool throwExceptions,
                                                          HTMLElement *source) noexcept
  {
    // 1. Let validityResult be the result of running check popover validity given element, true, and null.
    // 2. If validityResult is not true: if throwExceptions is true and validityResult is a DOMException, then
    //    throw validityResult; return.
    auto validity = CheckPopoverValidity(true, nullptr);
    if (!IsValid(validity))
    {
      return InvalidPopoverResult(krys::move(validity), throwExceptions);
    }

    // 3. Let document be element's node document.
    Ref<dom::Document> document = ShareRef(NodeDocument());
    Ref<HTMLElement> protectedThis = ShareRef(*this);

    // 4. Let nestedHide be element's popover hiding.
    bool nestedHide = GetPopoverData()->IsHidingPopover();
    // 5. Set element's popover hiding to true.
    GetPopoverData()->SetIsHidingPopover(true);
    // 6. If nestedHide is true, then set fireEvents to false.
    if (nestedHide)
    {
      fireEvents = dom::FireEvents::No;
    }
    // 7. Increment document's hiding popover nesting count.
    document->IncrementHidingPopoverNestingCount();
    // 8. Let cleanupSteps be the following steps:
    // 8a. If nestedHide is false, then set element's popover hiding to false.
    // 8b. Decrement document's hiding popover nesting count.
    auto cleanupSteps = [&]
    {
      if (!nestedHide && GetPopoverData())
      {
        GetPopoverData()->SetIsHidingPopover(false);
      }
      document->DecrementHidingPopoverNestingCount();
    };

    // 9. Let autoPopoverListContainsElement be true if document's showing auto popover list contains element;
    //    otherwise false.
    bool autoPopoverListContainsElement = ListContains(document->ShowingAutoPopoverList(), *this);
    // 10. Let hintPopoverListContainsElement be true if document's showing hint popover list contains
    // element;
    //     otherwise false.
    bool hintPopoverListContainsElement = ListContains(document->ShowingHintPopoverList(), *this);

    // 11. If element's opened in popover mode is "auto" or "hint":
    if (auto mode = GetPopoverData()->OpenedInPopoverMode();
        mode == dom::PopoverState::Auto || mode == dom::PopoverState::Hint)
    {
      // 11a. If hintPopoverListContainsElement is true, then run hide popover stack until given document,
      //      element, Hint, focusPreviousElement, and fireEvents.
      if (hintPopoverListContainsElement)
      {
        document->HidePopoverStackUntil(this, dom::PopoverState::Hint, focusPreviousElement, fireEvents);
      }
      // 11b. If element is document's hint stack parent, then run hide popover stack until given document,
      //      null, Hint, focusPreviousElement, and fireEvents.
      if (this == document->HintStackParent())
      {
        document->HidePopoverStackUntil(nullptr, dom::PopoverState::Hint, focusPreviousElement, fireEvents);
      }
      // 11c. If autoPopoverListContainsElement is true, then run hide popover stack until given document,
      //      element, Auto, focusPreviousElement, and fireEvents.
      if (autoPopoverListContainsElement)
      {
        document->HidePopoverStackUntil(this, dom::PopoverState::Auto, focusPreviousElement, fireEvents);
      }

      // 11d. Set validityResult to the result of running check popover validity given element, true, and
      //      document.
      // 11e. If validityResult is not true: run cleanupSteps; if throwExceptions is true and validityResult
      //      is a DOMException, then throw validityResult; return.
      validity = CheckPopoverValidity(true, document.get());
      if (!IsValid(validity))
      {
        cleanupSteps();
        return InvalidPopoverResult(krys::move(validity), throwExceptions);
      }
    }

    // 12. If fireEvents is true:
    if (fireEvents == dom::FireEvents::Yes)
    {
      // 12a. Fire an event named beforetoggle, using ToggleEvent, with the oldState attribute initialized to
      //      "open", the newState attribute initialized to "closed", and the source attribute set to source
      //      at element.
      (void)FirePopoverBeforeToggleEvent(dom::ToggleState::Open, dom::ToggleState::Closed, false, source);

      // 12b. Set validityResult to the result of running check popover validity given element, true, and
      //      document.
      // 12c. If validityResult is not true: run cleanupSteps; if throwExceptions is true and validityResult
      //      is a DOMException, then throw validityResult; return.
      validity = CheckPopoverValidity(true, document.get());
      if (!IsValid(validity))
      {
        cleanupSteps();
        return InvalidPopoverResult(krys::move(validity), throwExceptions);
      }

      // 12d. Request an element to be removed from the top layer given element.
      RequestRemoveFromTopLayer();
      // 12e. Set element's implicit anchor element to null.
      // (With the trigger, in step 14.)
    }
    // 13. Otherwise, remove an element from the top layer immediately given element.
    else if (IsInTopLayer())
    {
      RemoveFromTopLayer();
    }

    // 14. Set element's popover trigger to null.
    SetInvoker(nullptr);
    // 15. Set element's opened in popover mode to null.
    GetPopoverData()->SetOpenedInPopoverMode(dom::PopoverState::None);
    // 16. Set element's popover visibility state to hidden.
    {
      Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
      if (ParentNode())
      {
        styleInvalidation.emplace(*this, css::SelectorPseudoClass::PopoverOpen, false);
      }
      GetPopoverData()->SetVisibilityState(dom::PopoverVisibilityState::Hidden);
    }

    // 17. If element is document's hint stack parent, or document's showing hint popover list is empty, then
    //     set document's hint stack parent to null.
    if (this == document->HintStackParent() || document->ShowingHintPopoverList().empty())
    {
      document->SetHintStackParent(nullptr);
    }

    // 18. If fireEvents is true, then queue a popover toggle event task given element, "open", "closed", and
    //     source.
    if (fireEvents == dom::FireEvents::Yes)
    {
      QueuePopoverToggleEventTask(dom::ToggleState::Open, dom::ToggleState::Closed, source);
    }

    // 19. Let previouslyFocusedElement be element's previously focused element.
    // 20. If previouslyFocusedElement is not null:
    if (RefPtr<dom::Element> previouslyFocusedElement =
          ShareRefPtr(GetPopoverData()->PreviouslyFocusedElement()))
    {
      // 20a. Set element's previously focused element to null.
      // 20b. If focusPreviousElement is true and document's focused area of the document's DOM anchor is a
      //      shadow-including inclusive descendant of element, then run the focusing steps for
      //      previouslyFocusedElement; the viewport should not be scrolled by doing this step.
      GetPopoverData()->SetPreviouslyFocusedElement(nullptr);
      auto *focusedElement = document->FocusedElement();
      if (focusPreviousElement == dom::FocusPreviousElement::Yes && focusedElement
          && dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*focusedElement, *this))
      {
        FocusAlgorithms::FocusingSteps(*previouslyFocusedElement, nullptr, {.PreventScroll = true});
      }
    }

    // 21. If element's popover close watcher is not null:
    // 21a. Destroy element's popover close watcher.
    // 21b. Set element's popover close watcher to null.
    // (The focusing steps' events could have removed the popover attribute, which destroys the close
    // watcher.)
    if (auto *popoverData = GetPopoverData())
    {
      if (RefPtr<CloseWatcher> closeWatcher = ShareRefPtr(popoverData->CloseWatcher()))
      {
        closeWatcher->Destroy();
        popoverData->SetCloseWatcher(nullptr);
      }
    }

    // 22. Run cleanupSteps.
    cleanupSteps();
    // FIXME: Telling the popover's invokers they collapsed (onPopoverToggle), with accessibility relations.
    return {};
  }

  // https://html.spec.whatwg.org/#check-popover-validity
  dom::ExceptionOr<bool> HTMLElement::CheckPopoverValidity(bool expectedToBeShowing,
                                                           dom::Document *expectedDocument) const noexcept
  {
    // 1. If element's popover attribute is in the No Popover state, then throw a "NotSupportedError"
    //    DOMException.
    if (GetPopoverState() == dom::PopoverState::None)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError,
                             u8"Element does not have the popover attribute."};
    }

    // 2. If any of the following are true: expectedToBeShowing is true and element's popover visibility state
    //    is not showing; or expectedToBeShowing is false and element's popover visibility state is not
    //    hidden, then return false.
    if (IsPopoverShowing() != expectedToBeShowing)
    {
      return false;
    }

    // 3. If any of the following are true: expectedToBeShowing is false and element is not connected;
    //    expectedToBeShowing is false and element's node document is not fully active; expectedDocument is
    //    not null and element's node document is not expectedDocument; element is a dialog element and its is
    //    modal is set to true; or element's fullscreen flag is set, then throw an "InvalidStateError"
    //    DOMException.
    // A showing popover can be disconnected (the removing steps run after its removal) or in a document that
    // isn't fully active: hiding it has to succeed.
    if (!expectedToBeShowing && !IsConnected())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Element is not connected."};
    }
    if (!expectedToBeShowing && !NodeDocument().IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Invalid for popovers within documents that are not fully active."};
    }
    if (expectedDocument && &NodeDocument() != expectedDocument)
    {
      return dom::Exception {
        dom::ExceptionCode::InvalidStateError,
        u8"Invalid when the document changes while showing or hiding a popover element."};
    }
    if (auto *dialog = DynamicDowncast<HTMLDialogElement>(*this); dialog && dialog->IsModal())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Element is a modal <dialog> element."};
    }
    // FIXME: The fullscreen flag, once there's fullscreen.
    // 4. Return true.
    return true;
  }

  // https://html.spec.whatwg.org/#popover-focusing-steps
  void HTMLElement::RunPopoverFocusingSteps() noexcept
  {
    // 1. If the allow focus steps given subject's node document return false, then return.
    // (Focus is always allowed; see Focus in HTMLOrSVGOrMathMLElement.)

    // 2. If subject is a dialog element, then run the dialog focusing steps given subject and return.
    if (auto *dialog = DynamicDowncast<HTMLDialogElement>(*this))
    {
      dialog->RunFocusingSteps();
      return;
    }

    // 3. If subject has the autofocus attribute, then let control be subject.
    // 4. Otherwise, let control be the autofocus delegate for subject given "other".
    RefPtr<dom::Node> control =
      HasAttribute(u8"autofocus")
        ? ShareRefPtr<dom::Node>(this)
        : ShareRefPtr(FocusAlgorithms::AutofocusDelegate(*this, dom::FocusTrigger::Other));
    // 5. If control is null, then return.
    if (!control)
    {
      return;
    }

    // (A document that isn't in a page has no top-level traversable.)
    Ref<dom::Document> controlDocument = ShareRef(control->NodeDocument());
    if (!controlDocument->GetPage())
    {
      return;
    }

    // 6. Run the focusing steps given control.
    FocusAlgorithms::FocusingSteps(*control);

    // 7. Let topDocument be control's node navigable's top-level traversable's active document.
    // 8. If control's node document's origin is not the same as the origin of topDocument, then return.
    // (There are no nested navigables, so topDocument is control's node document.)
    // 9. Empty topDocument's autofocus candidates.
    controlDocument->AutofocusCandidates().clear();
    // 10. Set topDocument's autofocus processed flag to true.
    controlDocument->SetAutofocusProcessed();
  }

  // https://html.spec.whatwg.org/#queue-a-popover-toggle-event-task
  // (ToggleEventTask is the popover toggle task tracker.)
  void HTMLElement::QueuePopoverToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState,
                                                HTMLElement *source) noexcept
  {
    GetPopoverData()->EnsureToggleEventTask(*this)->Queue(oldState, newState, source);
  }

  bool HTMLElement::IsValidCommandType(dom::CommandType command) const noexcept
  {
    return Element::IsValidCommandType(command) || command == dom::CommandType::TogglePopover
           || command == dom::CommandType::ShowPopover || command == dom::CommandType::HidePopover;
  }

  bool HTMLElement::HandleCommandInternal(HTMLButtonElement &invoker, dom::CommandType command) noexcept
  {
    if (GetPopoverState() == dom::PopoverState::None)
    {
      return false;
    }

    if (IsPopoverShowing())
    {
      bool shouldHide =
        command == dom::CommandType::TogglePopover || command == dom::CommandType::HidePopover;
      if (shouldHide)
      {
        (void)HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, false, &invoker);
        return true;
      }
    }
    else
    {
      bool shouldShow =
        command == dom::CommandType::TogglePopover || command == dom::CommandType::ShowPopover;
      if (shouldShow)
      {
        (void)ShowPopoverInternal(&invoker, false);
        return true;
      }
    }

    return false;
  }

  bool HTMLElement::FirePopoverBeforeToggleEvent(dom::ToggleState oldState, dom::ToggleState newState,
                                                 bool cancelable, HTMLElement *source) noexcept
  {
    auto stringForState = [](dom::ToggleState state)
    {
      return dom::DOMString(state == dom::ToggleState::Closed ? u8"closed" : u8"open");
    };

    dom::ToggleEventInit init;
    init.OldState = stringForState(oldState);
    init.NewState = stringForState(newState);
    init.Source = ShareRefPtr<dom::Element>(source);
    auto event = dom::ToggleEvent::Create(dom::DOMStringAtom(EventNames::BeforeToggle), init, cancelable);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
    return !event->DefaultPrevented();
  }

  void HTMLElement::SetInvoker(HTMLElement *invoker) noexcept
  {
    if (RefPtr<HTMLElement> oldInvoker = ShareRefPtr(GetPopoverData()->Invoker()))
    {
      oldInvoker->SetInvokedPopover(nullptr);
    }
    GetPopoverData()->SetInvoker(invoker);
    if (invoker)
    {
      invoker->SetInvokedPopover(this);
    }
  }

  // https://html.spec.whatwg.org/#the-popover-attribute
  // The following attribute change steps, given element, localName, oldValue, value, and namespace, are used
  // for all HTML elements:
  // 1. If namespace is not null, then return.
  // 2. If localName is not popover, then return.
  // 3. If element's popover visibility state is in the showing state and oldValue and value are in different
  //    states, then run the hide popover algorithm given element, true, true, false, and null.
  // (Run for the popover attribute only; the popover data is made and cleared with it.)
  void HTMLElement::PopoverAttributeChanged(Maybe<dom::DOMStringView> value) noexcept
  {
    auto newPopoverState = EnumeratedAttribute<"popover", HTMLElement>::ResolveState(value);
    if (newPopoverState == GetPopoverState())
    {
      return;
    }

    style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::PopoverOpen,
                                                           false);

    // A showing popover whose state changes hides; its beforetoggle listeners could change the attribute
    // again.
    if (IsPopoverShowing())
    {
      (void)HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, false, nullptr);
      newPopoverState = EnumeratedAttribute<"popover", HTMLElement>::ResolveState(
        GetAttribute(dom::DOMStringAtom(u8"popover")));
    }

    if (newPopoverState == dom::PopoverState::None)
    {
      if (auto *popoverData = GetPopoverData())
      {
        if (RefPtr<CloseWatcher> closeWatcher = ShareRefPtr(popoverData->CloseWatcher()))
        {
          closeWatcher->Destroy();
        }
      }
      ClearPopoverData();
    }
    else
    {
      EnsurePopoverData().SetPopoverState(newPopoverState);
    }
  }

  void HTMLElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                       Maybe<dom::DOMStringView> value,
                                       dom::DOMStringAtom namespaceURI) noexcept
  {
    StyledElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"popover")
    {
      PopoverAttributeChanged(value);
    }

    // The computed heading levels below, which `:heading()` matches, follow these.
    if (namespaceURI == dom::DOMStringAtom::Null()
        && (localName == u8"headingoffset" || localName == u8"headingreset") && oldValue != value)
    {
      InvalidateStyleForSubtree();
    }
  }

#pragma endregion

  // https://html.spec.whatwg.org/#dom-contenteditable
  dom::DOMString HTMLElement::ContentEditable() const noexcept
  {
    // The contentEditable IDL attribute, on getting, must return the string "true" if the content attribute
    // is set to the True state, "plaintext-only" if the content attribute is set to the Plaintext-Only state,
    // "false" if the content attribute is set to the False state, and "inherit" otherwise.
    switch (
      EnumeratedAttribute<"contenteditable", HTMLElement>::ResolveState(GetAttribute(u8"contenteditable")))
    {
      case ContentEditableAttributeState::True:          return u8"true";
      case ContentEditableAttributeState::PlaintextOnly: return u8"plaintext-only";
      case ContentEditableAttributeState::False:         return u8"false";
      case ContentEditableAttributeState::Inherit:       return u8"inherit";
    }
    krys_unreachable();
  }

  // https://html.spec.whatwg.org/#dom-contenteditable
  dom::ExceptionOr<void> HTMLElement::ContentEditable(dom::DOMStringView value) noexcept
  {
    CEReactionsScope ceReactions;
    // On setting, if the new value is an ASCII case-insensitive match for the string "inherit", then the
    // content attribute must be removed, if the new value is an ASCII case-insensitive match for the string
    // "true", then the content attribute must be set to the string "true", if the new value is an ASCII
    // case-insensitive match for the string "plaintext-only", then the content attribute must be set to the
    // string "plaintext-only", if the new value is an ASCII case-insensitive match for the string "false",
    // then the content attribute must be set to the string "false", and otherwise the attribute setter must
    // throw a "SyntaxError" DOMException.
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"inherit"))
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"contenteditable", *this);
      return {};
    }
    for (auto keyword : {u8"true", u8"plaintext-only", u8"false"})
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, keyword))
      {
        dom::ElementAlgorithms::SetAttributeValue(*this, u8"contenteditable", keyword);
        return {};
      }
    }
    return dom::Exception {dom::ExceptionCode::SyntaxError};
  }

  dom::Node::Editability HTMLElement::EditabilityFromContentEditableAttr(const dom::Node &node) noexcept
  {
    auto *startElement = DynamicDowncast<dom::Element>(node);
    if (!startElement)
    {
      startElement = node.ParentElement();
    }
    for (auto *element = startElement; element; element = element->ParentElement())
    {
      if (!Is<HTMLElement>(*element))
      {
        continue;
      }
      switch (EnumeratedAttribute<"contenteditable", HTMLElement>::ResolveState(
        element->GetAttribute(u8"contenteditable")))
      {
        case ContentEditableAttributeState::True:          return dom::Node::Editability::CanEditRichly;
        case ContentEditableAttributeState::PlaintextOnly: return dom::Node::Editability::CanEditPlainText;
        case ContentEditableAttributeState::False:         return dom::Node::Editability::ReadOnly;
        case ContentEditableAttributeState::Inherit:       break;
      }
    }

    auto *containingShadowRoot = dom::TreeQueries::ContainingShadowRoot(node);
    if (containingShadowRoot && containingShadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
    {
      return dom::Node::Editability::ReadOnly;
    }

    if (node.NodeDocument().InDesignMode() && node.IsInDocumentTree())
    {
      return dom::Node::Editability::CanEditRichly;
    }

    return dom::Node::Editability::ReadOnly;
  }

  dom::DOMString HTMLElement::AttributeOrEmpty(dom::DOMStringView localName) const noexcept
  {
    return GetAttribute(dom::DOMStringAtom(localName)).value_or(dom::DOMString {});
  }

  uint32 HTMLElement::ParseBorderWidthAttribute(Maybe<dom::DOMStringView> value) const noexcept
  {
    if (value)
    {
      if (auto borderWidth = Numbers::ParseNonNegativeInteger(*value);
          borderWidth.Success() && borderWidth.Value <= std::numeric_limits<int32>::max())
      {
        return static_cast<uint32>(borderWidth.Value);
      }
    }

    return TagQualifiedName().TagName() == TagName::table && value ? 1 : 0;
  }

  void HTMLElement::ApplyBorderAttributeToStyle(dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept
  {
    AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderWidth,
                                         ParseBorderWidthAttribute(value), css::UnitType::px);
    AddPropertyToPresentationalHintStyle(style, css::PropertyId::BorderStyle, css::ValueId::Solid);
  }

  void HTMLElement::MapLanguageAttributeToLocale(dom::DOMStringView value,
                                                 css::MutableStyleProperties &style) noexcept
  {
    if (!value.empty())
    {
      // Have to quote so the locale id is treated as a string instead of as a CSS keyword.
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::InternalLocale,
                                           css::SerialiseString(css::CSSOMString(value)));
    }
    else
    {
      // The empty string means the language is explicitly unknown.
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::InternalLocale, css::ValueId::Auto);
    }
  }

  bool HTMLElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                       dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      if (localName == u8"align" || localName == u8"contenteditable" || localName == u8"hidden"
          || localName == u8"lang" || localName == u8"draggable" || localName == u8"dir")
      {
        return true;
      }
    }
    else if (namespaceURI == infra::Namespaces::XML && localName == u8"lang")
    {
      return true;
    }
    return StyledElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                           dom::DOMStringAtom namespaceURI,
                                                           dom::DOMStringView value,
                                                           css::MutableStyleProperties &style) noexcept
  {
    if (namespaceURI == infra::Namespaces::XML && localName == u8"lang")
    {
      MapLanguageAttributeToLocale(value, style);
      return;
    }
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }

    if (localName == u8"align")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::Center);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::CSSOMString(value));
      }
    }
    else if (localName == u8"contenteditable")
    {
      auto userModifyValue = css::ValueId::ReadWrite;
      auto state = EnumeratedAttribute<"contenteditable", HTMLElement>::ResolveState(value);
      switch (state)
      {
        case ContentEditableAttributeState::Inherit:
        {
          return;
        }
        case ContentEditableAttributeState::False:
        {
          userModifyValue = css::ValueId::ReadOnly;
          break;
        }
        case ContentEditableAttributeState::PlaintextOnly:
        case ContentEditableAttributeState::True:
        {
          if (state == ContentEditableAttributeState::PlaintextOnly)
          {
            userModifyValue = css::ValueId::ReadWritePlaintextOnly;
          }
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::BreakWord);
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::LineBreak,
                                               css::ValueId::AfterWhiteSpace);
          break;
        }
      }
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::InternalUserModify, userModifyValue);
    }
    else if (localName == u8"hidden")
    {
      // The hiddenUntilFoundEnabled setting is on by default.
      if (EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(value) == HiddenAttributeState::UntilFound)
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ContentVisibility, css::ValueId::Hidden);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::Display, css::ValueId::None);
      }
    }
    else if (localName == u8"draggable")
    {
      switch (EnumeratedAttribute<"draggable", HTMLElement>::ResolveState(value))
      {
        case DraggableAttributeState::True:
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::InternalUserDrag,
                                               css::ValueId::Element);
          break;
        }
        case DraggableAttributeState::False:
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::InternalUserDrag, css::ValueId::None);
          break;
        }
        case DraggableAttributeState::Auto: break;
      }
    }
    else if (localName == u8"dir")
    {
      switch (dom::ParseTextDirectionState(value))
      {
        case dom::TextDirectionState::Auto:
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::UnicodeBidi,
                                               UnicodeBidiAttributeForDirAuto(*this));
          break;
        }
        case dom::TextDirectionState::LTR:
        case dom::TextDirectionState::RTL:
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::Direction, css::CSSOMString(value));
          auto tagName = TagQualifiedName().TagName();
          if (tagName != TagName::bdi && tagName != TagName::bdo && tagName != TagName::output)
          {
            AddPropertyToPresentationalHintStyle(style, css::PropertyId::UnicodeBidi, css::ValueId::Isolate);
          }
          break;
        }
        case dom::TextDirectionState::Undefined: break;
      }
    }
    else if (localName == u8"lang")
    {
      // xml:lang has a higher priority than lang.
      if (!HasAttributeNS(dom::DOMStringAtom(infra::Namespaces::XML), u8"lang"))
      {
        MapLanguageAttributeToLocale(value, style);
      }
    }
    else
    {
      StyledElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
    }
  }

  void HTMLElement::ApplyAspectRatioFromWidthAndHeightAttributesToStyle(
    dom::DOMStringView widthAttribute, dom::DOMStringView heightAttribute,
    css::MutableStyleProperties &style) noexcept
  {
    auto dimensionWidth = Dimensions::ParseHTMLDimension(widthAttribute);
    if (!dimensionWidth || dimensionWidth->DimensionType != HTMLDimension::Type::Pixel)
    {
      return;
    }
    auto dimensionHeight = Dimensions::ParseHTMLDimension(heightAttribute);
    if (!dimensionHeight || dimensionHeight->DimensionType != HTMLDimension::Type::Pixel)
    {
      return;
    }

    AddParsedWidthAndHeightToAspectRatioList(dimensionWidth->Number, dimensionHeight->Number, style);
  }

  void HTMLElement::ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(
    dom::DOMStringView widthAttribute, dom::DOMStringView heightAttribute,
    css::MutableStyleProperties &style) noexcept
  {
    auto dimensionWidth = Numbers::ParseNonNegativeInteger(widthAttribute);
    if (!dimensionWidth.Success())
    {
      return;
    }
    auto dimensionHeight = Numbers::ParseNonNegativeInteger(heightAttribute);
    if (!dimensionHeight.Success())
    {
      return;
    }

    AddParsedWidthAndHeightToAspectRatioList(static_cast<double>(dimensionWidth.Value),
                                             static_cast<double>(dimensionHeight.Value), style);
  }

  void HTMLElement::AddParsedWidthAndHeightToAspectRatioList(double width, double height,
                                                             css::MutableStyleProperties &style) noexcept
  {
    style.SetProperty(css::PropertyId::AspectRatio, css::ValueList::CreateSpaceSeparated(
                                                      css::PrimitiveValue::Create(css::ValueId::Auto),
                                                      css::RatioValue::Create(css::Ratio {width, height})));
  }

  void HTMLElement::ApplyAlignmentAttributeToStyle(dom::DOMStringView alignment,
                                                   css::MutableStyleProperties &style) noexcept
  {
    // Vertical alignment with respect to the current baseline of the text
    // right or left means floating images.
    auto floatValue = css::ValueId::Invalid;
    auto verticalAlignValue = css::ValueId::Invalid;

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"absmiddle")
        || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"abscenter"))
    {
      verticalAlignValue = css::ValueId::Middle;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"absbottom"))
    {
      verticalAlignValue = css::ValueId::Bottom;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"left"))
    {
      floatValue = css::ValueId::Left;
      verticalAlignValue = css::ValueId::Top;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"right"))
    {
      floatValue = css::ValueId::Right;
      verticalAlignValue = css::ValueId::Top;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"top"))
    {
      verticalAlignValue = css::ValueId::Top;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"middle"))
    {
      verticalAlignValue = css::ValueId::InternalBaselineMiddle;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"center"))
    {
      verticalAlignValue = css::ValueId::Middle;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"bottom"))
    {
      verticalAlignValue = css::ValueId::Baseline;
    }
    else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(alignment, u8"texttop"))
    {
      verticalAlignValue = css::ValueId::TextTop;
    }

    if (floatValue != css::ValueId::Invalid)
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::Float, floatValue);
    }

    if (verticalAlignValue != css::ValueId::Invalid)
    {
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, verticalAlignValue);
    }
  }

  void HTMLElement::AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                         dom::DOMStringView value, AllowPercentage allowPercentage,
                                         UseCSSPXAsUnitType useCSSPX, IsMultiLength isMultiLength,
                                         AllowZeroValue allowZeroValue) noexcept
  {
    // Without a zero value, these are the rules for parsing nonzero dimension values.
    // @see https://html.spec.whatwg.org/#rules-for-parsing-non-zero-dimension-values
    auto dimensionValue = isMultiLength == IsMultiLength::No ? Dimensions::ParseHTMLDimension(value)
                                                             : Dimensions::ParseHTMLMultiLength(value);
    if (!dimensionValue || (!dimensionValue->Number && allowZeroValue == AllowZeroValue::No))
    {
      return;
    }
    if (dimensionValue->DimensionType == HTMLDimension::Type::Percentage)
    {
      if (allowPercentage == AllowPercentage::Yes)
      {
        AddPropertyToPresentationalHintStyle(style, property, dimensionValue->Number,
                                             css::UnitType::Percentage);
      }
      return;
    }
    if (useCSSPX == UseCSSPXAsUnitType::Yes)
    {
      AddPropertyToPresentationalHintStyle(style, property, dimensionValue->Number, css::UnitType::px);
    }
    else
    {
      AddPropertyToPresentationalHintStyle(style, property, dimensionValue->Number, css::UnitType::Number);
    }
  }

  void HTMLElement::AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                         dom::DOMStringView value, AllowZeroValue allowZeroValue) noexcept
  {
    AddHTMLLengthToStyle(style, property, value, AllowPercentage::Yes, UseCSSPXAsUnitType::Yes,
                         IsMultiLength::No, allowZeroValue);
  }

  void HTMLElement::AddHTMLMultiLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                              dom::DOMStringView value) noexcept
  {
    AddHTMLLengthToStyle(style, property, value, AllowPercentage::Yes, UseCSSPXAsUnitType::Yes,
                         IsMultiLength::Yes);
  }

  void HTMLElement::AddHTMLPixelsToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                         dom::DOMStringView value) noexcept
  {
    AddHTMLLengthToStyle(style, property, value, AllowPercentage::No, UseCSSPXAsUnitType::Yes,
                         IsMultiLength::No);
  }

  Maybe<SRGBA<uint8>> HTMLElement::ParseLegacyColorValue(dom::DOMStringView string) noexcept
  {
    // 1. If input is the empty string, then return failure.
    if (string.empty())
    {
      return {};
    }

    // 2. Strip leading and trailing ASCII whitespace from input.
    string = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(string);
    // Steps 6 to 19 give black for what's left.
    if (string.empty())
    {
      return boo::Color::black;
    }

    // 3. If input is an ASCII case-insensitive match for "transparent", then return failure.
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(string, u8"transparent"))
    {
      return {};
    }

    // 4. If input is an ASCII case-insensitive match for one of the named colors, then return the CSS color
    //    corresponding to that keyword.
    if (auto namedColor = css::ParserFastPaths::ParseNamedColor(string))
    {
      return namedColor;
    }

    // 5. If input's code point length is four, and the first character in input is U+0023 (#), and the last
    //    three characters of input are all ASCII hex digits:
    // 5a. Let result be a CSS color.
    // 5b. Interpret the second character of input as a hexadecimal digit; let the red component of result be
    //     the resulting number multiplied by 17.
    // 5c. Interpret the third character of input as a hexadecimal digit; let the green component of result be
    //     the resulting number multiplied by 17.
    // 5d. Interpret the fourth character of input as a hexadecimal digit; let the blue component of result be
    //     the resulting number multiplied by 17.
    // 5e. Return result.
    if (string.size() == 4 && string[0] == '#' && text::IsASCIIHexDigit(string[1])
        && text::IsASCIIHexDigit(string[2]) && text::IsASCIIHexDigit(string[3]))
    {
      return SRGBA<uint8> {static_cast<uint8>(text::ToASCIIHexValue(string[1]) * 0x11),
                           static_cast<uint8>(text::ToASCIIHexValue(string[2]) * 0x11),
                           static_cast<uint8>(text::ToASCIIHexValue(string[3]) * 0x11)};
    }

    // 6. Replace any code points greater than U+FFFF in input (i.e., any characters that are not in the basic
    //    multilingual plane) with "00".
    // 7. If input's code point length is greater than 128, truncate input, leaving only the first 128
    //    characters.
    // 8. If the first character in input is U+0023 (#), then remove it.
    // 9. Replace any character in input that is not an ASCII hex digit with U+0030 (0).
    // The "#" is removed before truncating, so 128 digits follow it, as in other browsers.
    constexpr size_t maxColorLength = 128;

    // We'll pad the buffer with two extra 0s later, so reserve two more than the max.
    SmallList<char8, maxColorLength + 2> digitBuffer;

    size_t i = 0;
    if (string[0] == '#')
    {
      i = 1;
    }
    while (i < string.size() && digitBuffer.size() < maxColorLength)
    {
      auto lead = static_cast<uint8>(string[i]);
      size_t length = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
      if (length == 1 && text::IsASCIIHexDigit(string[i]))
      {
        digitBuffer.push_back(string[i]);
      }
      else
      {
        digitBuffer.push_back('0');
        if (length == 4 && digitBuffer.size() < maxColorLength)
        {
          digitBuffer.push_back('0');
        }
      }
      i += length;
    }

    if (digitBuffer.empty())
    {
      return boo::Color::black;
    }

    // 10. While input's code point length is zero or not a multiple of three, append U+0030 (0) to input.
    // Two zeros pad it at least that far; the components' length below leaves out any more.
    digitBuffer.push_back('0');
    digitBuffer.push_back('0');

    // 11. Split input into three strings of equal code point length, to obtain three components. Let length
    // be
    //     the code point length that all of those components have (one third the code point length of input).
    // 15. Let result be a CSS color.
    // 16. Interpret the first component as a hexadecimal number; let the red component of result be the
    //     resulting number.
    // 17. Interpret the second component as a hexadecimal number; let the green component of result be the
    //     resulting number.
    // 18. Interpret the third component as a hexadecimal number; let the blue component of result be the
    //     resulting number.
    // 19. Return result.
    if (digitBuffer.size() < 6)
    {
      return SRGBA<uint8> {text::ToASCIIHexValue(digitBuffer[0]), text::ToASCIIHexValue(digitBuffer[1]),
                           text::ToASCIIHexValue(digitBuffer[2])};
    }

    krys_debug_assert(digitBuffer.size() >= 6);
    size_t componentLength = digitBuffer.size() / 3;

    // 12. If length is greater than 8, then remove the leading length-8 characters in each component, and let
    //     length be 8.
    size_t componentSearchWindowLength = std::min<size_t>(componentLength, 8);
    size_t redIndex = componentLength - componentSearchWindowLength;
    size_t greenIndex = componentLength * 2 - componentSearchWindowLength;
    size_t blueIndex = componentLength * 3 - componentSearchWindowLength;

    // 13. While length is greater than two and the first character in each component is U+0030 (0), remove
    //     that character and reduce length by one.
    while (digitBuffer[redIndex] == '0' && digitBuffer[greenIndex] == '0' && digitBuffer[blueIndex] == '0'
           && (componentLength - redIndex) > 2)
    {
      redIndex++;
      greenIndex++;
      blueIndex++;
    }

    // 14. If length is still greater than two, truncate each component, leaving only the first two characters
    //     in each.
    // 15.-19. (As above, with each component's first two characters.)
    krys_debug_assert(redIndex + 1 < componentLength);
    krys_debug_assert(greenIndex >= componentLength);
    krys_debug_assert(greenIndex + 1 < componentLength * 2);
    krys_debug_assert(blueIndex >= componentLength * 2);
    krys_debug_assert(blueIndex + 1 < digitBuffer.size());

    auto redValue = text::ToASCIIHexValue(digitBuffer[redIndex], digitBuffer[redIndex + 1]);
    auto greenValue = text::ToASCIIHexValue(digitBuffer[greenIndex], digitBuffer[greenIndex + 1]);
    auto blueValue = text::ToASCIIHexValue(digitBuffer[blueIndex], digitBuffer[blueIndex + 1]);
    return SRGBA<uint8> {redValue, greenValue, blueValue};
  }

  void HTMLElement::AddHTMLColorToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                        dom::DOMStringView attributeValue) noexcept
  {
    if (auto color = ParseLegacyColorValue(attributeValue))
    {
      style.SetProperty(property, css::ValuePool::MainThreadPool().CreateColorValue(boo::Color(*color)));
    }
  }

#pragma endregion
}
