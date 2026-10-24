#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/CaretRectComputation.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD dom::Position PositionForIndex(TextControlInnerTextElement *innerText,
                                                  uint32 index) noexcept
    {
      uint32 remainingCharactersToMoveForward = index;
      dom::Node *lastBrOrText = innerText;
      for (dom::Node *node = innerText; node; node = dom::TreeQueries::Next(*node, innerText))
      {
        if (IsHTMLElementOfType(node, TagName::br))
        {
          if (!remainingCharactersToMoveForward)
          {
            return dom::PositionBeforeNode(node);
          }
          remainingCharactersToMoveForward--;
          lastBrOrText = node;
        }
        else if (auto *text = DynamicDowncast<dom::Text>(*node))
        {
          if (remainingCharactersToMoveForward < text->Length())
          {
            return dom::Position(ShareRefPtr(text), remainingCharactersToMoveForward);
          }
          remainingCharactersToMoveForward -= static_cast<uint32>(text->Length());
          lastBrOrText = node;
        }
      }
      return LastPositionInOrAfterNode(lastBrOrText);
    }

    /// @brief @p offset moved back to the start of the UTF-8 character it's in.
    KRYS_NODISCARD uint32 SnapToCharacterStart(dom::DOMStringView text, uint32 offset) noexcept
    {
      while (offset > 0 && offset < text.size() && (static_cast<uint8>(text[offset]) & 0xC0) == 0x80)
      {
        --offset;
      }
      return offset;
    }

    KRYS_NODISCARD dom::DOMStringView DirectionString(TextFieldSelectionDirection direction) noexcept
    {
      switch (direction)
      {
        case TextFieldSelectionDirection::SelectionHasNoDirection:       return u8"none";
        case TextFieldSelectionDirection::SelectionHasForwardDirection:  return u8"forward";
        case TextFieldSelectionDirection::SelectionHasBackwardDirection: return u8"backward";
      }

      krys_unreachable();
    }

    void StripTrailingNewline(dom::DOMString &result) noexcept
    {
      // Remove one trailing newline; there's always one that's collapsed out by rendering.
      if (!result.empty() && result.back() == u8'\n')
      {
        result.pop_back();
      }
    }

    KRYS_NODISCARD dom::DOMString InnerTextValueFrom(TextControlInnerTextElement &innerText) noexcept
    {
      dom::DOMString result;
      for (dom::Node *node = innerText.FirstChild(); node; node = dom::TreeQueries::Next(*node, &innerText))
      {
        if (IsHTMLElementOfType(node, TagName::br))
        {
          result.push_back(u8'\n');
        }
        else if (auto *textNode = DynamicDowncast<dom::Text>(*node))
        {
          result.append(textNode->Data());
        }
      }
      StripTrailingNewline(result);
      return result;
    }

    void SetContainerAndOffsetForRange(dom::Node &node, uint32 offset, RefPtr<dom::Node> &containerNode,
                                       uint32 &offsetInContainer) noexcept
    {
      if (Is<dom::Text>(node))
      {
        containerNode = ShareRefPtr(&node);
        offsetInContainer = offset;
      }
      else
      {
        containerNode = ShareRefPtr(node.ParentNode());
        offsetInContainer = static_cast<uint32>(dom::TreeQueries::Index(node)) + offset;
      }
    }
  }

  HTMLTextFormControlElement::HTMLTextFormControlElement(dom::Document &document,
                                                         HTMLElementInterface interface) noexcept
      : HTMLFormControlElement(document, interface)
  {
    if (document.Frame() && document.GetEditor().Behavior().ShouldConsiderSelectionAsDirectional())
    {
      _cachedSelectionDirection = TextFieldSelectionDirection::SelectionHasForwardDirection;
    }
  }

  HTMLTextFormControlElement::~HTMLTextFormControlElement() noexcept = default;

  bool HTMLTextFormControlElement::ChildShouldCreateRenderer(const dom::Node &child) const noexcept
  {
    // FIXME: We shouldn't force the pseudo elements down into the shadow, but
    // this perserves the current behavior of WebKit.
    if (Is<dom::PseudoElement>(child))
    {
      return HTMLFormControlElement::ChildShouldCreateRenderer(child);
    }
    return Is<dom::ShadowRoot>(child.ParentNode())
           && HTMLFormControlElement::ChildShouldCreateRenderer(child);
  }

  void HTMLTextFormControlElement::OnInsert() noexcept
  {
    HTMLFormControlElement::OnInsert();
    if (IsConnected())
    {
      SetTextAsOfLastFormControlChangeEvent(Value());
    }
  }

  void HTMLTextFormControlElement::DispatchFocusEvent(dom::EventTarget *relatedTarget,
                                                      const dom::FocusOptions &options) noexcept
  {
    if (SupportsPlaceholder())
    {
      UpdatePlaceholderVisibility();
    }
    HandleFocusEvent(DynamicDowncast<dom::Node>(relatedTarget));
    HTMLFormControlElement::DispatchFocusEvent(relatedTarget, options);
  }

  void HTMLTextFormControlElement::DispatchBlurEvent(dom::EventTarget *relatedTarget) noexcept
  {
    if (SupportsPlaceholder())
    {
      UpdatePlaceholderVisibility();
    }
    // Match the order in Document::setFocusedElement.
    HandleBlurEvent();
    HTMLFormControlElement::DispatchBlurEvent(relatedTarget);
  }

  void HTMLTextFormControlElement::DidEditInnerTextValue(bool wasUserEdit) noexcept
  {
    if (!Renderer() || !IsTextField())
    {
      return;
    }

    _lastChangeWasUserEdit = wasUserEdit;
    _wasEverChangedByUserEdit |= wasUserEdit;
    SubtreeHasChanged();
  }

  void HTMLTextFormControlElement::ForwardEvent(dom::Event &event) noexcept
  {
    if (event.Type() == EventNames::Blur || event.Type() == EventNames::Focus)
    {
      return;
    }

    if (auto innerText = InnerTextElement())
    {
      innerText->DefaultEventHandler(event);
    }
  }

  bool HTMLTextFormControlElement::IsPlaceholderEmpty() const noexcept
  {
    auto attributeValue = GetAttribute(u8"placeholder").value_or(dom::DOMString());
    return std::ranges::all_of(attributeValue, [](char8 c) { return c == u8'\n' || c == u8'\r'; });
  }

  bool HTMLTextFormControlElement::PlaceholderShouldBeVisible() const noexcept
  {
    // This function is used by the style resolver to match the :placeholder-shown pseudo class.
    // Since it is used for styling, it must not use any value depending on the style.
    return SupportsPlaceholder() && IsEmptyValue() && !IsPlaceholderEmpty() && _canShowPlaceholder;
  }

  void HTMLTextFormControlElement::UpdatePlaceholderVisibility() noexcept
  {
    bool newIsPlaceholderVisible = PlaceholderShouldBeVisible();
    if (_isPlaceholderVisible == newIsPlaceholderVisible)
    {
      return;
    }

    style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::PlaceholderShown,
                                                           newIsPlaceholderVisible);
    _isPlaceholderVisible = newIsPlaceholderVisible;

    if (auto *placeholder = PlaceholderElement())
    {
      placeholder->InvalidateStyle();
    }
  }

  void HTMLTextFormControlElement::SetCanShowPlaceholder(bool canShowPlaceholder) noexcept
  {
    _canShowPlaceholder = canShowPlaceholder;
    UpdatePlaceholderVisibility();
  }

  void HTMLTextFormControlElement::SetSelectionStart(uint32 start) noexcept
  {
    SetSelectionRange(start, std::max(start, SelectionEnd()), SelectionDirection());
  }

  void HTMLTextFormControlElement::SetSelectionEnd(uint32 end) noexcept
  {
    SetSelectionRange(std::min(end, SelectionStart()), end, SelectionDirection());
  }

  void HTMLTextFormControlElement::SetSelectionDirection(dom::DOMStringView direction) noexcept
  {
    SetSelectionRange(SelectionStart(), SelectionEnd(), direction);
  }

  void HTMLTextFormControlElement::Select(SelectionRevealMode revealMode) noexcept
  {
    Focus({.PreventScroll = true});
    if (SetSelectionRange(0, std::numeric_limits<uint32>::max(),
                          TextFieldSelectionDirection::SelectionHasNoDirection, revealMode))
    {
      ScheduleSelectEvent();
    }
  }

  dom::DOMString HTMLTextFormControlElement::SelectedText() const noexcept
  {
    if (!IsTextField())
    {
      return {};
    }
    auto value = Value();
    auto start = std::min<size_t>(SelectionStart(), value.size());
    auto end = std::min<size_t>(SelectionEnd(), value.size());
    return value.substr(start, end - start);
  }

  void HTMLTextFormControlElement::DispatchFormControlChangeEvent() noexcept
  {
    auto value = Value();
    if (_textAsOfLastFormControlChangeEvent != value)
    {
      SetTextAsOfLastFormControlChangeEvent(krys::move(value));
      DispatchChangeEvent();
    }
    SetChangedSinceLastFormControlChangeEvent(false);
    SetInteractedWithSinceLastFormSubmitEvent(true);
  }

  dom::ExceptionOr<void> HTMLTextFormControlElement::SetRangeText(dom::DOMStringView replacement) noexcept
  {
    return SetRangeText(replacement, SelectionStart(), SelectionEnd(), {});
  }

  dom::ExceptionOr<void> HTMLTextFormControlElement::SetRangeText(dom::DOMStringView replacement,
                                                                  uint32 start, uint32 end,
                                                                  dom::DOMStringView selectionMode) noexcept
  {
    if (start > end)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }

    auto text = InnerTextValue();
    auto textLength = static_cast<uint32>(text.size());
    auto replacementLength = static_cast<uint32>(replacement.size());
    uint32 newSelectionStart = SelectionStart();
    uint32 newSelectionEnd = SelectionEnd();

    start = SnapToCharacterStart(text, std::min(start, textLength));
    end = SnapToCharacterStart(text, std::min(end, textLength));

    if (start < end)
    {
      text.replace(start, end - start, replacement);
    }
    else
    {
      text.insert(start, replacement);
    }

    (void)SetValue(text, TextFieldEventBehavior::DispatchNoEvent, TextControlSetValueSelection::DoNotSet);

    using infra::StringAlgorithms;
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(selectionMode, u8"select"))
    {
      newSelectionStart = start;
      newSelectionEnd = start + replacementLength;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(selectionMode, u8"start"))
    {
      newSelectionStart = newSelectionEnd = start;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(selectionMode, u8"end"))
    {
      newSelectionStart = newSelectionEnd = start + replacementLength;
    }
    else
    {
      // Default is "preserve".
      int64 delta = static_cast<int64>(replacementLength) - (end - start);

      if (newSelectionStart > end)
      {
        newSelectionStart = static_cast<uint32>(newSelectionStart + delta);
      }
      else if (newSelectionStart > start)
      {
        newSelectionStart = start;
      }

      if (newSelectionEnd > end)
      {
        newSelectionEnd = static_cast<uint32>(newSelectionEnd + delta);
      }
      else if (newSelectionEnd > start)
      {
        newSelectionEnd = start + replacementLength;
      }
    }

    if (SetSelectionRange(newSelectionStart, newSelectionEnd,
                          TextFieldSelectionDirection::SelectionHasNoDirection))
    {
      ScheduleSelectEvent();
    }

    return {};
  }

  void HTMLTextFormControlElement::SetSelectionRange(uint32 start, uint32 end,
                                                     dom::DOMStringView directionString,
                                                     ForBindings forBindings) noexcept
  {
    auto direction = TextFieldSelectionDirection::SelectionHasNoDirection;
    if (directionString == u8"forward")
    {
      direction = TextFieldSelectionDirection::SelectionHasForwardDirection;
    }
    else if (directionString == u8"backward")
    {
      direction = TextFieldSelectionDirection::SelectionHasBackwardDirection;
    }

    if (SetSelectionRange(start, end, direction, SelectionRevealMode::DoNotReveal, forBindings))
    {
      ScheduleSelectEvent();
    }
  }

  bool HTMLTextFormControlElement::SetSelectionRange(uint32 start, uint32 end,
                                                     TextFieldSelectionDirection direction,
                                                     SelectionRevealMode revealMode,
                                                     ForBindings forBindings) noexcept
  {
    if (!IsTextField())
    {
      return false;
    }

    auto innerText = InnerTextElementCreatingShadowSubtreeIfNeeded();

    // Clamps to the current value length, and to whole characters.
    auto innerTextValue = InnerTextValue();
    auto innerTextValueLength = static_cast<uint32>(innerTextValue.size());
    end = SnapToCharacterStart(innerTextValue, std::min(end, innerTextValueLength));
    start = SnapToCharacterStart(innerTextValue, std::min(start, end));

    Ref<dom::Document> document = ShareRef(NodeDocument());
    bool hasFocus = document->FocusedElement() == this;

    auto *frame = document->Frame();
    if (direction == TextFieldSelectionDirection::SelectionHasNoDirection && frame
        && document->GetEditor().Behavior().ShouldConsiderSelectionAsDirectional())
    {
      direction = TextFieldSelectionDirection::SelectionHasForwardDirection;
    }

    if (!hasFocus && innerText)
    {
      if (!IsConnected())
      {
        return CacheSelection(start, end, direction);
      }

      // Cache selection if neither selection or focus is on the input.
      if (frame && EnclosingTextFormControl(document->Selection().Selection().Start()) != this)
      {
        return CacheSelection(start, end, direction);
      }

      // FIXME: Removing this synchronous layout requires fixing setSelectionWithoutUpdatingAppearance not
      // needing up-to-date style.
      document->UpdateLayoutIgnorePendingStylesheets();

      // Cache selection if renderer is invisible.
      if (auto *renderer = Renderer())
      {
        if (renderer->Style().GetVisibility() == Visibility::Hidden || !innerText->GetRenderBox()
            || !innerText->GetRenderBox()->Height())
        {
          return CacheSelection(start, end, direction);
        }
      }
    }

    auto previousSelectionStart = _cachedSelectionStart;
    auto previousSelectionEnd = _cachedSelectionEnd;
    auto previousSelectionDirection = _cachedSelectionDirection;

    CacheSelection(start, end, direction);
    dom::Position startPosition = PositionForIndex(innerText.get(), start);
    dom::Position endPosition;
    if (start == end)
    {
      endPosition = startPosition;
    }
    else
    {
      if (direction == TextFieldSelectionDirection::SelectionHasBackwardDirection)
      {
        endPosition = startPosition;
        startPosition = PositionForIndex(innerText.get(), end);
      }
      else
      {
        endPosition = PositionForIndex(innerText.get(), end);
      }
    }

    if (document->Frame())
    {
      auto options = FrameSelection::DefaultSetSelectionOptions();
      if (forBindings == ForBindings::Yes)
      {
        options.Add(FrameSelection::SetSelectionOption::ForBindings);
      }
      if (hasFocus)
      {
        options.Add(FrameSelection::SetSelectionOption::DoNotSetFocus);
      }
      switch (revealMode)
      {
        case SelectionRevealMode::DoNotReveal: break;
        case SelectionRevealMode::Reveal:
          options.Add(FrameSelection::SetSelectionOption::RevealSelection);
          break;
        case SelectionRevealMode::RevealUpToMainFrame:
          options.Add(FrameSelection::SetSelectionOption::RevealSelectionUpToMainFrame);
          break;
        case SelectionRevealMode::DelegateMainFrameScroll:
          options.Add(FrameSelection::SetSelectionOption::DelegateMainFrameScroll);
          break;
      }
      document->Selection().MoveWithoutValidationTo(
        startPosition, endPosition, direction != TextFieldSelectionDirection::SelectionHasNoDirection,
        options);
    }

    return _cachedSelectionStart != previousSelectionStart || _cachedSelectionEnd != previousSelectionEnd
           || _cachedSelectionDirection != previousSelectionDirection;
  }

  bool HTMLTextFormControlElement::CacheSelection(uint32 start, uint32 end,
                                                  TextFieldSelectionDirection direction) noexcept
  {
    bool didChange =
      _cachedSelectionStart != start || _cachedSelectionEnd != end || _cachedSelectionDirection != direction;
    _cachedSelectionStart = start;
    _cachedSelectionEnd = end;
    _cachedSelectionDirection = direction;
    _hasCachedSelection = true;
    return didChange;
  }

  int32 HTMLTextFormControlElement::IndexForVisiblePosition(const VisiblePosition &position) const noexcept
  {
    auto innerText = InnerTextElement();
    if (!innerText || !innerText->Contains(position.DeepEquivalent().AnchorNode()))
    {
      return 0;
    }
    return static_cast<int32>(IndexForPosition(position.DeepEquivalent()));
  }

  VisiblePosition HTMLTextFormControlElement::VisiblePositionForIndex(int32 index) const noexcept
  {
    krys_debug_assert(InnerTextElement());
    return VisiblePosition(PositionForIndex(InnerTextElement().get(), static_cast<uint32>(index)));
  }

  uint32 HTMLTextFormControlElement::SelectionStart() const noexcept
  {
    if (!IsTextField())
    {
      return 0;
    }
    if (NodeDocument().FocusedElement() != this && HasCachedSelection())
    {
      return _cachedSelectionStart;
    }

    return ComputeSelectionStart();
  }

  uint32 HTMLTextFormControlElement::ComputeSelectionStart() const noexcept
  {
    krys_debug_assert(IsTextField());
    if (!NodeDocument().Frame())
    {
      return 0;
    }

    return IndexForPosition(NodeDocument().Selection().Selection().Start());
  }

  uint32 HTMLTextFormControlElement::SelectionEnd() const noexcept
  {
    if (!IsTextField())
    {
      return 0;
    }
    if (NodeDocument().FocusedElement() != this && HasCachedSelection())
    {
      return _cachedSelectionEnd;
    }
    return ComputeSelectionEnd();
  }

  uint32 HTMLTextFormControlElement::ComputeSelectionEnd() const noexcept
  {
    krys_debug_assert(IsTextField());
    if (!NodeDocument().Frame())
    {
      return 0;
    }

    return IndexForPosition(NodeDocument().Selection().Selection().End());
  }

  dom::DOMStringView HTMLTextFormControlElement::SelectionDirection() const noexcept
  {
    if (!IsTextField())
    {
      return DirectionString(TextFieldSelectionDirection::SelectionHasNoDirection);
    }
    if (NodeDocument().FocusedElement() != this && HasCachedSelection())
    {
      return DirectionString(_cachedSelectionDirection);
    }

    return DirectionString(ComputeSelectionDirection());
  }

  TextFieldSelectionDirection HTMLTextFormControlElement::ComputeSelectionDirection() const noexcept
  {
    krys_debug_assert(IsTextField());
    if (!NodeDocument().Frame())
    {
      return TextFieldSelectionDirection::SelectionHasNoDirection;
    }

    const auto &selection = NodeDocument().Selection().Selection();
    return selection.GetDirectionality() == Directionality::Strong
             ? (selection.IsBaseFirst() ? TextFieldSelectionDirection::SelectionHasForwardDirection
                                        : TextFieldSelectionDirection::SelectionHasBackwardDirection)
             : TextFieldSelectionDirection::SelectionHasNoDirection;
  }

  Maybe<dom::SimpleRange> HTMLTextFormControlElement::Selection() const noexcept
  {
    if (!Renderer() || !IsTextField() || !HasCachedSelection())
    {
      return null;
    }

    uint32 start = _cachedSelectionStart;
    uint32 end = _cachedSelectionEnd;

    krys_debug_assert(start <= end);
    auto innerText = InnerTextElement();
    if (!innerText)
    {
      return null;
    }

    if (!innerText->FirstChild())
    {
      return dom::SimpleRange {{Ref<dom::Node>(ShareRef(*innerText)), 0},
                               {Ref<dom::Node>(ShareRef(*innerText)), 0}};
    }

    uint32 offset = 0;
    RefPtr<dom::Node> startNode;
    RefPtr<dom::Node> endNode;
    for (dom::Node *node = innerText->FirstChild(); node;
         node = dom::TreeQueries::Next(*node, innerText.get()))
    {
      krys_debug_assert(!node->FirstChild());
      krys_debug_assert(Is<dom::Text>(*node) || IsHTMLElementOfType(node, TagName::br));

      uint32 length = [&]
      {
        auto *textNode = DynamicDowncast<dom::Text>(*node);
        return textNode ? static_cast<uint32>(textNode->Length()) : 1u;
      }();

      if (offset <= start && start <= offset + length)
      {
        SetContainerAndOffsetForRange(*node, start - offset, startNode, start);
      }

      if (offset <= end && end <= offset + length)
      {
        SetContainerAndOffsetForRange(*node, end - offset, endNode, end);
        break;
      }

      offset += length;
    }

    if (!startNode || !endNode)
    {
      return null;
    }

    return dom::SimpleRange {{Ref<dom::Node>(ShareRef(*startNode)), start},
                             {Ref<dom::Node>(ShareRef(*endNode)), end}};
  }

  void HTMLTextFormControlElement::RestoreCachedSelection(SelectionRevealMode revealMode) noexcept
  {
    if (SetSelectionRange(_cachedSelectionStart, _cachedSelectionEnd, _cachedSelectionDirection, revealMode))
    {
      ScheduleSelectEvent();
    }
  }

  bool HTMLTextFormControlElement::SelectionChanged(bool shouldFireSelectEvent) noexcept
  {
    if (!IsTextField())
    {
      return false;
    }

    // FIXME: Don't re-compute selection start and end if this function was called inside setSelectionRange.
    // selectionStart() or selectionEnd() will return cached selection when this node doesn't have focus
    uint32 previousSelectionStart = _cachedSelectionStart;
    uint32 previousSelectionEnd = _cachedSelectionEnd;
    CacheSelection(ComputeSelectionStart(), ComputeSelectionEnd(), ComputeSelectionDirection());

    if (shouldFireSelectEvent && _cachedSelectionStart != _cachedSelectionEnd)
    {
      auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Select), true, false);
      (void)dom::EventDispatcher::DispatchToTarget(*event, this);
    }

    return previousSelectionStart != _cachedSelectionStart || previousSelectionEnd != _cachedSelectionEnd;
  }

  void HTMLTextFormControlElement::ScheduleSelectionChangeEvent() noexcept
  {
    if (_hasScheduledSelectionChangeEvent)
    {
      return;
    }

    _hasScheduledSelectionChangeEvent = true;
    EventLoop::Current().QueueTask(TaskSource::UserInteraction, NodeDocument(),
                                   [textControl = Ref<HTMLTextFormControlElement>(ShareRef(*this))]
                                   {
                                     textControl->_hasScheduledSelectionChangeEvent = false;
                                     auto event = dom::Event::Create(
                                       dom::DOMStringAtom(EventNames::SelectionChange), true, false);
                                     (void)dom::EventDispatcher::DispatchToTarget(*event, textControl.get());
                                   });
  }

  void HTMLTextFormControlElement::ScheduleSelectEvent() noexcept
  {
    EventLoop::Current().QueueTask(TaskSource::UserInteraction, NodeDocument(),
                                   [textControl = Ref<HTMLTextFormControlElement>(ShareRef(*this))]
                                   {
                                     auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Select),
                                                                     true, false);
                                     (void)dom::EventDispatcher::DispatchToTarget(*event, textControl.get());
                                   });
    ScheduleSelectionChangeEvent();
  }

  void HTMLTextFormControlElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                                      Maybe<dom::DOMStringView> oldValue,
                                                      Maybe<dom::DOMStringView> value,
                                                      dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"placeholder")
    {
      UpdatePlaceholderText();
      UpdatePlaceholderVisibility();
    }
  }

  void HTMLTextFormControlElement::DisabledStateChanged() noexcept
  {
    HTMLFormControlElement::DisabledStateChanged();
    UpdateInnerTextElementEditability();
  }

  void HTMLTextFormControlElement::ReadOnlyStateChanged() noexcept
  {
    HTMLFormControlElement::ReadOnlyStateChanged();
    UpdateInnerTextElementEditability();
  }

  bool HTMLTextFormControlElement::IsInnerTextElementEditable() const noexcept
  {
    return IsMutable();
  }

  void HTMLTextFormControlElement::UpdateInnerTextElementEditability() noexcept
  {
    if (auto innerText = InnerTextElement())
    {
      innerText->UpdateInnerTextElementEditability(IsInnerTextElementEditable());
    }
  }

  bool HTMLTextFormControlElement::LastChangeWasUserEdit() const noexcept
  {
    if (!IsTextField())
    {
      return false;
    }
    return _lastChangeWasUserEdit;
  }

  bool HTMLTextFormControlElement::WasEverChangedByUserEdit() const noexcept
  {
    if (!IsTextField())
    {
      return false;
    }
    return _wasEverChangedByUserEdit;
  }

  void HTMLTextFormControlElement::SetInnerTextValue(dom::DOMString &&value) noexcept
  {
    auto innerText = InnerTextElementCreatingShadowSubtreeIfNeeded();
    if (!innerText)
    {
      return;
    }

    krys_debug_assert(IsTextField());
    auto previousValue = InnerTextValueFrom(*innerText);
    bool textIsChanged = value != previousValue;
    if (textIsChanged || !innerText->HasChildNodes())
    {
      // FIXME: Telling accessibility the value changed (onEditableTextValueChanged), with accessibility.
      bool endsWithNewLine = value.ends_with(u8'\n') || value.ends_with(u8'\r');
      innerText->InnerText(krys::move(value));

      if (endsWithNewLine)
      {
        (void)innerText->AppendChild(*CreateRef<HTMLBRElement>(NodeDocument()));
      }
    }

    SetFormControlValueMatchesRenderer(true);
  }

  dom::DOMString HTMLTextFormControlElement::InnerTextValue() const noexcept
  {
    auto innerText = InnerTextElement();
    return innerText ? InnerTextValueFrom(*innerText) : dom::DOMString {};
  }

  uint32 HTMLTextFormControlElement::IndexForPosition(const dom::Position &passedPosition) const noexcept
  {
    auto innerText = InnerTextElement();
    if (!innerText || passedPosition.IsNull() || !innerText->Contains(passedPosition.AnchorNode()))
    {
      return 0;
    }

    if (dom::PositionBeforeNode(innerText.get()) == passedPosition)
    {
      return 0;
    }

    uint32 index = 0;
    dom::Node *startNode = passedPosition.ComputeNodeBeforePosition();
    if (!startNode)
    {
      startNode = passedPosition.GetContainerNode();
    }
    krys_debug_assert(startNode);
    krys_debug_assert(innerText->Contains(startNode));

    for (dom::Node *node = startNode; node; node = dom::TreeQueries::Previous(*node, innerText.get()))
    {
      if (auto *text = DynamicDowncast<dom::Text>(*node))
      {
        auto length = static_cast<uint32>(text->Length());
        if (node == passedPosition.GetContainerNode())
        {
          index += std::min<uint32>(length, static_cast<uint32>(passedPosition.OffsetInContainerNode()));
        }
        else
        {
          index += length;
        }
      }
      else if (IsHTMLElementOfType(node, TagName::br))
      {
        ++index;
      }
    }

    auto length = static_cast<uint32>(InnerTextValue().size());
    // FIXME: We shouldn't have to call innerTextValue() just to ignore the last LF. See finishText.
    index = std::min(index, length);
    return index;
  }

  dom::DOMString HTMLTextFormControlElement::ValueWithHardLineBreaks() const noexcept
  {
    // FIXME: It's not acceptable to ignore the HardWrap setting when there is no renderer.
    // While we have no evidence this has ever been a practical problem, it would be best to fix it some day.
    if (!IsTextField())
    {
      return Value();
    }

    auto innerText = InnerTextElement();
    if (!innerText || innerText->TextContent().value_or(dom::DOMString {}).empty())
    {
      return Value();
    }

    auto *renderer = innerText->Renderer();
    if (!renderer)
    {
      return Value();
    }

    dom::Node *breakNode = nullptr;
    uint32 breakOffset = 0;
    auto currentLineBox = inline_iterator::FirstLineBoxFor(*renderer);
    if (!currentLineBox)
    {
      return Value();
    }

    auto skipToNextSoftLineBreakPosition = [&]
    {
      while (currentLineBox)
      {
        auto lastRun = currentLineBox->LineRightmostLeafBox();
        krys_debug_assert(lastRun);
        // Skip last line.
        currentLineBox.TraverseNext();
        if (currentLineBox && !lastRun->IsLineBreak())
        {
          breakNode = lastRun->Renderer().GetNode();
          breakOffset = lastRun->MaximumCaretOffset();
          return;
        }
      }
      breakNode = nullptr;
      breakOffset = 0;
    };

    skipToNextSoftLineBreakPosition();

    dom::DOMString result;
    for (dom::Node *node = innerText->FirstChild(); node;
         node = dom::TreeQueries::Next(*node, innerText.get()))
    {
      if (IsHTMLElementOfType(node, TagName::br))
      {
        result.push_back(u8'\n');
      }
      else if (auto *textNode = DynamicDowncast<dom::Text>(*node))
      {
        dom::DOMStringView data = textNode->Data();
        auto length = static_cast<uint32>(data.size());
        uint32 position = 0;
        while (breakNode == node && breakOffset <= length)
        {
          if (breakOffset > position)
          {
            result.append(data.substr(position, breakOffset - position));
            position = breakOffset;
            result.push_back(u8'\n');
          }
          skipToNextSoftLineBreakPosition();
        }
        result.append(data.substr(position, length - position));
      }
      while (breakNode == node)
      {
        skipToNextSoftLineBreakPosition();
      }
    }
    StripTrailingNewline(result);
    return result;
  }

  HTMLTextFormControlElement *EnclosingTextFormControl(const dom::Position &position) noexcept
  {
    auto *container = position.GetContainerNode();
    if (!container)
    {
      return nullptr;
    }
    auto *ancestor = dom::TreeQueries::ShadowHost(*container);
    if (!ancestor)
    {
      return nullptr;
    }
    auto *textControl = DynamicDowncast<HTMLTextFormControlElement>(*ancestor);
    return textControl && textControl->IsTextField() ? textControl : nullptr;
  }

  void HTMLTextFormControlElement::AdjustInnerTextStyle(const RenderStyle &parentStyle,
                                                        RenderStyle &textBlockStyle) const noexcept
  {
    // The inner block, if present, always has its direction set to LTR,
    // so we need to inherit the direction and unicode-bidi style from the element.
    textBlockStyle.SetDirection(parentStyle.GetWritingMode().ComputedTextDirection());
    textBlockStyle.SetUnicodeBidi(parentStyle.GetUnicodeBidi());

    // FIXME: The inner text element's `-webkit-user-modify` presentational hint, which nothing sets.

    if (parentStyle.GetFieldSizing() == FieldSizing::Content)
    {
      textBlockStyle.SetLogicalMinWidth(style::MinimumSize::Fixed {static_cast<float>(CaretWidth())});
    }
  }

  int32 HTMLTextFormControlElement::ParseLengthAttribute(Maybe<dom::DOMStringView> value) noexcept
  {
    if (!value)
    {
      return -1;
    }
    auto result = Numbers::ParseNonNegativeInteger(*value);
    if (!result.Success() || result.Value > static_cast<uint64>(std::numeric_limits<int32>::max()))
    {
      return -1;
    }
    return static_cast<int32>(result.Value);
  }

  size_t HTMLTextFormControlElement::LengthForAPIValue(dom::DOMStringView text) noexcept
  {
    size_t length = 0;
    for (char8 character : text)
    {
      auto byte = static_cast<uint8>(character);
      if ((byte & 0xC0) == 0x80)
      {
        continue;
      }
      // A code point beyond the Basic Multilingual Plane (four UTF-8 bytes) is a surrogate pair.
      length += byte >= 0xF0 ? 2 : 1;
    }
    return length;
  }

  dom::DOMString HTMLTextFormControlElement::DirectionForFormData() const noexcept
  {
    return css::EffectiveTextDirection(*this) == TextDirection::LTR ? u8"ltr" : u8"rtl";
  }
}
