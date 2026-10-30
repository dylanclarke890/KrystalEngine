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
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/SimulatedClick.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/LabelsNodeList.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Dimensions.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

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
    return Reflection::Reflect<dom::DOMString>(*this, u8"title").value_or(u8"");
  }

  void HTMLElement::Title(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<dom::DOMString>(*this, u8"title", krys::move(value));
  }

  dom::DOMString HTMLElement::Lang() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"lang").value_or(u8"");
  }

  void HTMLElement::Lang(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<dom::DOMString>(*this, u8"lang", krys::move(value));
  }

  bool HTMLElement::Translate() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"translate");
    auto state = EnumeratedAttribute<"translate", HTMLElement>::ResolveState(value);
    return state == AttributeState::Yes;
  }

  void HTMLElement::Translate(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<dom::DOMString>(*this, u8"translate", value ? u8"yes" : u8"no");
  }

  dom::DOMString HTMLElement::Dir() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"dir");
    return EnumeratedAttribute<"dir", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      krys::move(value));
  }

  void HTMLElement::Dir(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<dom::DOMString>(*this, u8"dir", krys::move(value));
  }

  BoolOr<dom::DOMString> HTMLElement::Hidden() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"hidden");

    auto state = EnumeratedAttribute<"hidden", HTMLElement>::ResolveState(value);
    if (state == AttributeState::UntilFound)
    {
      return u8"until-found";
    }

    return state == AttributeState::Hidden;
  }

  bool HTMLElement::IsHiddenUntilFound() const noexcept
  {
    auto value = GetAttribute(u8"hidden");
    return value && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(*value, u8"until-found");
  }

  void HTMLElement::Hidden(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"until-found"))
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"until-found");
    }
    else if (value.empty())
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  void HTMLElement::Hidden(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    if (value)
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
    else
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
  }

  void HTMLElement::Hidden(double value) noexcept
  {
    CEReactionsScope ceReactions;
    if (value == 0)
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else if (std::isnan(value))
    {
      dom::ElementAlgorithms::RemoveAttributeByName(u8"hidden", *this);
    }
    else
    {
      dom::ElementAlgorithms::SetAttributeValue(*this, u8"hidden", u8"");
    }
  }

  bool HTMLElement::Inert() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"inert");
  }

  void HTMLElement::Inert(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<bool>(*this, u8"inert", krys::move(value));
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

  bool HTMLElement::IsActuallyDisabled() const noexcept
  {
    return CanBeActuallyDisabled() && IsDisabledFormControl();
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

    // FIXME: The same list each time ([SameObject]), once nodes cache their lists.
    return CreateRef<LabelsNodeList>(*this);
  }

  void HTMLElement::Click() noexcept
  {
    (void)dom::SimulateClick(*this, nullptr, dom::SimulatedClickMouseEventOptions::SendNoEvents,
                             dom::SimulatedClickVisualOptions::DoNotShowPressedLook,
                             dom::SimulatedClickSource::Bindings);
  }

  dom::DOMString HTMLElement::AccessKey() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"accesskey").value_or(u8"");
  }

  void HTMLElement::AccessKey(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<dom::DOMString>(*this, u8"accesskey", krys::move(value));
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
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"draggable");
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

  bool HTMLElement::SpellCheck() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"spellcheck");
    switch (EnumeratedAttribute<"spellcheck", HTMLElement>::ResolveState(value))
    {
      case AttributeState::True:  return true;
      case AttributeState::False: return false;
      case AttributeState::Default:
      {
        // FIXME: Implement default behavior for SpellCheck() getter.
        // The spellcheck IDL attribute, on getting, must return true if the element's spellcheck content
        // attribute is in the True state, or if the element's spellcheck content attribute is in the Default
        // state and the element's default behavior is true-by-default, or if the element's spellcheck content
        // attribute is in the Default state and the element's default behavior is inherit-by-default and the
        // element's parent element's spellcheck IDL attribute would return true; otherwise, if none of those
        // conditions applies, then the attribute must instead return false.
        return false;
      }
    }

    return false;
  }

  void HTMLElement::SpellCheck(bool value) noexcept
  {
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"spellcheck", value ? u8"true" : u8"false");
  }

  dom::DOMString HTMLElement::WritingSuggestions() const noexcept
  {
    // FIXME: Implement WritingSuggestions() method.
    // Return this's computed writing suggestions value.
    return {};
  }

  void HTMLElement::WritingSuggestions(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::ReflectSetter(*this, u8"writingsuggestions", krys::move(value));
  }

  dom::DOMString HTMLElement::Autocapitalize() const noexcept
  {
    // FIXME: Implement Autocapitalize() method.
    // Let state be the own autocapitalization hint of this.
    // If state is Default, then return the empty string.
    // If state is None, then return "none".
    // If state is Sentences, then return "sentences".
    // Return the keyword value corresponding to state.
    return {};
  }

  void HTMLElement::Autocapitalize(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::ReflectSetter(*this, u8"autocapitalize", krys::move(value));
  }

  bool HTMLElement::Autocorrect() const noexcept
  {
    // FIXME: Implement Autocorrect() method.
    // The autocorrect getter steps are: return true if the element's used autocorrection state is On and
    // false if the element's used autocorrection state is Off
    return false;
  }

  void HTMLElement::Autocorrect(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"autocorrect", value ? u8"on" : u8"off");
  }

  dom::DOMString HTMLElement::InnerText() const noexcept
  {
    auto &self = const_cast<HTMLElement &>(*this);

    // We need to update layout, since plainText uses line boxes in the render tree.
    self.NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (!Renderer())
    {
      return TextContent().value_or(dom::DOMString {});
    }

    if (Renderer()->IsSkippedContent())
    {
      return {};
    }

    return PlainText(dom::MakeRangeSelectingNodeContents(self));
  }

  void HTMLElement::InnerText(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    // FIXME: This doesn't take whitespace collapsing into account at all.

    if (value.find_first_of(u8"\r\n") == dom::DOMString::npos)
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
      return;
    }

    if (IsConnected() && IsTextControlInnerTextElement())
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(infra::StringAlgorithms::NormalizeNewlines(value), *this);
      return;
    }

    // Add text nodes and <br> elements.
    auto &document = NodeDocument();
    auto fragment = document.CreateDocumentFragment();
    for (size_t start = 0, length = value.size(); start < length;)
    {
      // Find next line break.
      char8 c = 0;
      size_t i;
      for (i = start; i < length; i++)
      {
        c = value[i];
        if (c == u8'\r' || c == u8'\n')
        {
          break;
        }
      }

      // If text is not the empty string, then append a new Text node whose data is text and node document is
      // document to fragment.
      if (i > start)
      {
        (void)fragment->AppendChild(*document.CreateTextNode(value.substr(start, i - start)));
      }

      if (i == length)
      {
        break;
      }

      (void)fragment->AppendChild(*CreateRef<HTMLBRElement>(document));
      // Make sure \r\n doesn't result in two line breaks.
      if (c == u8'\r' && i + 1 < length && value[i + 1] == u8'\n')
      {
        ++i;
      }

      start = i + 1; // Character after line break.
    }

    (void)dom::MutationAlgorithms::ReplaceAll(fragment.get(), *this);
  }

  dom::DOMString HTMLElement::OuterText() const noexcept
  {
    // FIXME: Implement OuterText() method.
    krys_not_implemented();
  }

  void HTMLElement::OuterText(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    // FIXME: Implement OuterText() getter method.
    (void)value;
    krys_not_implemented();
  }

  dom::ExceptionOr<Ref<ElementInternals>> HTMLElement::AttachInternals() noexcept
  {
    // NOTE: customized built-in elements aren't supported, so an element's is value doesn't matter.
    auto *queue = ReactionQueue();
    if (!queue)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (queue->IsElementInternalsDisabled())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (queue->IsElementInternalsAttached())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (!IsPrecustomizedOrDefinedCustomElement())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

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
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"popover");
    return EnumeratedAttribute<"popover", HTMLElement>::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(
      krys::move(value));
  }

  void HTMLElement::Popover(Maybe<dom::DOMString> &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"popover", krys::move(value));
  }

  uint32 HTMLElement::HeadingOffset() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectRange<uint32>(0u, 8u)>(*this, u8"headingoffset");
  }

  void HTMLElement::HeadingOffset(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"headingoffset", krys::move(value));
  }

  bool HTMLElement::HeadingReset() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"headingreset");
  }

  void HTMLElement::HeadingReset(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"headingreset", krys::move(value));
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

  // Color parsing that matches HTML's "rules for parsing a legacy color value"
  Maybe<SRGBA<uint8>> HTMLElement::ParseLegacyColorValue(dom::DOMStringView string) noexcept
  {
    // An empty string doesn't apply a color.
    if (string.empty())
    {
      return {};
    }

    string = infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(string);
    if (string.empty())
    {
      return boo::Color::black;
    }

    // "transparent" doesn't apply a color either.
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(string, u8"transparent"))
    {
      return {};
    }

    if (auto namedColor = css::ParserFastPaths::ParseNamedColor(string))
    {
      return namedColor;
    }

    if (string.size() == 4 && string[0] == '#' && text::IsASCIIHexDigit(string[1])
        && text::IsASCIIHexDigit(string[2]) && text::IsASCIIHexDigit(string[3]))
    {
      return SRGBA<uint8> {static_cast<uint8>(text::ToASCIIHexValue(string[1]) * 0x11),
                           static_cast<uint8>(text::ToASCIIHexValue(string[2]) * 0x11),
                           static_cast<uint8>(text::ToASCIIHexValue(string[3]) * 0x11)};
    }

    // Per spec, only look at the first 128 digits of the string.
    constexpr size_t maxColorLength = 128;

    // We'll pad the buffer with two extra 0s later, so reserve two more than the max.
    SmallList<char8, maxColorLength + 2> digitBuffer;

    // Grab the first 128 code points, replacing non-hex characters with 0. Code points outside the BMP become
    // "00", as they're two code units in UTF-16.
    size_t i = 0;
    if (string[0] == '#') // Skip a leading #.
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

    // Pad the buffer out to at least the next multiple of three in size.
    digitBuffer.push_back('0');
    digitBuffer.push_back('0');

    if (digitBuffer.size() < 6)
    {
      return SRGBA<uint8> {text::ToASCIIHexValue(digitBuffer[0]), text::ToASCIIHexValue(digitBuffer[1]),
                           text::ToASCIIHexValue(digitBuffer[2])};
    }

    // Split the digits into three components, then search the last 8 digits of each component.
    krys_debug_assert(digitBuffer.size() >= 6);
    size_t componentLength = digitBuffer.size() / 3;
    size_t componentSearchWindowLength = std::min<size_t>(componentLength, 8);
    size_t redIndex = componentLength - componentSearchWindowLength;
    size_t greenIndex = componentLength * 2 - componentSearchWindowLength;
    size_t blueIndex = componentLength * 3 - componentSearchWindowLength;
    // Skip digits until one of them is non-zero, or we've only got two digits left in the component.
    while (digitBuffer[redIndex] == '0' && digitBuffer[greenIndex] == '0' && digitBuffer[blueIndex] == '0'
           && (componentLength - redIndex) > 2)
    {
      redIndex++;
      greenIndex++;
      blueIndex++;
    }
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
