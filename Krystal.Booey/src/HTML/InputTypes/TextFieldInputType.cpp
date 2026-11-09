/*
 * Copyright (C) 2010-2013 Google Inc. All rights reserved.
 * Copyright (C) 2011-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/InputTypes/TextFieldInputType.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Events/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Events/TextEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/HTML/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
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
    CloseSuggestions();
    if (auto picker = std::exchange(_suggestionPicker, nullptr))
    {
      picker->Detach();
    }
    if (auto indicator = _dataListDropdownIndicator)
    {
      indicator->RemoveOwner();
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
    auto &element = *Element();
    if (element.Focused() && element.List())
    {
      DisplaySuggestions(DataListSuggestionActivationType::ControlClicked);
    }
  }

  void TextFieldInputType::ShowPicker() noexcept
  {
    if (Element()->List())
    {
      DisplaySuggestions(DataListSuggestionActivationType::ControlClicked);
    }
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
    // The suggestion list's keys go to it.
    auto key = event.KeyIdentifier();
    if (auto picker = _suggestionPicker;
        picker && (key == u8"Enter" || key == u8"Up" || key == u8"Down" || key == u8"U+001B"))
    {
      picker->HandleKeydownWithIdentifier(key);
      event.SetDefaultHandled();
    }
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
    // The arrow keys move through the suggestion list while it's shown.
    if (_suggestionPicker)
    {
      return;
    }
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
    if (event.IsMouseEvent() || isFocusEvent || isBlurEvent)
    {
      Element()->ForwardEvent(event);
    }
  }

  void TextFieldInputType::ElementDidBlur() noexcept
  {
    krys_debug_assert(Element());
    CloseSuggestions();
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
    bool createsContainer = shouldHaveSpinButton || element->List() || NeedsContainer();

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

    DataListMayHaveChanged();
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
    if (auto indicator = _dataListDropdownIndicator)
    {
      indicator->RemoveOwner();
    }
    _dataListDropdownIndicator = nullptr;
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
    if (element.List())
    {
      DisplaySuggestions(DataListSuggestionActivationType::TextChanged);
    }
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
        [selectionState = *selectionState, element = CreateWeakPtr(element.get())]
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

  namespace
  {
    KRYS_NODISCARD bool DataListSuggestionStartsWith(dom::DOMStringView value,
                                                     dom::DOMStringView prefix) noexcept
    {
      return text::ToASCIILower(value).starts_with(text::ToASCIILower(prefix));
    }

    KRYS_NODISCARD bool DataListSuggestionContains(dom::DOMStringView value, dom::DOMStringView part) noexcept
    {
      return text::ToASCIILower(value).contains(text::ToASCIILower(part));
    }
  }

  void TextFieldInputType::CreateDataListDropdownIndicator() noexcept
  {
    krys_debug_assert(!_dataListDropdownIndicator);
    if (!_container)
    {
      CreateContainer();
    }
    auto indicator = html::DataListButtonElement::Create(Element()->NodeDocument(), *this);
    _dataListDropdownIndicator = indicator;
    (void)_container->AppendChild(*indicator);
  }

  void TextFieldInputType::DataListMayHaveChanged() noexcept
  {
    if (!HasCreatedShadowSubtree())
    {
      return;
    }

    _cachedSuggestions = null;

    RefPtr<HTMLInputElement> element = ShareRefPtr(Element());
    if (!element)
    {
      return;
    }
    bool hasDataList = !!element->List();
    if (!_dataListDropdownIndicator)
    {
      CreateDataListDropdownIndicator();
    }
    else
    {
      // It's displayed while the field has a datalist.
      _dataListDropdownIndicator->InvalidateStyle();
    }
    if (hasDataList && element->Focused())
    {
      DisplaySuggestions(DataListSuggestionActivationType::DataListMayHaveChanged);
    }
  }

  HTMLElement *TextFieldInputType::DataListButtonElement() const noexcept
  {
    return _dataListDropdownIndicator.get();
  }

  void TextFieldInputType::DataListButtonElementWasClicked() noexcept
  {
    Ref<HTMLInputElement> input = ShareRef(*Element());
    if (input->List())
    {
      // The caret goes to the end of the text, as the suggestions complete it.
      input->SetSelectionRange(std::numeric_limits<uint32>::max(), std::numeric_limits<uint32>::max());
      DisplaySuggestions(DataListSuggestionActivationType::IndicatorClicked);
    }
  }

  IntRect TextFieldInputType::ElementRectInRootViewCoordinates() const noexcept
  {
    auto &element = *Element();
    auto *renderer = element.Renderer();
    auto *view = element.NodeDocument().View();
    if (!renderer || !view)
    {
      return {};
    }
    return view->ContentsToView(renderer->AbsoluteBoundingBoxRect());
  }

  bool TextFieldInputType::IsElementRTL() const noexcept
  {
    auto *renderer = Element()->Renderer();
    return renderer && renderer->Style().GetWritingMode().ComputedTextDirection() == TextDirection::RTL;
  }

  List<DataListSuggestion> TextFieldInputType::Suggestions() noexcept
  {
    // FIXME: Suggestions are typing completions, so they should match as find-in-page does rather than by
    // ignoring ASCII case.
    Ref<HTMLInputElement> element = ShareRef(*Element());
    auto elementValue = element->Value();
    if (_cachedSuggestions
        && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(_cachedSuggestions->first, elementValue))
    {
      return _cachedSuggestions->second;
    }

    // Those starting with the text, then those containing it (or whose labels do, if they're shown).
    List<DataListSuggestion> suggestions;
    List<DataListSuggestion> matchesContainingValue;
    auto *page = element->NodeDocument().GetPage();
    bool canShowLabels = page && page->GetChromeClient().CanShowDataListSuggestionLabels();
    if (auto dataList = element->List())
    {
      for (auto &option : dataList->Suggestions())
      {
        DataListSuggestion suggestion;
        suggestion.Value = option->Value();
        if (!element->IsValidValue(suggestion.Value))
        {
          continue;
        }
        suggestion.Value = SanitizeValue(suggestion.Value);
        suggestion.Label = option->Label();
        if (suggestion.Value == suggestion.Label)
        {
          suggestion.Label.clear();
        }

        if (elementValue.empty() || DataListSuggestionStartsWith(suggestion.Value, elementValue))
        {
          suggestions.push_back(krys::move(suggestion));
        }
        else if (DataListSuggestionContains(suggestion.Value, elementValue)
                 || (canShowLabels && DataListSuggestionContains(suggestion.Label, elementValue)))
        {
          matchesContainingValue.push_back(krys::move(suggestion));
        }
      }
    }

    suggestions.insert(suggestions.end(), std::make_move_iterator(matchesContainingValue.begin()),
                       std::make_move_iterator(matchesContainingValue.end()));
    _cachedSuggestions = Pair<dom::DOMString, List<DataListSuggestion>> {elementValue, suggestions};
    return suggestions;
  }

  void TextFieldInputType::DidSelectDataListOption(utf8_stringview value) noexcept
  {
    Ref<HTMLInputElement> element = ShareRef(*Element());
    (void)element->SetValue(value, TextFieldEventBehavior::DispatchInputAndChangeEvent);
  }

  void TextFieldInputType::DidCloseSuggestions() noexcept
  {
    _cachedSuggestions = null;
    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (_suggestionPicker)
    {
      styleInvalidation.emplace(*Element(), css::SelectorPseudoClass::Open, false);
    }
    if (auto picker = std::exchange(_suggestionPicker, nullptr))
    {
      picker->Detach();
    }
    if (auto *renderer = Element()->Renderer())
    {
      renderer->Repaint();
    }
  }

  void TextFieldInputType::DisplaySuggestions(DataListSuggestionActivationType type) noexcept
  {
    auto &element = *Element();
    if (element.IsDisabledFormControl() || !element.Renderer())
    {
      return;
    }

    // A click shows them only for a user's click; changes to the text or the datalist always update them.
    if ((type == DataListSuggestionActivationType::ControlClicked
         || type == DataListSuggestionActivationType::IndicatorClicked)
        && !HasTransientActivation())
    {
      return;
    }

    auto *page = element.NodeDocument().GetPage();
    if (!page)
    {
      return;
    }
    if (!_suggestionPicker && !Suggestions().empty())
    {
      // The field matches :open while its suggestions are shown.
      style::PseudoClassChangeInvalidation styleInvalidation(element, css::SelectorPseudoClass::Open, true);
      _suggestionPicker = page->GetChromeClient().CreateDataListSuggestionPicker(*this);
    }
    if (auto picker = _suggestionPicker)
    {
      picker->DisplayWithActivationType(type);
    }
  }

  void TextFieldInputType::CloseSuggestions() noexcept
  {
    if (auto picker = _suggestionPicker)
    {
      picker->Close();
    }
  }

  bool TextFieldInputType::IsPresentingAttachedView() const noexcept
  {
    return !!_suggestionPicker;
  }
}
