#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlMultiLine.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @brief The length of @p text as script measures it, with each CRLF counted once.
    KRYS_NODISCARD size_t ComputeLengthForAPIValue(dom::DOMStringView text) noexcept
    {
      size_t crlfCount = 0;
      for (size_t i = 0; i + 1 < text.size(); ++i)
      {
        if (text[i] == u8'\r' && text[i + 1] == u8'\n')
        {
          ++crlfCount;
        }
      }
      return HTMLTextAreaElement::LengthForAPIValue(text) - crlfCount;
    }
  }

  HTMLTextAreaElement::HTMLTextAreaElement(dom::Document &document) noexcept
      : HTMLTextFormControlElement(document, HTMLElementInterface::TextArea)
  {
    SetFormControlValueMatchesRenderer(true);
  }

  Ref<HTMLTextAreaElement> HTMLTextAreaElement::Create(dom::Document &document) noexcept
  {
    auto textArea = CreateRef<HTMLTextAreaElement>(document);
    textArea->EnsureUserAgentShadowRoot();
    return textArea;
  }

  void HTMLTextAreaElement::DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept
  {
    (void)root.AppendChild(
      *TextControlInnerTextElement::Create(NodeDocument(), IsInnerTextElementEditable()));
  }

  void HTMLTextAreaElement::OnChildrenChanged() noexcept
  {
    HTMLTextFormControlElement::OnChildrenChanged();
    SetLastChangeWasNotUserEdit();
    if (_isDirty)
    {
      SetInnerTextValue(Value());
    }
    else
    {
      SetNonDirtyValue(DefaultValue(), TextControlSetValueSelection::Clamp);
    }
  }

  RenderPtr<RenderElement> HTMLTextAreaElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderTextControlMultiLine>(*this, krys::move(style));
  }

  void HTMLTextAreaElement::UpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                                                  SelectionRevealMode revealMode) noexcept
  {
    if (restorationMode == dom::SelectionRestorationMode::RestoreOrSelectAll && HasCachedSelection())
    {
      RestoreCachedSelection(revealMode);
    }
    else
    {
      // If this is the first focus, set a caret at the beginning of the text.
      // This matches some browsers' behavior; see bug 11746 Comment #15.
      // http://bugs.webkit.org/show_bug.cgi?id=11746#c15
      SetSelectionRange(0, 0, TextFieldSelectionDirection::SelectionHasNoDirection, revealMode);
    }
  }

  void HTMLTextAreaElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (Renderer())
    {
      if (event.IsMouseEvent() || event.Type() == u8"blur")
      {
        ForwardEvent(event);
      }
      else if (auto *insertedEvent = DynamicDowncast<dom::BeforeTextInsertedEvent>(event))
      {
        HandleBeforeTextInsertedEvent(*insertedEvent);
      }
    }

    HTMLTextFormControlElement::DefaultEventHandler(event);
  }

  void HTMLTextAreaElement::SubtreeHasChanged() noexcept
  {
    SetFormControlValueMatchesRenderer(false);
    UpdateValidity();
    UpdatePlaceholderVisibility();

    if (!Focused())
    {
      return;
    }

    SetChangedSinceLastFormControlChangeEvent(true);

    if (NodeDocument().Frame())
    {
      NodeDocument().GetEditor().TextDidChangeInTextArea(*this);
    }
    // FIXME: Auto directionality, which typing doesn't update through childrenChanged
    // (updateEffectiveTextDirection), with dir=auto.
  }

  void HTMLTextAreaElement::HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &event) const noexcept
  {
    krys_debug_assert(Renderer());
    int32 signedMaxLength = MaxLength();
    if (signedMaxLength < 0)
    {
      return;
    }
    auto unsignedMaxLength = static_cast<size_t>(signedMaxLength);

    auto currentValue = InnerTextValue();
    size_t currentLength = ComputeLengthForAPIValue(currentValue);
    if (currentLength + ComputeLengthForAPIValue(event.Text()) < unsignedMaxLength)
    {
      return;
    }

    // selectionLength represents the selection length of this text field to be
    // removed by this insertion.
    // If the text field has no focus, we don't need to take account of the
    // selection length. The selection is the source of text drag-and-drop in
    // that case, and nothing in the text field will be removed.
    auto selectionRange = Focused() ? NodeDocument().Selection().Selection().ToNormalizedRange() : null;
    size_t selectionLength = selectionRange ? ComputeLengthForAPIValue(PlainText(*selectionRange)) : 0;
    krys_debug_assert(currentLength >= selectionLength);
    size_t baseLength = currentLength - selectionLength;
    size_t appendableLength = unsignedMaxLength > baseLength ? unsignedMaxLength - baseLength : 0;
    event.SetText(SanitizeUserInputValue(event.Text(), appendableLength));
  }

  dom::DOMString HTMLTextAreaElement::SanitizeUserInputValue(dom::DOMStringView proposedValue,
                                                             size_t maxLength) noexcept
  {
    size_t submissionLength = 0;
    size_t i = 0;
    while (i < proposedValue.size())
    {
      if (proposedValue[i] == u8'\r' && i + 1 < proposedValue.size() && proposedValue[i + 1] == u8'\n')
      {
        ++i;
        continue;
      }
      // A character outside the Basic Multilingual Plane counts twice, as its two UTF-16 code units.
      auto lead = static_cast<uint8>(proposedValue[i]);
      size_t characterLength = lead < 0x80 ? 1 : lead < 0xE0 ? 2 : lead < 0xF0 ? 3 : 4;
      size_t units = characterLength == 4 ? 2 : 1;
      if (submissionLength + units > maxLength)
      {
        break;
      }
      submissionLength += units;
      i += characterLength;
      if (submissionLength == maxLength)
      {
        break;
      }
    }
    return dom::DOMString(proposedValue.substr(0, std::min(i, proposedValue.size())));
  }

  RefPtr<TextControlInnerTextElement> HTMLTextAreaElement::InnerTextElement() const noexcept
  {
    auto *root = UserAgentShadowRoot();
    return root ? ShareRefPtr(Downcast<TextControlInnerTextElement>(root->FirstChild())) : nullptr;
  }

  RefPtr<TextControlInnerTextElement>
    HTMLTextAreaElement::InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept
  {
    return InnerTextElement();
  }

  void HTMLTextAreaElement::UpdateValue() const noexcept
  {
    if (FormControlValueMatchesRenderer())
    {
      return;
    }

    krys_debug_assert(Renderer());
    auto &self = const_cast<HTMLTextAreaElement &>(*this);
    _value = InnerTextValue();
    self.SetFormControlValueMatchesRenderer(true);
    _isDirty = true;
    _wasModifiedByUser = true;
    self.UpdatePlaceholderVisibility();
  }

  void HTMLTextAreaElement::UpdatePlaceholderText() noexcept
  {
    auto placeholderText = GetAttribute(u8"placeholder").value_or(dom::DOMString {});
    if (placeholderText.empty())
    {
      if (_placeholder)
      {
        (void)UserAgentShadowRoot()->RemoveChild(*_placeholder);
        _placeholder = nullptr;
      }
      return;
    }
    if (!_placeholder)
    {
      _placeholder = TextControlPlaceholderElement::Create(NodeDocument());
      (void)UserAgentShadowRoot()->InsertBefore(*_placeholder, InnerTextElement()->NextSibling());
    }
    _placeholder->InnerText(krys::move(placeholderText));
  }

  RenderStyle HTMLTextAreaElement::CreateInnerTextStyle(const RenderStyle &style) noexcept
  {
    auto textBlockStyle = RenderStyle::Create();
    textBlockStyle.InheritFrom(style);
    AdjustInnerTextStyle(style, textBlockStyle);
    textBlockStyle.SetDisplay(DisplayType::Block);
    return textBlockStyle;
  }

  void HTMLTextAreaElement::SetSelectionRangeForBindings(uint32 start, uint32 end,
                                                         dom::DOMStringView direction) noexcept
  {
    SetSelectionRange(start, end, direction, ForBindings::Yes);
  }

  void HTMLTextAreaElement::OnClone(dom::Node &copy, bool subtree) noexcept
  {
    HTMLTextFormControlElement::OnClone(copy, subtree);
    auto &clone = Downcast<HTMLTextAreaElement>(copy);
    clone.SetValueCommon(Value(), TextFieldEventBehavior::DispatchNoEvent,
                         TextControlSetValueSelection::DoNotSet);
    clone._isDirty = _isDirty;
    clone.UpdateValidity();
  }

  void HTMLTextAreaElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value,
                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLTextFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"wrap")
    {
      // The virtual/physical values were a Netscape extension of HTML 3.0, now deprecated.
      // The soft/hard /off values are a recommendation for HTML 4 extension by IE and NS 4.
      auto newValue = value.value_or(dom::DOMStringView());
      WrapMethod wrap;
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"physical")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"hard")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"on"))
      {
        wrap = HardWrap;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"off"))
      {
        wrap = NoWrap;
      }
      else
      {
        wrap = SoftWrap;
      }
      _wrap = wrap;
    }
    else if (localName == u8"rows" || localName == u8"cols")
    {
      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
      }
    }
    else if (localName == u8"maxlength")
    {
      InternalSetMaxLength(ParseLengthAttribute(value));
      UpdateValidity();
    }
    else if (localName == u8"minlength")
    {
      InternalSetMinLength(ParseLengthAttribute(value));
      UpdateValidity();
    }
  }

  bool HTMLTextAreaElement::AppendFormData(DOMFormData &formData) noexcept
  {
    auto name = Name();
    if (name.empty())
    {
      return false;
    }

    NodeDocument().UpdateLayout();

    formData.Append(name, _wrap == HardWrap ? ValueWithHardLineBreaks() : Value());
    if (auto dirname = GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, DirectionForFormData());
    }
    return true;
  }

  void HTMLTextAreaElement::Reset() noexcept
  {
    SetInteractedWithSinceLastFormSubmitEvent(false);
    SetNonDirtyValue(DefaultValue(), TextControlSetValueSelection::SetSelectionToEnd);
  }

  dom::DOMString HTMLTextAreaElement::Value() const noexcept
  {
    UpdateValue();
    return _value;
  }

  dom::ExceptionOr<void> HTMLTextAreaElement::SetValue(dom::DOMStringView value,
                                                       TextFieldEventBehavior eventBehavior,
                                                       TextControlSetValueSelection selection) noexcept
  {
    SetValueCommon(value, eventBehavior, selection);
    _isDirty = true;
    UpdateValidity();
    return {};
  }

  void HTMLTextAreaElement::SetNonDirtyValue(dom::DOMStringView value,
                                             TextControlSetValueSelection selection) noexcept
  {
    SetValueCommon(value, TextFieldEventBehavior::DispatchNoEvent, selection);
    _isDirty = false;
    UpdateValidity();
  }

  void HTMLTextAreaElement::SetValueCommon(dom::DOMStringView newValue, TextFieldEventBehavior,
                                           TextControlSetValueSelection selection) noexcept
  {
    _wasModifiedByUser = false;
    // Code elsewhere normalizes line endings added by the user via the keyboard or pasting.
    // We normalize line endings coming from JavaScript here.
    auto normalizedValue = infra::StringAlgorithms::NormalizeNewlines(newValue);

    // Return early because we don't want to move the caret or trigger other side effects
    // when the value isn't changing. This matches Firefox behavior, at least.
    if (normalizedValue == Value())
    {
      return;
    }

    bool shouldClamp = selection == TextControlSetValueSelection::Clamp;
    auto selectionStartValue = shouldClamp ? ComputeSelectionStart() : 0;
    auto selectionEndValue = shouldClamp ? ComputeSelectionEnd() : 0;

    _value = normalizedValue;
    SetInnerTextValue(dom::DOMString(_value));
    SetLastChangeWasNotUserEdit();
    UpdatePlaceholderVisibility();
    InvalidateStyleForSubtree();
    // FIXME: Auto directionality (updateEffectiveTextDirection), with dir=auto.
    SetFormControlValueMatchesRenderer(true);

    auto endOfString = static_cast<uint32>(_value.size());
    if (selection == TextControlSetValueSelection::SetSelectionToEnd)
    {
      if (NodeDocument().FocusedElement() == this)
      {
        SetSelectionRange(endOfString, endOfString);
      }
      else
      {
        // We don't change text selection here but need to update caret to
        // the end of the text value except for initialize.
        CacheSelection(endOfString, endOfString, TextFieldSelectionDirection::SelectionHasNoDirection);
      }
    }
    else if (shouldClamp)
    {
      CacheSelection(std::min(endOfString, selectionStartValue), std::min(endOfString, selectionEndValue),
                     TextFieldSelectionDirection::SelectionHasNoDirection);
    }

    SetTextAsOfLastFormControlChangeEvent(krys::move(normalizedValue));

    // FIXME: Telling accessibility the value changed, and the chrome client's
    // didProgrammaticallyClearTextFormControl, with accessibility and the host's autofill.
  }

  FormControlState HTMLTextAreaElement::SaveFormControlState() const noexcept
  {
    return _isDirty ? FormControlState {Value()} : FormControlState {};
  }

  void HTMLTextAreaElement::RestoreFormControlState(const FormControlState &state) noexcept
  {
    (void)SetValue(state[0]);
  }

  dom::DOMString HTMLTextAreaElement::DefaultValue() const noexcept
  {
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  void HTMLTextAreaElement::DefaultValue(dom::DOMString &&defaultValue) noexcept
  {
    (void)TextContent(krys::move(defaultValue));
  }

  uint32 HTMLTextAreaElement::TextLength() const noexcept
  {
    return static_cast<uint32>(LengthForAPIValue(Value()));
  }

  bool HTMLTextAreaElement::ValueMissing() const noexcept
  {
    return ValueMissing(std::nullopt);
  }

  bool HTMLTextAreaElement::TooShort() const noexcept
  {
    return TooShort(std::nullopt, CheckDirtyFlag);
  }

  bool HTMLTextAreaElement::TooLong() const noexcept
  {
    return TooLong(std::nullopt, CheckDirtyFlag);
  }

  bool HTMLTextAreaElement::ValueMissing(Maybe<dom::DOMStringView> value) const noexcept
  {
    if (!(IsRequired() && IsMutable()))
    {
      return false;
    }
    if (!value)
    {
      return Value().empty();
    }
    return value->empty();
  }

  bool HTMLTextAreaElement::IsValidValue(dom::DOMStringView candidate) const noexcept
  {
    return !ValueMissing(candidate) && !TooShort(candidate, IgnoreDirtyFlag)
           && !TooLong(candidate, IgnoreDirtyFlag);
  }

  bool HTMLTextAreaElement::TooShort(Maybe<dom::DOMStringView> value,
                                     NeedsToCheckDirtyFlag check) const noexcept
  {
    // Return false for the default value or value set by script even if it is shorter than minLength.
    if (check == CheckDirtyFlag && !_wasModifiedByUser)
    {
      return false;
    }

    int32 min = MinLength();
    if (min <= 0)
    {
      return false;
    }

    // The empty string is excluded from tooShort validation.
    size_t length = !value ? LengthForAPIValue(Value()) : ComputeLengthForAPIValue(*value);
    return length > 0 && length < static_cast<size_t>(min);
  }

  bool HTMLTextAreaElement::TooLong(Maybe<dom::DOMStringView> value,
                                    NeedsToCheckDirtyFlag check) const noexcept
  {
    // Return false for the default value or value set by script even if it is longer than maxLength.
    if (check == CheckDirtyFlag && !_wasModifiedByUser)
    {
      return false;
    }

    int32 max = MaxLength();
    if (max < 0)
    {
      return false;
    }

    size_t length = !value ? LengthForAPIValue(Value()) : ComputeLengthForAPIValue(*value);
    return length > static_cast<size_t>(max);
  }

#pragma region HTMLTextAreaElement

  dom::DOMString HTMLTextAreaElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLTextAreaElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Cols() const noexcept
  {
    return Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32> {DefaultCols}>(*this,
                                                                                                 u8"cols");
  }

  void HTMLTextAreaElement::Cols(uint32 value) noexcept
  {
    (void)Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32> {DefaultCols}>(
      *this, u8"cols", value);
  }

  dom::DOMString HTMLTextAreaElement::DirName() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLTextAreaElement::DirName(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dirname", krys::move(value));
  }

  bool HTMLTextAreaElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLTextAreaElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  void HTMLTextAreaElement::MaxLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"maxlength", krys::move(value));
  }

  void HTMLTextAreaElement::MinLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"minlength", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLTextAreaElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Placeholder() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLTextAreaElement::Placeholder(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"placeholder", krys::move(value));
  }

  bool HTMLTextAreaElement::ReadOnly() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLTextAreaElement::ReadOnly(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"readonly", krys::move(value));
  }

  bool HTMLTextAreaElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLTextAreaElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Rows() const noexcept
  {
    return Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32> {DefaultRows}>(*this,
                                                                                                 u8"rows");
  }

  void HTMLTextAreaElement::Rows(uint32 value) noexcept
  {
    (void)Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32> {DefaultRows}>(
      *this, u8"rows", value);
  }

  dom::DOMString HTMLTextAreaElement::Wrap() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"wrap").value_or(u8"");
  }

  void HTMLTextAreaElement::Wrap(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"wrap", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLTextAreaElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      if (localName == u8"align")
      {
        // Don't map 'align' attribute.  This matches what Firefox, Opera and IE do.
        return false;
      }
      if (localName == u8"wrap")
      {
        return true;
      }
    }
    return HTMLTextFormControlElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTextAreaElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"wrap")
    {
      if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"off"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                             css::ValueId::Preserve);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextWrapMode, css::ValueId::Wrap);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::BreakWord);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                             css::ValueId::Preserve);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextWrapMode, css::ValueId::Nowrap);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::Normal);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
