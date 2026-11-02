#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/SimulatedClick.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/FormAssociatedElement.hpp"
#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/LabelsNodeList.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Dimensions.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
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

  void HTMLElement::Title(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"title", krys::move(value));
  }

  dom::DOMString HTMLElement::Lang() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"lang").value_or(u8"");
  }

  void HTMLElement::Lang(dom::DOMString &&value) noexcept
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
        if (state == AttributeState::Yes)
        {
          return true;
        }
        if (state == AttributeState::No)
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
    auto value = Reflect<dom::DOMString>(*this, u8"dir");
    return EnumeratedAttribute<"dir", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      krys::move(value));
  }

  void HTMLElement::Dir(dom::DOMString &&value) noexcept
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
    if (state == AttributeState::UntilFound)
    {
      return u8"until-found";
    }

    // 2. If the hidden attribute is set, then return true.
    // 3. Return false.
    // (Any other value, and the missing value, is in the Hidden state.)
    return state == AttributeState::Hidden;
  }

  bool HTMLElement::IsHiddenUntilFound() const noexcept
  {
    auto value = GetAttribute(u8"hidden");
    return value && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"until-found");
  }

  // https://html.spec.whatwg.org/#dom-hidden
  // (The setter's steps are split between the overloads by the given value's type; a null value is the
  // empty string's overload.)
  void HTMLElement::Hidden(dom::DOMString &&value) noexcept
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

  void HTMLElement::AccessKey(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"accesskey", krys::move(value));
  }

  bool HTMLElement::AccessKeyAction(bool sendMouseEvents) noexcept
  {
    if (FocusAlgorithms::IsFocusableArea(*this))
    {
      Focus();
    }
    return DispatchSimulatedClick(nullptr, sendMouseEvents
                                             ? dom::SimulatedClickMouseEventOptions::SendMouseUpDownEvents
                                             : dom::SimulatedClickMouseEventOptions::SendNoEvents);
  }

  dom::DOMString HTMLElement::AccessKeyLabel() const noexcept
  {
    // FIXME: Implement AccessKeyLabel() method.
    return {};
  }

  bool HTMLElement::Draggable() const noexcept
  {
    auto value = Reflect<dom::DOMString>(*this, u8"draggable");
    switch (EnumeratedAttribute<"draggable", HTMLElement>::ResolveState(value))
    {
      case AttributeState::True:  return true;
      case AttributeState::False: return false;
      case AttributeState::Auto:
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
    KRYS_NODISCARD AttributeState OwnAutocapitalizationHint(const HTMLElement &element) noexcept
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
      return AttributeState::Default;
    }

    /// @see https://html.spec.whatwg.org/#used-autocorrection-state
    KRYS_NODISCARD AttributeState UsedAutocorrectionState(const HTMLElement &element) noexcept
    {
      // 1. If element is an input element whose type attribute is in one of the URL, Email, or Password
      //    states, then return Off.
      if (auto *input = DynamicDowncast<HTMLInputElement>(element);
          input && (input->IsURLField() || input->IsEmailField() || input->IsPasswordField()))
      {
        return AttributeState::Off;
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
      return AttributeState::On;
    }

    /// @brief The keyword of @p name's state if it's one of @p keywords, or the empty string: an attribute
    /// reflected limited to only known values, whose keywords are each their own state.
    /// @see https://html.spec.whatwg.org/#limited-to-only-known-values
    KRYS_NODISCARD dom::DOMString KnownKeyword(const HTMLElement &element, dom::DOMStringAtom name,
                                               std::initializer_list<const char8_t *> keywords) noexcept
    {
      if (auto value = element.GetAttribute(name))
      {
        for (auto *keyword : keywords)
        {
          if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, keyword))
          {
            return keyword;
          }
        }
      }
      return {};
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
        case AttributeState::True:  return true;
        case AttributeState::False: return false;
        default:                    break;
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
        case AttributeState::False:   return u8"false";
        // 2. If element's writingsuggestions content attribute is in the Default state, element has a parent
        //    element, and the computed writing suggestions value of element's parent element is "false", then
        //    return "false".
        case AttributeState::Default: break;
        // 3. Return "true".
        default:                      return u8"true";
      }
    }
    return u8"true";
  }

  void HTMLElement::WritingSuggestions(dom::DOMString &&value) noexcept
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
      case AttributeState::Default:    return {};
      // 3. If state is None, then return "none".
      case AttributeState::None:       return u8"none";
      // 4. If state is Sentences, then return "sentences".
      case AttributeState::Sentences:  return u8"sentences";
      // 5. Return the keyword value corresponding to state.
      case AttributeState::Words:      return u8"words";
      case AttributeState::Characters: return u8"characters";
      default:                         return {};
    }
  }

  void HTMLElement::Autocapitalize(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    ReflectSetter(*this, u8"autocapitalize", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-autocorrect
  bool HTMLElement::Autocorrect() const noexcept
  {
    // The autocorrect getter steps are: return true if the element's used autocorrection state is On and
    // false if the element's used autocorrection state is Off.
    return UsedAutocorrectionState(*this) == AttributeState::On;
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
    return KnownKeyword(
      *this, u8"inputmode",
      {u8"none", u8"text", u8"tel", u8"url", u8"email", u8"numeric", u8"decimal", u8"search"});
  }

  // https://html.spec.whatwg.org/#dom-inputmode
  void HTMLElement::InputMode(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"inputmode", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-enterkeyhint
  // The enterKeyHint IDL attribute must reflect the enterkeyhint content attribute, limited to only known
  // values.
  dom::DOMString HTMLElement::EnterKeyHint() const noexcept
  {
    return KnownKeyword(*this, u8"enterkeyhint",
                        {u8"enter", u8"done", u8"go", u8"next", u8"previous", u8"search", u8"send"});
  }

  // https://html.spec.whatwg.org/#dom-enterkeyhint
  void HTMLElement::EnterKeyHint(dom::DOMString &&value) noexcept
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
      (void)node.ReplaceData(node.Length(), 0, dom::DOMString(next->Data()));

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

  void HTMLElement::InnerText(dom::DOMString &&value) noexcept
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

  dom::ExceptionOr<void> HTMLElement::OuterText(dom::DOMString &&value) noexcept
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
    if (queue->IsElementInternalsAttached())
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
    queue->SetElementInternalsAttached();
    return ElementInternals::Create(*this);
  }

  dom::ExceptionOr<void> HTMLElement::ShowPopover(const ShowPopoverOptions &options) noexcept
  {
    return ShowPopoverInternal(options.Source.get());
  }

  dom::ExceptionOr<void> HTMLElement::HidePopover() noexcept
  {
    return HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, true, nullptr);
  }

  dom::ExceptionOr<bool> HTMLElement::TogglePopover(const TogglePopoverOptionsOrBool &options) noexcept
  {
    Maybe<bool> force;
    HTMLElement *source = nullptr;
    if (auto *value = std::get_if<bool>(&options))
    {
      force = *value;
    }
    else
    {
      auto &dictionary = std::get<TogglePopoverOptions>(options);
      force = dictionary.Force;
      source = dictionary.Source.get();
    }

    if (IsPopoverShowing() && !force.value_or(false))
    {
      if (auto result =
            HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, true, nullptr);
          result.HasException())
      {
        return result.ReleaseException();
      }
    }
    else if (force.value_or(true))
    {
      if (auto result = ShowPopoverInternal(source); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    else
    {
      if (auto check = CheckPopoverValidity(IsPopoverShowing(), nullptr); check.HasException())
      {
        return check.ReleaseException();
      }
    }
    return IsPopoverShowing();
  }

  Maybe<dom::DOMString> HTMLElement::Popover() const noexcept
  {
    auto value = Reflect<dom::DOMString>(*this, u8"popover");
    return EnumeratedAttribute<"popover", HTMLElement>::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(
      krys::move(value));
  }

  void HTMLElement::Popover(Maybe<dom::DOMString> &&value) noexcept
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

    enum class ContentEditableType : uint8
    {
      Inherit,
      True,
      False,
      PlaintextOnly
    };

    KRYS_NODISCARD ContentEditableType GetContentEditableType(dom::DOMStringView value) noexcept
    {
      if (value.empty() || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true"))
      {
        return ContentEditableType::True;
      }
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false"))
      {
        return ContentEditableType::False;
      }
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"plaintext-only"))
      {
        return ContentEditableType::PlaintextOnly;
      }

      return ContentEditableType::Inherit;
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

  dom::ExceptionOr<void> HTMLElement::ShowPopoverInternal(HTMLElement *source, bool throwExceptions) noexcept
  {
    Ref<dom::Document> document = ShareRef(NodeDocument());

    // This prevents showing a popover during the show or hide of another popover.
    if (document->IsShowingPopover() || document->HidingPopoverNestingCount())
    {
      if (throwExceptions)
      {
        return dom::Exception {dom::ExceptionCode::InvalidStateError,
                               u8"Cannot show a popover while showing or hiding a popover."};
      }
      return {};
    }

    auto validity = CheckPopoverValidity(false, nullptr);
    if (!IsValid(validity))
    {
      return InvalidPopoverResult(krys::move(validity), throwExceptions);
    }

    krys_debug_assert(!GetPopoverData()->Invoker());
    krys_debug_assert(!IsInTopLayer() || document->IsPendingTopLayerRemoval(*this));

    Ref<HTMLElement> protectedThis = ShareRef(*this);
    document->SetIsShowingPopover(true);
    auto cleanupShowingSteps = [&]
    {
      document->SetIsShowingPopover(false);
    };

    if (!FirePopoverBeforeToggleEvent(dom::ToggleState::Closed, dom::ToggleState::Open, true, source))
    {
      cleanupShowingSteps();
      return {};
    }

    // Firing beforetoggle could have disconnected the element or changed its popover attribute.
    validity = CheckPopoverValidity(false, document.get());
    if (!IsValid(validity))
    {
      cleanupShowingSteps();
      return InvalidPopoverResult(krys::move(validity), throwExceptions);
    }

    bool shouldRestoreFocus = false;
    auto originalType = GetPopoverState();
    auto effectiveType = originalType;
    RefPtr<HTMLElement> ancestor;
    if (originalType == dom::PopoverState::Auto || originalType == dom::PopoverState::Hint)
    {
      ancestor = TopmostPopoverAncestor(source, true);

      // Hint popovers are lower priority than auto popovers, so an auto popover with a hint popover as its
      // parent is downgraded to a hint popover.
      if (ancestor && ancestor->GetPopoverData()->OpenedInPopoverMode() == dom::PopoverState::Hint
          && effectiveType == dom::PopoverState::Auto)
      {
        effectiveType = dom::PopoverState::Hint;
      }

      document->HidePopoverStackUntil(ancestor.get(), dom::PopoverState::Hint, dom::FocusPreviousElement::No,
                                      dom::FireEvents::Yes);
      if (effectiveType == dom::PopoverState::Auto)
      {
        document->HidePopoverStackUntil(ancestor.get(), dom::PopoverState::Auto,
                                        dom::FocusPreviousElement::No, dom::FireEvents::Yes);
      }

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

      // Hiding the other popovers could have fired beforetoggle, whose listeners could have disconnected the
      // element or changed its popover attribute.
      validity = CheckPopoverValidity(false, document.get());
      if (!IsValid(validity))
      {
        cleanupShowingSteps();
        return InvalidPopoverResult(krys::move(validity), throwExceptions);
      }

      // The focus returns to the previously focused element only for the first popover in a stack.
      if (!document->TopmostAutoOrHintPopover())
      {
        shouldRestoreFocus = true;
      }

      GetPopoverData()->SetOpenedInPopoverMode(effectiveType);
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

    GetPopoverData()->SetPreviouslyFocusedElement(nullptr);
    RefPtr<dom::Element> originallyFocusedElement = ShareRefPtr(document->FocusedElement());

    AddToTopLayer();

    if (effectiveType == dom::PopoverState::Hint && ancestor
        && ancestor->GetPopoverData()->OpenedInPopoverMode() == dom::PopoverState::Auto)
    {
      document->SetHintStackParent(ancestor.get());
    }

    {
      style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::PopoverOpen,
                                                             true);
      GetPopoverData()->SetVisibilityState(dom::PopoverVisibilityState::Showing);
    }

    // The invoker is also the popover's implicit anchor element.
    SetInvoker(source);

    RunPopoverFocusingSteps();

    if (shouldRestoreFocus && GetPopoverData())
    {
      GetPopoverData()->SetPreviouslyFocusedElement(originallyFocusedElement.get());
    }

    cleanupShowingSteps();
    if (GetPopoverData())
    {
      QueuePopoverToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open, source);
    }
    return {};
  }

  dom::ExceptionOr<void> HTMLElement::HidePopoverInternal(dom::FocusPreviousElement focusPreviousElement,
                                                          dom::FireEvents fireEvents, bool throwExceptions,
                                                          HTMLElement *source) noexcept
  {
    auto validity = CheckPopoverValidity(true, nullptr);
    if (!IsValid(validity))
    {
      return InvalidPopoverResult(krys::move(validity), throwExceptions);
    }

    Ref<dom::Document> document = ShareRef(NodeDocument());
    Ref<HTMLElement> protectedThis = ShareRef(*this);

    bool nestedHide = GetPopoverData()->IsHidingPopover();
    GetPopoverData()->SetIsHidingPopover(true);
    if (nestedHide)
    {
      fireEvents = dom::FireEvents::No;
    }
    document->IncrementHidingPopoverNestingCount();
    auto cleanupSteps = [&]
    {
      if (!nestedHide && GetPopoverData())
      {
        GetPopoverData()->SetIsHidingPopover(false);
      }
      document->DecrementHidingPopoverNestingCount();
    };

    bool autoPopoverListContainsElement = ListContains(document->ShowingAutoPopoverList(), *this);
    bool hintPopoverListContainsElement = ListContains(document->ShowingHintPopoverList(), *this);

    if (auto mode = GetPopoverData()->OpenedInPopoverMode();
        mode == dom::PopoverState::Auto || mode == dom::PopoverState::Hint)
    {
      if (hintPopoverListContainsElement)
      {
        document->HidePopoverStackUntil(this, dom::PopoverState::Hint, focusPreviousElement, fireEvents);
      }
      // If the hint stack parent is being hidden, then all hint popovers are hidden.
      if (this == document->HintStackParent())
      {
        document->HidePopoverStackUntil(nullptr, dom::PopoverState::Hint, focusPreviousElement, fireEvents);
      }
      if (autoPopoverListContainsElement)
      {
        document->HidePopoverStackUntil(this, dom::PopoverState::Auto, focusPreviousElement, fireEvents);
      }

      // Hiding the other popovers could have disconnected the element or changed its popover attribute.
      validity = CheckPopoverValidity(true, document.get());
      if (!IsValid(validity))
      {
        cleanupSteps();
        return InvalidPopoverResult(krys::move(validity), throwExceptions);
      }
    }

    if (fireEvents == dom::FireEvents::Yes)
    {
      (void)FirePopoverBeforeToggleEvent(dom::ToggleState::Open, dom::ToggleState::Closed, false, source);

      // Firing beforetoggle could have disconnected the element or changed its popover attribute.
      validity = CheckPopoverValidity(true, document.get());
      if (!IsValid(validity))
      {
        cleanupSteps();
        return InvalidPopoverResult(krys::move(validity), throwExceptions);
      }

      RequestRemoveFromTopLayer();
    }
    else if (IsInTopLayer())
    {
      RemoveFromTopLayer();
    }

    SetInvoker(nullptr);
    GetPopoverData()->SetOpenedInPopoverMode(dom::PopoverState::None);
    {
      Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
      if (ParentNode())
      {
        styleInvalidation.emplace(*this, css::SelectorPseudoClass::PopoverOpen, false);
      }
      GetPopoverData()->SetVisibilityState(dom::PopoverVisibilityState::Hidden);
    }

    if (this == document->HintStackParent() || document->ShowingHintPopoverList().empty())
    {
      document->SetHintStackParent(nullptr);
    }

    if (fireEvents == dom::FireEvents::Yes)
    {
      QueuePopoverToggleEventTask(dom::ToggleState::Open, dom::ToggleState::Closed, source);
    }

    if (RefPtr<dom::Element> previouslyFocusedElement =
          ShareRefPtr(GetPopoverData()->PreviouslyFocusedElement()))
    {
      GetPopoverData()->SetPreviouslyFocusedElement(nullptr);
      auto *focusedElement = document->FocusedElement();
      if (focusPreviousElement == dom::FocusPreviousElement::Yes && focusedElement
          && dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*focusedElement, *this))
      {
        FocusAlgorithms::FocusingSteps(*previouslyFocusedElement, nullptr, {.PreventScroll = true});
      }
    }

    // The focusing steps' events could have removed the popover attribute, which destroys the close watcher.
    if (auto *popoverData = GetPopoverData())
    {
      if (RefPtr<CloseWatcher> closeWatcher = ShareRefPtr(popoverData->CloseWatcher()))
      {
        closeWatcher->Destroy();
        popoverData->SetCloseWatcher(nullptr);
      }
    }

    cleanupSteps();
    return {};
  }

  dom::ExceptionOr<bool> HTMLElement::CheckPopoverValidity(bool expectedToBeShowing,
                                                           dom::Document *expectedDocument) const noexcept
  {
    if (GetPopoverState() == dom::PopoverState::None)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError,
                             u8"Element does not have the popover attribute."};
    }

    if (IsPopoverShowing() != expectedToBeShowing)
    {
      return false;
    }

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
    return true;
  }

  void HTMLElement::RunPopoverFocusingSteps() noexcept
  {
    // FIXME: If the allow focus steps given subject's node document return false, then return, with
    // permissions policy.

    if (auto *dialog = DynamicDowncast<HTMLDialogElement>(*this))
    {
      dialog->RunFocusingSteps();
      return;
    }

    // Let control be subject if it has the autofocus attribute, else its autofocus delegate.
    RefPtr<dom::Node> control =
      HasAttribute(u8"autofocus")
        ? ShareRefPtr<dom::Node>(this)
        : ShareRefPtr(FocusAlgorithms::AutofocusDelegate(*this, dom::FocusTrigger::Other));
    if (!control)
    {
      return;
    }

    Ref<dom::Document> controlDocument = ShareRef(control->NodeDocument());
    if (!controlDocument->GetPage())
    {
      return;
    }

    FocusAlgorithms::FocusingSteps(*control);

    // Empty the top document's autofocus candidates, and set its autofocus processed flag.
    controlDocument->AutofocusCandidates().clear();
    controlDocument->SetAutofocusProcessed();
  }

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
    auto event = dom::ToggleEvent::Create(dom::DOMStringAtom(u8"beforetoggle"), init, cancelable);
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

  void HTMLElement::PopoverAttributeChanged(Maybe<dom::DOMStringView> value) noexcept
  {
    auto computePopoverState = [](Maybe<dom::DOMStringView> value)
    {
      auto state = EnumeratedAttribute<"popover", HTMLElement>::ResolveState(
        value ? Maybe<dom::DOMString>(dom::DOMString(*value)) : null);
      if (!state)
      {
        return dom::PopoverState::None;
      }
      switch (*state)
      {
        case AttributeState::Auto: return dom::PopoverState::Auto;
        case AttributeState::Hint: return dom::PopoverState::Hint;
        default:                   return dom::PopoverState::Manual;
      }
    };

    auto newPopoverState = computePopoverState(value);
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
      auto currentValue = GetAttribute(dom::DOMStringAtom(u8"popover"));
      newPopoverState = computePopoverState(currentValue ? Maybe<dom::DOMStringView>(*currentValue) : null);
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
  }

#pragma endregion

  // https://html.spec.whatwg.org/#dom-contenteditable
  dom::DOMString HTMLElement::ContentEditable() const noexcept
  {
    // The contentEditable IDL attribute, on getting, must return the string "true" if the content attribute
    // is set to the True state, "plaintext-only" if the content attribute is set to the Plaintext-Only state,
    // "false" if the content attribute is set to the False state, and "inherit" otherwise.
    auto value = GetAttribute(u8"contenteditable");
    switch (value ? GetContentEditableType(*value) : ContentEditableType::Inherit)
    {
      case ContentEditableType::True:          return u8"true";
      case ContentEditableType::PlaintextOnly: return u8"plaintext-only";
      case ContentEditableType::False:         return u8"false";
      case ContentEditableType::Inherit:       break;
    }
    return u8"inherit";
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
    // FIXME: A page that's editable as a whole.
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
      auto value = element->GetAttribute(u8"contenteditable");
      if (!value)
      {
        continue;
      }
      switch (GetContentEditableType(*value))
      {
        case ContentEditableType::True:          return dom::Node::Editability::CanEditRichly;
        case ContentEditableType::PlaintextOnly: return dom::Node::Editability::CanEditPlainText;
        case ContentEditableType::False:         return dom::Node::Editability::ReadOnly;
        case ContentEditableType::Inherit:       break;
      }
    }

    auto *containingShadowRoot = dom::TreeQueries::ContainingShadowRoot(node);
    if (containingShadowRoot && containingShadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
    {
      return dom::Node::Editability::ReadOnly;
    }

    // FIXME: The document's design mode makes the document tree editable, once there's a design mode.
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
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitLocale,
                                           css::SerialiseString(css::CSSOMString(value)));
    }
    else
    {
      // The empty string means the language is explicitly unknown.
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitLocale, css::ValueId::Auto);
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
      switch (GetContentEditableType(value))
      {
        case ContentEditableType::Inherit:
        {
          return;
        }
        case ContentEditableType::False:
        {
          userModifyValue = css::ValueId::ReadOnly;
          break;
        }
        case ContentEditableType::PlaintextOnly:
        case ContentEditableType::True:
        {
          if (GetContentEditableType(value) == ContentEditableType::PlaintextOnly)
          {
            userModifyValue = css::ValueId::ReadWritePlaintextOnly;
          }
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::BreakWord);
          // FIXME: -webkit-nbsp-mode: space is dropped with the property.
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::LineBreak,
                                               css::ValueId::AfterWhiteSpace);
          break;
        }
      }
      AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitUserModify, userModifyValue);
    }
    else if (localName == u8"hidden")
    {
      // The hiddenUntilFoundEnabled setting is on by default.
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"until-found"))
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
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"true"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitUserDrag, css::ValueId::Element);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"false"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WebkitUserDrag, css::ValueId::None);
      }
    }
    else if (localName == u8"dir")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"auto"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::UnicodeBidi,
                                             UnicodeBidiAttributeForDirAuto(*this));
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"rtl")
               || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"ltr"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::Direction, css::CSSOMString(value));
        auto tagName = TagQualifiedName().TagName();
        if (tagName != TagName::bdi && tagName != TagName::bdo && tagName != TagName::output)
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::UnicodeBidi, css::ValueId::Isolate);
        }
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
      verticalAlignValue = css::ValueId::WebkitBaselineMiddle;
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
