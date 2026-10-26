#include "Krystal.Booey/HTML/TextFieldInputType.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo::html
{
  namespace
  {
    /// @brief @p string truncated to @p maxLength UTF-16 code units, without splitting a code point.
    KRYS_NODISCARD dom::DOMString LimitLength(dom::DOMStringView string, size_t maxLength) noexcept
    {
      size_t length = 0;
      for (size_t i = 0; i < string.size(); ++i)
      {
        auto byte = static_cast<uint8>(string[i]);
        if ((byte & 0xC0) == 0x80)
        {
          continue;
        }
        size_t units = byte >= 0xF0 ? 2 : 1;
        if (length + units > maxLength)
        {
          return dom::DOMString(string.substr(0, i));
        }
        length += units;
      }
      return dom::DOMString(string);
    }

    /// @brief @p text with every occurrence of @p from replaced by @p to.
    KRYS_NODISCARD dom::DOMString ReplaceAll(dom::DOMStringView text, dom::DOMStringView from,
                                             dom::DOMStringView to) noexcept
    {
      dom::DOMString result;
      size_t start = 0;
      for (size_t found; (found = text.find(from, start)) != dom::DOMStringView::npos;
           start = found + from.size())
      {
        result.append(text.substr(start, found - start));
        result.append(to);
      }
      result.append(text.substr(start));
      return result;
    }
  }

  TextFieldInputType::TextFieldInputType(Type type, HTMLInputElement &element) noexcept
      : InputType(type, element)
  {
    krys_debug_assert(NeedsShadowSubtree());
  }

  TextFieldInputType::~TextFieldInputType() noexcept
  {
    if (auto innerSpinButton = _innerSpinButton)
    {
      innerSpinButton->RemoveSpinButtonOwner();
    }
  }

  bool TextFieldInputType::IsEmptyValue() const noexcept
  {
    auto innerText = InnerTextElement();
    if (!innerText)
    {
      if (Element()->UserAgentShadowRoot())
      {
        return true; // Shadow tree is empty
      }
      return VisibleValue().empty();
    }

    for (auto *node = innerText->FirstChild(); node; node = dom::TreeQueries::Next(*node, innerText.get()))
    {
      if (auto *text = DynamicDowncast<dom::Text>(*node); text && text->Length())
      {
        return false;
      }
    }
    return true;
  }

  bool TextFieldInputType::ValueMissing(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    return element.IsMutable() && element.IsRequired() && value.empty();
  }

  void TextFieldInputType::SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                                    TextFieldEventBehavior eventBehavior,
                                    TextControlSetValueSelection selection) noexcept
  {
    krys_debug_assert(Element());

    // Grab this input element to keep reference even if JS event handler
    // changes input type.
    Ref<HTMLInputElement> input = ShareRef(*Element());

    // We don't ask InputType::setValue to dispatch events because
    // TextFieldInputType dispatches events different way from InputType.
    InputType::SetValue(sanitizedValue, valueChanged, TextFieldEventBehavior::DispatchNoEvent, selection);

    // Visible value needs update if it differs from sanitized value, if it was set with setValue().
    // event_behavior == DispatchNoEvent usually means this call is not a user edit.
    bool needsTextUpdate = valueChanged
                           || (eventBehavior == TextFieldEventBehavior::DispatchNoEvent
                               && sanitizedValue != input->InnerTextValue());
    if (needsTextUpdate)
    {
      UpdateInnerTextValue();
    }
    if (!valueChanged)
    {
      return;
    }

    if (selection == TextControlSetValueSelection::SetSelectionToEnd)
    {
      auto max = static_cast<uint32>(VisibleValue().size());
      if (input->Focused())
      {
        input->SetSelectionRange(max, max);
      }
      else
      {
        input->CacheSelectionInResponseToSetValue(max);
      }
    }

    switch (eventBehavior)
    {
      case TextFieldEventBehavior::DispatchChangeEvent:
        // If the user is still editing this field, dispatch an input event rather than a change event.
        // The change event will be dispatched when editing finishes.
        if (input->Focused())
        {
          input->DispatchFormControlInputEvent();
        }
        else
        {
          input->DispatchFormControlChangeEvent();
        }
        break;

      case TextFieldEventBehavior::DispatchInputAndChangeEvent:
        input->DispatchFormControlInputEvent();
        input->DispatchFormControlChangeEvent();
        break;

      case TextFieldEventBehavior::DispatchNoEvent: break;
    }

    if (!input->Focused())
    {
      input->SetTextAsOfLastFormControlChangeEvent(dom::DOMString(sanitizedValue));
    }

    // FIXME: Telling the editor client a value the user typed changed (didSetValueByUserEdit while processing
    // a user typing gesture), with the user typing gesture indicator.
  }

  void TextFieldInputType::HandleClickEvent(dom::MouseEvent &) noexcept
  {
    // FIXME: Showing the datalist's suggestions, with datalists.
  }

  InputType::ShouldCallBaseEventHandler
    TextFieldInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (!element->Focused())
    {
      return ShouldCallBaseEventHandler::Yes;
    }
    // FIXME: The datalist suggestion picker's keys, with datalists.
    if (element->NodeDocument().Frame()
        && element->NodeDocument().GetEditor().DoTextFieldCommandFromEvent(*element, &event))
    {
      event.SetDefaultHandled();
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void TextFieldInputType::HandleKeydownEventForSpinButton(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    if (!Element()->IsMutable())
    {
      return;
    }
    // FIXME: Not while the datalist suggestion picker is open, with datalists.
    auto key = event.KeyIdentifier();
    if (key == u8"Up")
    {
      StepUpFromRenderer(1);
    }
    else if (key == u8"Down")
    {
      StepUpFromRenderer(-1);
    }
    else
    {
      return;
    }
    event.SetDefaultHandled();
  }

  void TextFieldInputType::ForwardEvent(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());

    bool isFocusEvent = event.Type() == EventNames::Focus;
    bool isBlurEvent = event.Type() == EventNames::Blur;
    // FIXME: The caps lock indicator (capsLockStateMayHaveChanged), with the caps lock indicator.
    if (event.IsMouseEvent() || isFocusEvent || isBlurEvent)
    {
      Element()->ForwardEvent(event);
    }
  }

  void TextFieldInputType::ElementDidBlur() noexcept
  {
    krys_debug_assert(Element());
    auto *renderer = Element()->Renderer();
    if (!renderer)
    {
      return;
    }

    auto *innerTextRenderer = InnerTextElement()->Renderer();
    if (!innerTextRenderer)
    {
      return;
    }

    auto *innerLayer = innerTextRenderer->Layer();
    if (!innerLayer)
    {
      return;
    }

    auto *innerLayerScrollable = innerLayer->EnsureLayerScrollableArea();

    bool isLeftToRightDirection =
      Downcast<RenderTextControlSingleLine>(*renderer).Style().IsLeftToRightDirection();
    ScrollOffset scrollOffset(isLeftToRightDirection ? 0 : innerLayerScrollable->ScrollWidth(), 0);
    innerLayerScrollable->ScrollToOffset(scrollOffset);

    // FIXME: Closing the datalist's suggestions, with datalists.
  }

  void TextFieldInputType::HandleFocusEvent(dom::Node *oldFocusedNode) noexcept
  {
    krys_debug_assert(Element());
    krys_debug_assert(oldFocusedNode != Element());
    (void)oldFocusedNode;
    auto &element = *Element();
    if (element.NodeDocument().Frame())
    {
      element.NodeDocument().GetEditor().TextFieldDidBeginEditing(element);
    }
  }

  void TextFieldInputType::HandleBlurEvent() noexcept
  {
    InputType::HandleBlurEvent();
    krys_debug_assert(Element());
    Element()->EndEditing();
  }

  bool TextFieldInputType::ShouldSubmitImplicitly(dom::Event &event) noexcept
  {
    auto *textEvent = DynamicDowncast<dom::TextEvent>(event);
    return (event.Type() == EventNames::TextInput && textEvent && textEvent->Data() == u8"\n")
           || InputType::ShouldSubmitImplicitly(event);
  }

  RenderPtr<RenderElement> TextFieldInputType::CreateInputRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(Element());
    return CreateRenderer<RenderTextControlSingleLine>(RenderObject::Type::TextControlSingleLine, *Element(),
                                                       krys::move(style));
  }

  bool TextFieldInputType::NeedsContainer() const noexcept
  {
    return false;
  }

  bool TextFieldInputType::ShouldHaveSpinButton() const noexcept
  {
    krys_debug_assert(Element());
    return RenderTheme::Singleton().ShouldHaveSpinButton(*Element());
  }

  void TextFieldInputType::CreateShadowSubtree() noexcept
  {
    krys_debug_assert(NeedsShadowSubtree());
    krys_debug_assert(Element());
    krys_debug_assert(Element()->UserAgentShadowRoot());
    krys_debug_assert(!Element()->UserAgentShadowRoot()->HasChildNodes());

    krys_debug_assert(!_innerText);
    krys_debug_assert(!_innerBlock);

    Ref<HTMLInputElement> element = ShareRef(*Element());
    auto &document = element->NodeDocument();
    bool shouldHaveSpinButton = ShouldHaveSpinButton();
    // FIXME: The caps lock indicator, the autofill button and the datalist dropdown also need a container.
    bool createsContainer = shouldHaveSpinButton || NeedsContainer();

    auto innerText = TextControlInnerTextElement::Create(document, element->IsInnerTextElementEditable());
    _innerText = innerText;

    auto &shadowRoot = *element->UserAgentShadowRoot();
    if (!createsContainer)
    {
      (void)shadowRoot.AppendChild(*innerText);
      UpdatePlaceholderText();
      UpdateInnerTextValue();
      return;
    }

    CreateContainer(PreserveSelectionRange::No);
    UpdatePlaceholderText();
    UpdateInnerTextValue();

    if (shouldHaveSpinButton)
    {
      auto innerSpinButton = SpinButtonElement::Create(document, *this);
      _innerSpinButton = innerSpinButton;
      (void)_container->AppendChild(*innerSpinButton);
    }

    // FIXME: The caps lock indicator, the autofill button and the datalist dropdown.
  }

  HTMLElement *TextFieldInputType::ContainerElement() const noexcept
  {
    return _container.get();
  }

  HTMLElement *TextFieldInputType::InnerBlockElement() const noexcept
  {
    return _innerBlock.get();
  }

  RefPtr<TextControlInnerTextElement> TextFieldInputType::InnerTextElement() const noexcept
  {
    return _innerText;
  }

  HTMLElement *TextFieldInputType::PlaceholderElement() const noexcept
  {
    return _placeholder.get();
  }

  HTMLElement *TextFieldInputType::InnerSpinButtonElement() const noexcept
  {
    return _innerSpinButton.get();
  }

  void TextFieldInputType::RemoveShadowSubtree() noexcept
  {
    InputType::RemoveShadowSubtree();
    _innerText = nullptr;
    _placeholder = nullptr;
    _innerBlock = nullptr;
    if (auto innerSpinButton = _innerSpinButton)
    {
      innerSpinButton->RemoveSpinButtonOwner();
    }
    _innerSpinButton = nullptr;
    _container = nullptr;
  }

  void TextFieldInputType::AttributeChanged(dom::DOMStringAtom name) noexcept
  {
    if (name == u8"value" || name == u8"placeholder")
    {
      if (Element() && Element()->UserAgentShadowRoot())
      {
        UpdateInnerTextValue();
      }
    }
    InputType::AttributeChanged(name);
  }

  void TextFieldInputType::DisabledStateChanged() noexcept
  {
    if (!HasCreatedShadowSubtree())
    {
      return;
    }

    if (auto innerSpinButton = _innerSpinButton)
    {
      innerSpinButton->ReleaseCapture();
    }
    // FIXME: The caps lock indicator and the autofill button, with them.
  }

  void TextFieldInputType::ReadOnlyStateChanged() noexcept
  {
    if (!HasCreatedShadowSubtree())
    {
      return;
    }

    if (auto innerSpinButton = _innerSpinButton)
    {
      innerSpinButton->ReleaseCapture();
    }
    // FIXME: The caps lock indicator and the autofill button, with them.
  }

  void TextFieldInputType::SpinButtonStepDown() noexcept
  {
    StepUpFromRenderer(-1);
  }

  void TextFieldInputType::SpinButtonStepUp() noexcept
  {
    StepUpFromRenderer(1);
  }

  void TextFieldInputType::FocusAndSelectSpinButtonOwner() noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> input = ShareRef(*Element());
    input->Focus();
    input->Select();
  }

  bool TextFieldInputType::ShouldSpinButtonRespondToMouseEvents() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->IsMutable();
  }

  bool TextFieldInputType::SupportsReadOnly() const noexcept
  {
    return true;
  }

  bool TextFieldInputType::ShouldUseInputMethod() const noexcept
  {
    return true;
  }

  dom::DOMString TextFieldInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    if (std::ranges::none_of(proposedValue, IsHTMLLineBreak)) KRYS_LIKELY
    {
      return LimitLength(proposedValue, HTMLInputElement::MaxEffectiveLength);
    }

    dom::DOMString proposedValueWithoutLineBreaks;
    for (char8 character : proposedValue)
    {
      if (!IsHTMLLineBreak(character))
      {
        proposedValueWithoutLineBreaks += character;
      }
    }
    return LimitLength(proposedValueWithoutLineBreaks, HTMLInputElement::MaxEffectiveLength);
  }

  void TextFieldInputType::HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &event) noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    // Make sure that the text to be inserted will not violate the maxLength.

    // We use RenderTextControlSingleLine::text() instead of InputElement::value()
    // because they can be mismatched by sanitizeValue() in
    // HTMLInputElement::subtreeHasChanged() in some cases.
    auto innerText = element.InnerTextValue();
    auto oldLength = HTMLTextFormControlElement::LengthForAPIValue(innerText);

    // selectionLength represents the selection length of this text field to be
    // removed by this insertion.
    // If the text field has no focus, we don't need to take account of the
    // selection length. The selection is the source of text drag-and-drop in
    // that case, and nothing in the text field will be removed.
    size_t selectionLength = 0;
    if (element.Focused())
    {
      krys_debug_assert(EnclosingTextFormControl(element.NodeDocument().Selection().Selection().Start())
                        == &element);
      auto selectionStart = std::min<size_t>(element.SelectionStart(), innerText.size());
      auto selectionEnd = std::min<size_t>(element.SelectionEnd(), innerText.size());
      krys_debug_assert(selectionStart <= selectionEnd);
      selectionLength = HTMLTextFormControlElement::LengthForAPIValue(
        dom::DOMStringView(innerText).substr(selectionStart, selectionEnd - selectionStart));
    }
    krys_debug_assert(oldLength >= selectionLength);

    // Selected characters will be removed by the next text event.
    size_t baseLength = oldLength - selectionLength;
    size_t maxLength = IsTextType() ? element.EffectiveMaxLength() : HTMLInputElement::MaxEffectiveLength;
    size_t appendableLength = maxLength > baseLength ? maxLength - baseLength : 0;

    // Truncate the inserted text to avoid violating the maxLength and other constraints.
    dom::DOMStringView eventText = event.Text();
    size_t textLength = eventText.size();
    while (textLength > 0 && IsHTMLLineBreak(eventText[textLength - 1]))
    {
      textLength--;
    }
    auto text = ReplaceAll(eventText.substr(0, textLength), u8"\r\n", u8" ");
    std::ranges::replace(text, u8'\r', u8' ');
    std::ranges::replace(text, u8'\n', u8' ');
    event.SetText(LimitLength(text, appendableLength));
  }

  void TextFieldInputType::UpdatePlaceholderText() noexcept
  {
    krys_debug_assert(Element());

    if (!HasCreatedShadowSubtree())
    {
      return;
    }

    if (!SupportsPlaceholder())
    {
      return;
    }

    Ref<HTMLInputElement> element = ShareRef(*Element());
    auto placeholderText = element->Placeholder();
    if (placeholderText.empty())
    {
      if (auto placeholder = std::exchange(_placeholder, nullptr))
      {
        placeholder->Remove();
      }
      return;
    }
    if (!_placeholder)
    {
      auto placeholder = TextControlPlaceholderElement::Create(element->NodeDocument());
      _placeholder = placeholder;
      auto &shadowRoot = *element->UserAgentShadowRoot();
      if (_container)
      {
        (void)shadowRoot.InsertBefore(*placeholder, _container.get());
      }
      else
      {
        (void)shadowRoot.InsertBefore(*placeholder, InnerTextElement().get());
      }
    }
    _placeholder->InnerText(krys::move(placeholderText));
  }

  bool TextFieldInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    InputType::AppendFormData(formData);
    krys_debug_assert(Element());
    auto &element = *Element();
    // FIXME: should type=number be TextFieldInputType to begin with?
    if (element.IsNumberField())
    {
      return true;
    }
    if (auto dirname = element.GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, element.DirectionForFormData());
    }
    return true;
  }

  dom::DOMString TextFieldInputType::ConvertFromVisibleValue(dom::DOMStringView visibleValue) const noexcept
  {
    return dom::DOMString(visibleValue);
  }

  void TextFieldInputType::SubtreeHasChanged() noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    element->SetChangedSinceLastFormControlChangeEvent(true);

    // We don't need to call sanitizeUserInputValue() function here because
    // HTMLInputElement::handleBeforeTextInsertedEvent() has already called
    // sanitizeUserInputValue().
    // ---
    // sanitizeValue() is needed because IME input doesn't dispatch BeforeTextInsertedEvent.
    // ---
    // Input types that support the selection API do *not* sanitize their
    // user input in order to retain parity between what's in the model and
    // what's on the screen. Otherwise, we retain the sanitization process for
    // backward compatibility. https://bugs.webkit.org/show_bug.cgi?id=150346
    auto innerText = ConvertFromVisibleValue(element->InnerTextValue());
    if (!SupportsSelectionAPI())
    {
      innerText = SanitizeValue(innerText);
    }
    element->SetValueFromRenderer(innerText);
    element->UpdatePlaceholderVisibility();
    // Recalc for :invalid change.
    element->InvalidateStyleForSubtree();

    DidSetValueByUserEdit();
  }

  void TextFieldInputType::DidSetValueByUserEdit() noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (!element.Focused())
    {
      return;
    }
    if (element.NodeDocument().Frame())
    {
      element.NodeDocument().GetEditor().TextDidChangeInTextField(element);
    }
    // FIXME: Showing the datalist's suggestions, with datalists.
  }

  void TextFieldInputType::UpdateInnerTextValue() noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (!element->FormControlValueMatchesRenderer())
    {
      // Update the renderer value if the formControlValueMatchesRenderer() flag is false.
      // It protects an unacceptable renderer value from being overwritten with the DOM value.
      element->SetInnerTextValue(VisibleValue());
      element->UpdatePlaceholderVisibility();
    }
  }

  void TextFieldInputType::CreateContainer(PreserveSelectionRange preserveSelection) noexcept
  {
    krys_debug_assert(!_container);
    krys_debug_assert(Element());

    Ref<HTMLInputElement> element = ShareRef(*Element());
    auto &shadowRoot = *element->UserAgentShadowRoot();

    auto &document = element->NodeDocument();
    // FIXME: <https://webkit.org/b/245977> Suppress selectionchange events during subtree modification.
    struct SelectionState
    {
      uint32 Start;
      uint32 End;
      TextFieldSelectionDirection Direction;
    };
    Maybe<SelectionState> selectionState;
    if (preserveSelection == PreserveSelectionRange::Yes
        && EnclosingTextFormControl(document.Selection().Selection().Start()) == element.get())
    {
      selectionState = SelectionState {element->SelectionStart(), element->SelectionEnd(),
                                       element->ComputeSelectionDirection()};
    }

    auto container = TextControlInnerContainer::Create(document);
    _container = container;
    (void)shadowRoot.AppendChild(*container);
    container->SetUserAgentPart(css::UserAgentParts::WebkitTextfieldDecorationContainer());

    auto innerBlock = TextControlInnerElement::Create(document);
    _innerBlock = innerBlock;
    (void)_container->AppendChild(*innerBlock);
    (void)innerBlock->AppendChild(*_innerText);

    if (selectionState)
    {
      EventLoop::Current().QueueTask(
        TaskSource::DOMManipulation, element->NodeDocument(),
        [selectionState = *selectionState, element = WeakPtr<HTMLInputElement>::WithRef(*element)]
        {
          if (!element || !element->Focused())
          {
            return;
          }

          auto &selection = element->NodeDocument().Selection().Selection();
          if (selection.Start().DeprecatedNode() != element->UserAgentShadowRoot())
          {
            return;
          }

          element->SetSelectionRange(selectionState.Start, selectionState.End, selectionState.Direction);
        });
    }
  }
}
