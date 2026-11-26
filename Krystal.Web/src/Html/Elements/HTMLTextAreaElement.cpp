#include "Krystal.Web/Html/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Web/Accessibility/AXObjectCache.hpp"
#include "Krystal.Web/Dom/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Editing/Editor.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Editing/TextIterator.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Forms/Autofill.hpp"
#include "Krystal.Web/Html/Forms/DOMFormData.hpp"
#include "Krystal.Web/Html/Internals/BeforeTextInsertedEvent.hpp"
#include "Krystal.Web/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Web/Html/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderTextControlMultiLine.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+SettersInlines.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief The length of @p text as script measures it, with each CRLF counted once.
    KRYS_NODISCARD size_t ComputeLengthForAPIValue(DOMStringView text) noexcept
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

  HTMLTextAreaElement::HTMLTextAreaElement(Document &document) noexcept
      : HTMLTextFormControlElement(document, html::HTMLElementInterface::TextArea)
  {
    SetFormControlValueMatchesRenderer(true);
  }

  Ref<HTMLTextAreaElement> HTMLTextAreaElement::Create(Document &document) noexcept
  {
    auto textArea = CreateRef<HTMLTextAreaElement>(document);
    textArea->EnsureUserAgentShadowRoot();
    return textArea;
  }

  void HTMLTextAreaElement::DidAddUserAgentShadowRoot(web::ShadowRoot &root) noexcept
  {
    (void)root.AppendChild(
      *html::TextControlInnerTextElement::Create(NodeDocument(), IsInnerTextElementEditable()));
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#the-textarea-element:children-changed-steps
  void HTMLTextAreaElement::OnChildrenChanged() noexcept
  {
    // If the element's dirty value flag is false, set the element's raw value to its child text content.
    HTMLTextFormControlElement::OnChildrenChanged();
    SetLastChangeWasNotUserEdit();
    if (_isDirty)
    {
      SetInnerTextValue(Value());
    }
    else
    {
      SetNonDirtyValue(DefaultValue(), html::TextControlSetValueSelection::Clamp);
    }
  }

  RenderPtr<RenderElement> HTMLTextAreaElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderTextControlMultiLine>(*this, krys::move(style));
  }

  bool HTMLTextAreaElement::AccessKeyAction(bool sendMouseEvents) noexcept
  {
    // NOLINTNEXTLINE(bugprone-parent-virtual-call): as WebKit.
    return web::Element::AccessKeyAction(sendMouseEvents);
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
      // Without a kept selection to restore, the caret goes to the start.
      SetSelectionRange(0, 0, html::TextFieldSelectionDirection::SelectionHasNoDirection, revealMode);
    }
  }

  void HTMLTextAreaElement::DefaultEventHandler(Event &event) noexcept
  {
    if (Renderer())
    {
      if (event.IsMouseEvent() || event.Type() == u8"blur")
      {
        ForwardEvent(event);
      }
      else if (auto *insertedEvent = DynamicDowncast<BeforeTextInsertedEvent>(event))
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
    // Typing changes the value without changing the text area's children, so its direction is checked here.
    if (SelfOrPrecedingNodesAffectDirAuto())
    {
      UpdateEffectiveTextDirection();
    }
  }

  void HTMLTextAreaElement::HandleBeforeTextInsertedEvent(BeforeTextInsertedEvent &event) const noexcept
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

    // The insertion replaces the selection, if the text area is focused; otherwise the selection is the
    // source of a drag and nothing is replaced.
    auto selectionRange = Focused() ? NodeDocument().Selection().Selection().ToNormalizedRange() : null;
    size_t selectionLength = selectionRange ? ComputeLengthForAPIValue(PlainText(*selectionRange)) : 0;
    krys_debug_assert(currentLength >= selectionLength);
    size_t baseLength = currentLength - selectionLength;
    size_t appendableLength = unsignedMaxLength > baseLength ? unsignedMaxLength - baseLength : 0;
    event.SetText(SanitizeUserInputValue(event.Text(), appendableLength));
  }

  DOMString HTMLTextAreaElement::SanitizeUserInputValue(DOMStringView proposedValue,
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
    return DOMString(proposedValue.substr(0, std::min(i, proposedValue.size())));
  }

  RefPtr<html::TextControlInnerTextElement> HTMLTextAreaElement::InnerTextElement() const noexcept
  {
    auto *root = UserAgentShadowRoot();
    return root ? ShareRefPtr(Downcast<html::TextControlInnerTextElement>(root->FirstChild())) : nullptr;
  }

  RefPtr<html::TextControlInnerTextElement>
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
    auto placeholderText = GetAttribute(u8"placeholder").value_or(DOMString {});
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
      _placeholder = html::TextControlPlaceholderElement::Create(NodeDocument());
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
                                                         DOMStringView direction) noexcept
  {
    SetSelectionRange(start, end, direction, html::ForBindings::Yes);
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#the-textarea-element:concept-node-clone-ext
  void HTMLTextAreaElement::OnClone(Node &copy, bool subtree) noexcept
  {
    // Propagate the raw value and dirty value flag from node to copy.
    HTMLTextFormControlElement::OnClone(copy, subtree);
    auto &clone = Downcast<HTMLTextAreaElement>(copy);
    clone.SetValueCommon(Value(), html::TextFieldEventBehavior::DispatchNoEvent,
                         html::TextControlSetValueSelection::DoNotSet);
    clone._isDirty = _isDirty;
    clone.UpdateValidity();
  }

  void HTMLTextAreaElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                               Maybe<DOMStringView> value,
                                               const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLTextFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    if (localName == u8"wrap")
    {
      // https://html.spec.whatwg.org/multipage/form-elements.html#attr-textarea-wrap
      // Soft or Hard; off (no wrapping when rendered) is the rendering section's.
      auto newValue = value.value_or(DOMStringView());
      WrapMethod wrap;
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"hard"))
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

  bool HTMLTextAreaElement::AppendFormData(html::DOMFormData &formData) noexcept
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

  // https://html.spec.whatwg.org/multipage/form-elements.html#the-textarea-element:concept-form-reset-control
  void HTMLTextAreaElement::Reset() noexcept
  {
    // Set the user validity to false, the dirty value flag back to false, and the raw value to its child text
    // content.
    SetInteractedWithSinceLastFormSubmitEvent(false);
    SetNonDirtyValue(DefaultValue(), html::TextControlSetValueSelection::SetSelectionToEnd);
  }

  DOMString HTMLTextAreaElement::Value() const noexcept
  {
    UpdateValue();
    return _value;
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-value
  ExceptionOr<void> HTMLTextAreaElement::SetValue(DOMStringView value,
                                                  html::TextFieldEventBehavior eventBehavior,
                                                  html::TextControlSetValueSelection selection) noexcept
  {
    // 1. Let oldAPIValue be this element's API value.
    // 2. Set this element's raw value to the new value.
    // 4. If the new API value is different from oldAPIValue, then move the text entry cursor position to the
    //    end of the text control, unselecting any selected text and resetting the selection direction to
    //    "none".
    // (SetValueCommon.)
    SetValueCommon(value, eventBehavior, selection);

    // 3. Set this element's dirty value flag to true.
    _isDirty = true;
    UpdateValidity();
    return {};
  }

  void HTMLTextAreaElement::SetNonDirtyValue(DOMStringView value,
                                             html::TextControlSetValueSelection selection) noexcept
  {
    SetValueCommon(value, html::TextFieldEventBehavior::DispatchNoEvent, selection);
    _isDirty = false;
    UpdateValidity();
  }

  void HTMLTextAreaElement::SetValueCommon(DOMStringView newValue, html::TextFieldEventBehavior,
                                           html::TextControlSetValueSelection selection) noexcept
  {
    _wasModifiedByUser = false;
    // The API's value gets its line breaks normalized; the user's edits are normalized as they're made.
    auto normalizedValue = infra::StringAlgorithms::NormalizeNewlines(newValue);

    // An unchanged value leaves the caret where it is.
    if (normalizedValue == Value())
    {
      return;
    }

    bool shouldClamp = selection == html::TextControlSetValueSelection::Clamp;
    auto selectionStartValue = shouldClamp ? ComputeSelectionStart() : 0;
    auto selectionEndValue = shouldClamp ? ComputeSelectionEnd() : 0;

    _value = normalizedValue;
    SetInnerTextValue(_value);
    SetLastChangeWasNotUserEdit();
    UpdatePlaceholderVisibility();
    InvalidateStyleForSubtree();
    if (SelfOrPrecedingNodesAffectDirAuto())
    {
      UpdateEffectiveTextDirection();
    }
    SetFormControlValueMatchesRenderer(true);

    auto endOfString = static_cast<uint32>(_value.size());
    if (selection == html::TextControlSetValueSelection::SetSelectionToEnd)
    {
      if (NodeDocument().FocusedElement() == this)
      {
        SetSelectionRange(endOfString, endOfString);
      }
      else
      {
        // Unfocused, the kept selection moves to the end.
        CacheSelection(endOfString, endOfString, html::TextFieldSelectionDirection::SelectionHasNoDirection);
      }
    }
    else if (shouldClamp)
    {
      CacheSelection(std::min(endOfString, selectionStartValue), std::min(endOfString, selectionEndValue),
                     html::TextFieldSelectionDirection::SelectionHasNoDirection);
    }

    SetTextAsOfLastFormControlChangeEvent(krys::move(normalizedValue));

    if (auto *cache = NodeDocument().ExistingAXObjectCache())
    {
      cache->ValueChanged(*this);
    }
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-defaultvalue
  DOMString HTMLTextAreaElement::DefaultValue() const noexcept
  {
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  void HTMLTextAreaElement::DefaultValue(DOMString defaultValue) noexcept
  {
    (void)TextContent(krys::move(defaultValue));
  }

  // https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-textlength
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

  bool HTMLTextAreaElement::ValueMissing(Maybe<DOMStringView> value) const noexcept
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

  bool HTMLTextAreaElement::IsValidValue(DOMStringView candidate) const noexcept
  {
    return !ValueMissing(candidate) && !TooShort(candidate, IgnoreDirtyFlag)
           && !TooLong(candidate, IgnoreDirtyFlag);
  }

  bool HTMLTextAreaElement::TooShort(Maybe<DOMStringView> value, NeedsToCheckDirtyFlag check) const noexcept
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

    // An empty value is never too short.
    size_t length = !value ? LengthForAPIValue(Value()) : ComputeLengthForAPIValue(*value);
    return length > 0 && length < static_cast<size_t>(min);
  }

  bool HTMLTextAreaElement::TooLong(Maybe<DOMStringView> value, NeedsToCheckDirtyFlag check) const noexcept
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

  // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-fe-autocomplete
  DOMString HTMLTextAreaElement::Autocomplete() const noexcept
  {
    // Return the element's IDL-exposed autofill value.
    return html::AutofillData::Create(*this).IDLExposedValue;
  }

  void HTMLTextAreaElement::Autocomplete(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Cols() const noexcept
  {
    return html::ReflectPositiveWithFallback<uint32, html::ReflectDefault<uint32> {DefaultCols}>(*this,
                                                                                                 u8"cols");
  }

  void HTMLTextAreaElement::Cols(uint32 value) noexcept
  {
    html::ReflectPositiveWithFallback<uint32, html::ReflectDefault<uint32> {DefaultCols}>(*this, u8"cols",
                                                                                          value);
  }

  DOMString HTMLTextAreaElement::DirName() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLTextAreaElement::DirName(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"dirname", krys::move(value));
  }

  bool HTMLTextAreaElement::Disabled() const noexcept
  {
    return html::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLTextAreaElement::Disabled(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  void HTMLTextAreaElement::MaxLength(int32 value) noexcept
  {
    html::Reflect<int32>(*this, u8"maxlength", krys::move(value));
  }

  void HTMLTextAreaElement::MinLength(int32 value) noexcept
  {
    html::Reflect<int32>(*this, u8"minlength", krys::move(value));
  }

  DOMString HTMLTextAreaElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLTextAreaElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  DOMString HTMLTextAreaElement::Placeholder() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLTextAreaElement::Placeholder(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"placeholder", krys::move(value));
  }

  bool HTMLTextAreaElement::ReadOnly() const noexcept
  {
    return html::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLTextAreaElement::ReadOnly(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"readonly", krys::move(value));
  }

  bool HTMLTextAreaElement::Required() const noexcept
  {
    return html::Reflect<bool>(*this, u8"required");
  }

  void HTMLTextAreaElement::Required(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Rows() const noexcept
  {
    return html::ReflectPositiveWithFallback<uint32, html::ReflectDefault<uint32> {DefaultRows}>(*this,
                                                                                                 u8"rows");
  }

  void HTMLTextAreaElement::Rows(uint32 value) noexcept
  {
    html::ReflectPositiveWithFallback<uint32, html::ReflectDefault<uint32> {DefaultRows}>(*this, u8"rows",
                                                                                          value);
  }

  DOMString HTMLTextAreaElement::Wrap() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"wrap").value_or(u8"");
  }

  void HTMLTextAreaElement::Wrap(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"wrap", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool
    HTMLTextAreaElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null)
    {
      if (localName == u8"align")
      {
        // A text area's align has no presentational hint.
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
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
