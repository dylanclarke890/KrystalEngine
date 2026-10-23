#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTextControl.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <algorithm>
#include <climits>
#include <unicode/utf8.h>

namespace krys::boo
{
  namespace
  {
    constexpr char32 ObjectReplacementCharacter = 0xFFFC;

    KRYS_NODISCARD bool IsHTMLElementOfType(const dom::Node &node, html::TagName tagName) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element && element->TagQualifiedName().TagName() == tagName;
    }
  }

  // --------

  constexpr uint32 BitsInWord = sizeof(uint32) * 8;
  constexpr uint32 BitInWordMask = BitsInWord - 1;

  void BitStack::Push(bool bit) noexcept
  {
    uint32 index = _size / BitsInWord;
    uint32 shift = _size & BitInWordMask;
    if (!shift && index == _words.size())
    {
      _words.push_back(0);
    }
    uint32 &word = _words[index];
    uint32 mask = 1u << shift;
    if (bit)
    {
      word |= mask;
    }
    else
    {
      word &= ~mask;
    }
    ++_size;
  }

  void BitStack::Pop() noexcept
  {
    if (_size)
    {
      --_size;
    }
  }

  bool BitStack::Top() const noexcept
  {
    if (!_size)
    {
      return false;
    }
    uint32 shift = (_size - 1) & BitInWordMask;
    uint32 index = (_size - 1) / BitsInWord;
    return _words[index] & (1u << shift);
  }

  // --------

  namespace
  {
    // This function is like Range::pastLastNode, except for the fact that it can climb up out of shadow
    // trees.
    KRYS_NODISCARD RefPtr<dom::Node> NextInPreOrderCrossingShadowBoundaries(dom::Node &rangeEndContainer,
                                                                            int32 rangeEndOffset) noexcept
    {
      if (rangeEndOffset >= 0 && !Is<dom::CharacterData>(rangeEndContainer))
      {
        if (auto *next = rangeEndContainer.TraverseToChildAt(static_cast<uint32>(rangeEndOffset)))
        {
          return ShareRefPtr(next);
        }
      }
      for (dom::Node *node = &rangeEndContainer; node; node = node->ParentOrShadowHostNode())
      {
        if (auto *next = node->NextSibling())
        {
          return ShareRefPtr(next);
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD inline bool FullyClipsContents(const dom::Node &node,
                                                  TextIteratorBehaviors behaviors) noexcept
    {
      auto *renderer = node.Renderer();
      if (!renderer)
      {
        auto *element = DynamicDowncast<dom::Element>(node);
        return element && !element->HasDisplayContents();
      }
      auto *box = DynamicDowncast<RenderBox>(*renderer);
      if (!box || !box->HasNonVisibleOverflow())
      {
        return false;
      }

      // Quirk to keep copy/paste in the CodeMirror editor version used in Jenkins working.
      if (IsHTMLElementOfType(node, html::TagName::textarea))
      {
        return box->Size().IsEmpty();
      }

      if (behaviors.Contains(TextIteratorBehavior::EntersSkippedContentRelevantToUser)
          && IsSkippedContentRoot(*box))
      {
        // This may reveal collapsed content to find-in-page, but it's uncommon (and highly redundant) to have
        // computed block height 0px while applying c-v: hidden.
        return false;
      }

      return box->ContentBoxSize().IsEmpty();
    }

    KRYS_NODISCARD inline bool IgnoresContainerClip(const dom::Node &node) noexcept
    {
      auto *renderer = node.Renderer();
      if (!renderer || renderer->IsRenderTextOrLineBreak())
      {
        return false;
      }
      return renderer->IsOutOfFlowPositioned();
    }

    void PushFullyClippedState(BitStack &stack, dom::Node &node, TextIteratorBehaviors behaviors) noexcept
    {
      // Push true if this node full clips its contents, or if a parent already has fully
      // clipped and this is not a node that ignores its container's clip.
      stack.Push(FullyClipsContents(node, behaviors) || (stack.Top() && !IgnoresContainerClip(node)));
    }

    void SetUpFullyClippedStack(BitStack &stack, dom::Node &node, TextIteratorBehaviors behaviors) noexcept
    {
      // Put the nodes in a vector so we can iterate in reverse order.
      // FIXME: This (and TextIterator in general) should use ComposedTreeIterator.
      List<dom::Node *> ancestry;
      for (auto *parent = node.ParentOrShadowHostNode(); parent; parent = parent->ParentOrShadowHostNode())
      {
        ancestry.push_back(parent);
      }

      // Call pushFullyClippedState on each node starting with the earliest ancestor.
      size_t size = ancestry.size();
      for (size_t i = 0; i < size; ++i)
      {
        PushFullyClippedState(stack, *ancestry[size - i - 1], behaviors);
      }
      PushFullyClippedState(stack, node, behaviors);
    }

    KRYS_NODISCARD bool IsClippedByFrameAncestor(const dom::Document &,
                                                 TextIteratorBehaviors behaviors) noexcept
    {
      if (!behaviors.Contains(TextIteratorBehavior::ClipsToFrameAncestors))
      {
        return false;
      }

      // FIXME: The frame owner elements' clips, once there are subframes.
      return false;
    }
  }

  // FIXME: editingIgnoresContent and isRendererReplacedElement try to do the same job.
  // It's not good to have both of them.
  bool IsRendererReplacedElement(const RenderObject *renderer, TextIteratorBehaviors) noexcept
  {
    if (!renderer)
    {
      return false;
    }

    // FIXME: Widgets, media and attachments, once there are any.
    if (renderer->IsRenderImage())
    {
      return true;
    }

    if (auto *element = DynamicDowncast<dom::Element>(renderer->GetNode()))
    {
      auto isFormControl = IsHTMLElementOfType(*element, html::TagName::button)
                           || IsHTMLElementOfType(*element, html::TagName::fieldset)
                           || IsHTMLElementOfType(*element, html::TagName::input)
                           || IsHTMLElementOfType(*element, html::TagName::output)
                           || IsHTMLElementOfType(*element, html::TagName::select)
                           || IsHTMLElementOfType(*element, html::TagName::textarea);
      if (isFormControl || IsHTMLElementOfType(*element, html::TagName::legend)
          || IsHTMLElementOfType(*element, html::TagName::progress)
          || IsHTMLElementOfType(*element, html::TagName::meter))
      {
        return true;
      }
      if (auto role = element->GetAttribute(u8"role");
          role && infra::StringAlgorithms::ASCIICaseInsensitiveMatch(dom::DOMStringView(*role), u8"img"))
      {
        return true;
      }
    }

    return false;
  }

  // --------

  void TextIteratorCopyableText::Reset() noexcept
  {
    _singleCharacterLength = 0;
    _string = utf8_string();
    _offset = 0;
    _length = 0;
  }

  void TextIteratorCopyableText::Set(utf8_string &&string) noexcept
  {
    _singleCharacterLength = 0;
    _string = krys::move(string);
    _offset = 0;
    _length = static_cast<uint32>(_string.size());
  }

  void TextIteratorCopyableText::Set(utf8_string &&string, uint32 offset, uint32 length) noexcept
  {
    krys_debug_assert(offset < string.size());
    krys_debug_assert(length);
    krys_debug_assert(length <= string.size() - offset);

    _singleCharacterLength = 0;
    _string = krys::move(string);
    _offset = offset;
    _length = length;
  }

  void TextIteratorCopyableText::Set(char32 singleCharacter) noexcept
  {
    int32 length = 0;
    UBool isError = false;
    U8_APPEND(reinterpret_cast<uint8_t *>(_singleCharacter), length, 4, static_cast<UChar32>(singleCharacter),
              isError);
    _singleCharacterLength = static_cast<uint32>(length);
    _string = utf8_string();
    _offset = 0;
    _length = 0;
  }

  void TextIteratorCopyableText::AppendToString(utf8_string &builder) const noexcept
  {
    builder.append(Text());
  }

  // --------

  namespace
  {
    KRYS_NODISCARD dom::Node *FirstNode(const dom::BoundaryPoint &point) noexcept
    {
      if (Is<dom::CharacterData>(*point.Container))
      {
        return point.Container.get();
      }
      if (auto *child = point.Container->TraverseToChildAt(static_cast<uint32>(point.Offset)))
      {
        return child;
      }
      if (!point.Offset)
      {
        return point.Container.get();
      }
      return dom::TreeQueries::NextSkippingChildren(*point.Container);
    }
  }

  TextIterator::TextIterator(const dom::SimpleRange &range, TextIteratorBehaviors behaviors) noexcept
      : _behaviors(behaviors)
  {
    krys_debug_assert(
      !_behaviors.Contains(TextIteratorBehavior::EmitsObjectReplacementCharacters)
      || !_behaviors.Contains(TextIteratorBehavior::EmitsObjectReplacementCharactersForImages));

    range.Start.GetDocument().UpdateLayoutIgnorePendingStylesheets();

    _startContainer = range.Start.Container;
    _startOffset = static_cast<int32>(range.Start.Offset);
    _endContainer = range.End.Container;
    _endOffset = static_cast<int32>(range.End.Offset);

    _currentNode = ShareRefPtr(FirstNode(range.Start));
    if (!_currentNode)
    {
      return;
    }

    Init();
  }

  void TextIterator::Init() noexcept
  {
    auto *currentNode = _currentNode.get();
    if (IsClippedByFrameAncestor(currentNode->NodeDocument(), _behaviors))
    {
      return;
    }

    SetUpFullyClippedStack(_fullyClippedStack, *currentNode, _behaviors);

    _offset = currentNode == _startContainer.get() ? _startOffset : 0;

    _pastEndNode = NextInPreOrderCrossingShadowBoundaries(*_endContainer, _endOffset);

    _positionNode = _currentNode;

    Advance();
  }

  TextIterator::~TextIterator() noexcept = default;

  utf8_string TextIterator::RendererTextForBehavior(const RenderText &renderer) const noexcept
  {
    return _behaviors.Contains(TextIteratorBehavior::EmitsOriginalText) ? renderer.OriginalText()
                                                                        : renderer.Text();
  }

  namespace
  {
    // FIXME: Traversing the flat tree (TraversesFlatTree), once something asks to (finding text).
    KRYS_NODISCARD inline dom::Node *FirstChild(TextIteratorBehaviors, dom::Node &node) noexcept
    {
      return node.FirstChild();
    }

    KRYS_NODISCARD inline dom::Node *NextSibling(TextIteratorBehaviors, dom::Node &node) noexcept
    {
      return node.NextSibling();
    }

    KRYS_NODISCARD inline dom::Node *NextNode(TextIteratorBehaviors, dom::Node &node) noexcept
    {
      return dom::TreeQueries::Next(node);
    }

    KRYS_NODISCARD inline bool IsDescendantOf(TextIteratorBehaviors, dom::Node &node,
                                              dom::Node &possibleAncestor) noexcept
    {
      return node.IsDescendantOf(&possibleAncestor);
    }

    KRYS_NODISCARD inline dom::Node *ParentNodeOrShadowHost(TextIteratorBehaviors, dom::Node &node) noexcept
    {
      return node.ParentOrShadowHostNode();
    }

    KRYS_NODISCARD inline bool HasDisplayContents(dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && element->HasDisplayContents();
    }

    KRYS_NODISCARD bool IsRendererAccessible(const RenderObject *renderer,
                                             TextIteratorBehaviors behaviors) noexcept
    {
      if (!renderer)
      {
        return false;
      }

      auto &style = renderer->Style();
      if (style.UsedUserSelect() == UserSelect::None
          && behaviors.Contains(TextIteratorBehavior::IgnoresUserSelectNone))
      {
        return false;
      }

      if (renderer->IsSkippedContent())
      {
        if (!behaviors.Contains(TextIteratorBehavior::EntersSkippedContentRelevantToUser))
        {
          return false;
        }
        return style.UsedContentVisibility() == ContentVisibility::Auto || style.AutoRevealsWhenFound();
      }

      return true;
    }

    KRYS_NODISCARD bool IsConsideredSkippedContent(const RenderBox *renderBox,
                                                   TextIteratorBehaviors behaviors) noexcept
    {
      if (!renderBox || !IsSkippedContentRoot(*renderBox))
      {
        return false;
      }

      if (behaviors.Contains(TextIteratorBehavior::EntersSkippedContentRelevantToUser))
      {
        return renderBox->Style().UsedContentVisibility() == ContentVisibility::Hidden
               && !renderBox->Style().AutoRevealsWhenFound();
      }

      return true;
    }
  }

  void TextIterator::Advance() noexcept
  {
    krys_debug_assert(!AtEnd());

    // reset the run information
    _positionNode = nullptr;
    _copyableText.Reset();
    _text = utf8_stringview();

    // handle remembered node that needed a newline after the text node's newline
    if (auto nodeForAdditionalNewline = std::exchange(_nodeForAdditionalNewline, nullptr))
    {
      // Emit the extra newline, and position it *inside* m_node, after m_node's
      // contents, in case it's a block, in the same way that we position the first
      // newline. The range for the emitted newline should start where the line
      // break begins.
      // FIXME: It would be cleaner if we emitted two newlines during the last
      // iteration, instead of using m_needsAnotherNewline.
      auto parentNode = ShareRefPtr<dom::Node>(nodeForAdditionalNewline->ParentNode());
      EmitCharacter('\n', krys::move(parentNode), krys::move(nodeForAdditionalNewline), 1, 1);
      return;
    }

    if (!_textRun && _remainingTextRun)
    {
      RevertToRemainingTextRun();
    }

    // handle remembered text box
    if (_textRun)
    {
      HandleTextRun();
      if (_positionNode)
      {
        return;
      }
    }

    while (_currentNode && _currentNode != _pastEndNode)
    {
      // if the range ends at offset 0 of an element, represent the
      // position, but not the content, of that element e.g. if the
      // node is a blockflow element, emit a newline that
      // precedes the element
      if (_currentNode == _endContainer && !_endOffset)
      {
        RepresentNodeOffsetZero();
        _currentNode = nullptr;
        return;
      }

      auto *renderer = _currentNode->Renderer();
      if (!_handledNode)
      {
        if (!IsRendererAccessible(renderer, _behaviors))
        {
          _handledNode = true;
          _handledChildren = !HasDisplayContents(*_currentNode) && !renderer;
        }
        else
        {
          if (IsConsideredSkippedContent(DynamicDowncast<RenderBox>(renderer), _behaviors))
          {
            _handledChildren = true;
          }
          else if (renderer->IsRenderText() && Is<dom::Text>(*_currentNode))
          {
            _handledNode = HandleTextNode();
          }
          else if (IsRendererReplacedElement(renderer, _behaviors))
          {
            _handledNode = HandleReplacedElement();
          }
          else
          {
            _handledNode = HandleNonTextNode();
          }
          if (_positionNode)
          {
            return;
          }
        }
      }

      // find a new current node to handle in depth-first manner,
      // calling exitNode() as we come back thru a parent node

      RefPtr<dom::Node> next =
        _handledChildren ? nullptr : ShareRefPtr(FirstChild(_behaviors, *_currentNode));
      _offset = 0;
      if (!next)
      {
        RefPtr<dom::Node> currentNode = _currentNode;
        next = ShareRefPtr(NextSibling(_behaviors, *currentNode));
        if (!next)
        {
          bool pastEnd = NextNode(_behaviors, *currentNode) == _pastEndNode.get();
          RefPtr<dom::Node> parentNode =
            ShareRefPtr<dom::Node>(ParentNodeOrShadowHost(_behaviors, *currentNode));
          while (!next && parentNode)
          {
            if ((pastEnd && parentNode == _endContainer)
                || IsDescendantOf(_behaviors, *_endContainer, *parentNode))
            {
              return;
            }
            bool haveRenderer = IsRendererAccessible(currentNode->Renderer(), _behaviors);
            RefPtr<dom::Node> exitedNode = krys::move(currentNode);
            _currentNode = krys::move(parentNode);
            currentNode = _currentNode;
            _fullyClippedStack.Pop();
            parentNode = ShareRefPtr<dom::Node>(ParentNodeOrShadowHost(_behaviors, *currentNode));
            if (haveRenderer)
            {
              ExitNode(exitedNode.get());
            }
            if (_positionNode)
            {
              _handledNode = true;
              _handledChildren = true;
              return;
            }
            next = ShareRefPtr(NextSibling(_behaviors, *currentNode));
            if (next && IsRendererAccessible(currentNode->Renderer(), _behaviors))
            {
              ExitNode(currentNode.get());
            }
          }
        }
        _fullyClippedStack.Pop();
      }

      // set the new current node
      _currentNode = krys::move(next);
      if (_currentNode)
      {
        PushFullyClippedState(_fullyClippedStack, *_currentNode, _behaviors);
      }
      _handledNode = false;
      _handledChildren = false;
      _handledFirstLetter = false;
      _firstLetterText = nullptr;

      // how would this ever be?
      if (_positionNode)
      {
        return;
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD bool HasVisibleTextNode(const RenderText &renderer) noexcept
    {
      if (renderer.Style().GetVisibility() == Visibility::Visible)
      {
        return true;
      }
      if (auto *renderTextFragment = DynamicDowncast<RenderTextFragment>(renderer))
      {
        if (auto *firstLetter = renderTextFragment->FirstLetter())
        {
          if (firstLetter->Style().GetVisibility() == Visibility::Visible)
          {
            return true;
          }
        }
      }
      return false;
    }
  }

  bool TextIterator::HandleTextNode() noexcept
  {
    Ref textNode = ShareRef(Downcast<dom::Text>(*_currentNode));

    if (_fullyClippedStack.Top() && !_behaviors.Contains(TextIteratorBehavior::IgnoresStyleVisibility))
    {
      return false;
    }

    auto &renderer = *textNode->Renderer();
    _lastTextNode = textNode;
    auto rendererText = RendererTextForBehavior(renderer);

    // handle pre-formatted text
    if (!renderer.Style().CollapseWhiteSpace())
    {
      int32 runStart = _offset;
      if (_lastTextNodeEndedWithCollapsedSpace && HasVisibleTextNode(renderer))
      {
        EmitCharacter(' ', krys::move(textNode), nullptr, runStart, runStart);
        return false;
      }
      if (auto *renderTextFragment = DynamicDowncast<RenderTextFragment>(renderer);
          renderTextFragment && !_handledFirstLetter && !_offset)
      {
        HandleTextNodeFirstLetter(*renderTextFragment);
        if (_firstLetterText)
        {
          utf8_string firstLetter = _firstLetterText->Text();
          EmitText(*textNode, *_firstLetterText, _offset, _offset + static_cast<int32>(firstLetter.size()));
          _firstLetterText = nullptr;
          _textRun = {};
          return false;
        }
      }
      if (renderer.Style().GetVisibility() != Visibility::Visible
          && !_behaviors.Contains(TextIteratorBehavior::IgnoresStyleVisibility))
      {
        return false;
      }
      int32 rendererTextLength = static_cast<int32>(rendererText.size());
      int32 end = (textNode.get() == _endContainer.get()) ? _endOffset : INT_MAX;
      int32 runEnd = std::min(rendererTextLength, end);

      if (runStart >= runEnd)
      {
        return true;
      }

      EmitText(*textNode, renderer, runStart, runEnd);
      return true;
    }

    std::tie(_textRun, _textRunLogicalOrderCache) = inline_iterator::FirstTextBoxInLogicalOrderFor(renderer);

    if (auto *renderTextFragment = DynamicDowncast<RenderTextFragment>(renderer);
        renderTextFragment && !_handledFirstLetter && !_offset)
    {
      HandleTextNodeFirstLetter(const_cast<RenderTextFragment &>(*renderTextFragment));
    }
    else if (!_textRun && rendererText.size())
    {
      if (renderer.Style().GetVisibility() != Visibility::Visible
          && !_behaviors.Contains(TextIteratorBehavior::IgnoresStyleVisibility))
      {
        return false;
      }
      _lastTextNodeEndedWithCollapsedSpace = true; // entire block is collapsed space
      return true;
    }

    HandleTextRun();
    return true;
  }

  void TextIterator::HandleTextRun() noexcept
  {
    Ref textNode = ShareRef(Downcast<dom::Text>(*_currentNode));

    const RenderText &renderer = _firstLetterText ? *_firstLetterText : *textNode->Renderer();
    if (renderer.Style().GetVisibility() != Visibility::Visible
        && !_behaviors.Contains(TextIteratorBehavior::IgnoresStyleVisibility))
    {
      _textRun = {};
      return;
    }

    auto [firstTextRun, orderCache] = inline_iterator::FirstTextBoxInLogicalOrderFor(renderer);

    auto rendererText = RendererTextForBehavior(renderer);
    uint32 rangeStart = static_cast<uint32>(_offset);
    auto rangeEnd = Maybe<uint32> {};
    if (textNode.get() == _endContainer.get())
    {
      rangeEnd = static_cast<uint32>(_endOffset);
    }

    while (_textRun)
    {
      auto textRunStart = _textRun->Start();
      auto textRunEnd = textRunStart + _textRun->Length();

      auto runStart = std::max(textRunStart, rangeStart);
      auto runEnd = std::min(textRunEnd, rangeEnd.value_or(textRunEnd));

      // Check if we need to emit (previously) collapsed whitespace at the start of this run.
      auto isAfterRangeEnd = rangeEnd ? runStart > *rangeEnd : false;
      auto hasPrecedingCollapsedWhitespace =
        _lastTextNodeEndedWithCollapsedSpace
        || (_textRun == firstTextRun && textRunStart == runStart && runStart);
      auto shouldEmitWhitespace =
        !isAfterRangeEnd && hasPrecedingCollapsedWhitespace && _lastCharacter
        && !renderer.Style().IsCollapsibleWhiteSpace(static_cast<char16_t>(_lastCharacter));
      if (shouldEmitWhitespace)
      {
        if (_lastTextNode.get() == textNode.get() && runStart
            && renderer.Style().IsCollapsibleWhiteSpace(rendererText[runStart - 1]))
        {
          uint32 spaceRunStart = runStart - 1;
          while (spaceRunStart && renderer.Style().IsCollapsibleWhiteSpace(rendererText[spaceRunStart - 1]))
          {
            --spaceRunStart;
          }
          EmitCharacter(' ', krys::move(textNode), nullptr, static_cast<int32>(spaceRunStart),
                        static_cast<int32>(spaceRunStart + 1));
        }
        else
        {
          EmitCharacter(' ', krys::move(textNode), nullptr, static_cast<int32>(runStart),
                        static_cast<int32>(runStart));
        }
        return;
      }

      // Determine what the next text run will be, but don't advance yet
      auto nextTextRun = inline_iterator::NextTextBoxInLogicalOrder(_textRun, _textRunLogicalOrderCache);
      if (runStart < runEnd)
      {
        auto isNewlineOrTab = [&](char8 character)
        {
          return character == '\n' || character == '\t';
        };
        // Handle either a single newline or tab character (which becomes a space),
        // or a run of characters that does not include newlines or tabs.
        // This effectively translates newlines and tabs to spaces without copying the text.
        if (isNewlineOrTab(rendererText[runStart]))
        {
          EmitCharacter(' ', textNode, nullptr, static_cast<int32>(runStart),
                        static_cast<int32>(runStart + 1));
          _offset = static_cast<int32>(runStart + 1);
        }
        else
        {
          auto subrunEnd = runStart + 1;
          for (; subrunEnd < runEnd; ++subrunEnd)
          {
            if (isNewlineOrTab(rendererText[subrunEnd]))
            {
              break;
            }
          }
          if (subrunEnd == runEnd && _behaviors.Contains(TextIteratorBehavior::BehavesAsIfNodesFollowing))
          {
            bool lastSpaceCollapsedByNextNonTextRun =
              !nextTextRun && rendererText.size() > subrunEnd && rendererText[subrunEnd] == ' ';
            if (lastSpaceCollapsedByNextNonTextRun)
            {
              ++subrunEnd; // runEnd stopped before last space. Increment by one to restore the space.
            }
          }
          _offset = static_cast<int32>(subrunEnd);
          EmitText(*textNode, renderer, static_cast<int32>(runStart), static_cast<int32>(subrunEnd));
        }

        // If we are doing a subrun that doesn't go to the end of the text box,
        // come back again to finish handling this text box; don't advance to the next one.
        if (static_cast<uint32>(_positionEndOffset) < textRunEnd)
        {
          return;
        }

        // Advance and return
        uint32 nextRunStart = nextTextRun ? nextTextRun->Start() : static_cast<uint32>(rendererText.size());
        if (nextRunStart > runEnd)
        {
          _lastTextNodeEndedWithCollapsedSpace = true; // collapsed space between runs or at the end
        }
        _textRun = nextTextRun;
        return;
      }
      // Advance and continue
      _textRun = nextTextRun;
    }
    if (!_textRun && _remainingTextRun)
    {
      RevertToRemainingTextRun();
      HandleTextRun();
    }
  }

  void TextIterator::RevertToRemainingTextRun() noexcept
  {
    krys_debug_assert(!_textRun && _remainingTextRun);

    _textRun = _remainingTextRun;
    _textRunLogicalOrderCache = std::exchange(_remainingTextRunLogicalOrderCache, {});
    _remainingTextRun = {};
    _firstLetterText = {};
    _offset = 0;
  }

  namespace
  {
    KRYS_NODISCARD inline RenderText *FirstRenderTextInFirstLetter(RenderBoxModelObject *firstLetter) noexcept
    {
      if (!firstLetter)
      {
        return nullptr;
      }

      // FIXME: Should this check descendent objects?
      for (auto *child = firstLetter->FirstChild(); child; child = child->NextSibling())
      {
        if (auto *renderText = DynamicDowncast<RenderText>(*child))
        {
          return renderText;
        }
      }
      return nullptr;
    }
  }

  void TextIterator::HandleTextNodeFirstLetter(RenderTextFragment &renderer) noexcept
  {
    if (auto *firstLetter = renderer.FirstLetter())
    {
      if (firstLetter->Style().GetVisibility() != Visibility::Visible
          && !_behaviors.Contains(TextIteratorBehavior::IgnoresStyleVisibility))
      {
        return;
      }
      if (auto *firstLetterText = FirstRenderTextInFirstLetter(firstLetter))
      {
        _handledFirstLetter = true;
        _remainingTextRun = _textRun;
        _remainingTextRunLogicalOrderCache = std::exchange(_textRunLogicalOrderCache, {});
        std::tie(_textRun, _textRunLogicalOrderCache) =
          inline_iterator::FirstTextBoxInLogicalOrderFor(*firstLetterText);
        _firstLetterText = CreateWeakPtr(firstLetterText);
      }
    }
    _handledFirstLetter = true;
  }

  bool TextIterator::HandleReplacedElement() noexcept
  {
    if (_fullyClippedStack.Top())
    {
      return false;
    }

    // Note that RenderInlines can get passed in as replaced elements.
    auto *renderer = DynamicDowncast<RenderElement>(_currentNode->Renderer());
    if (!renderer)
    {
      krys_debug_assert(false);
      return false;
    }

    if (renderer->Style().GetVisibility() != Visibility::Visible
        && !_behaviors.Contains(TextIteratorBehavior::IgnoresStyleVisibility))
    {
      return false;
    }

    if (_lastTextNodeEndedWithCollapsedSpace)
    {
      EmitCharacter(' ', ShareRefPtr<dom::Node>(_lastTextNode->ParentNode()),
                    RefPtr<dom::Node>(_lastTextNode), 1, 1);
      return false;
    }

    if (auto *renderTextControl = DynamicDowncast<RenderTextControl>(*renderer);
        renderTextControl && _behaviors.Contains(TextIteratorBehavior::EntersTextControls))
    {
      if (auto innerTextElement = renderTextControl->TextFormControlElement().InnerTextElement())
      {
        _currentNode = ShareRefPtr<dom::Node>(dom::TreeQueries::ContainingShadowRoot(*innerTextElement));
        PushFullyClippedState(_fullyClippedStack, *_currentNode, _behaviors);
        _offset = 0;
        return false;
      }
    }

    // FIXME: Entering image overlays (EntersImageOverlays), once there are any.

    _hasEmitted = true;

    auto shouldEmitObjectReplacementCharacter = [&]
    {
      if (_behaviors.Contains(TextIteratorBehavior::EmitsObjectReplacementCharacters))
      {
        return true;
      }

      if (_behaviors.Contains(TextIteratorBehavior::EmitsObjectReplacementCharactersForImages)
          && IsHTMLElementOfType(*_currentNode, html::TagName::img))
      {
        return true;
      }

      return false;
    }();

    if (shouldEmitObjectReplacementCharacter)
    {
      EmitCharacter(ObjectReplacementCharacter, ShareRefPtr<dom::Node>(_currentNode->ParentNode()),
                    RefPtr<dom::Node>(_currentNode), 0, 1);
      // Don't process subtrees for embedded objects. If the text there is required,
      // it must be explicitly asked by specifying a range falling inside its boundaries.
      _handledChildren = true;
      return true;
    }

    if (_behaviors.Contains(TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions))
    {
      // We want replaced elements to behave like punctuation for boundary
      // finding, and to simply take up space for the selection preservation
      // code in moveParagraphs, so we use a comma.
      EmitCharacter(',', ShareRefPtr<dom::Node>(_currentNode->ParentNode()), RefPtr<dom::Node>(_currentNode),
                    0, 1);
      return true;
    }

    _positionNode = ShareRefPtr<dom::Node>(_currentNode->ParentNode());
    _positionOffsetBaseNode = _currentNode;
    _positionStartOffset = 0;
    _positionEndOffset = 1;

    if (auto *renderImage = DynamicDowncast<RenderImage>(*renderer);
        renderImage && _behaviors.Contains(TextIteratorBehavior::EmitsImageAltText))
    {
      auto altText = utf8_string(renderImage->AltText());
      if (!altText.empty())
      {
        _lastCharacter = text::LastCodePoint(altText);
        _copyableText.Set(krys::move(altText));
        _text = _copyableText.Text();
        return true;
      }
    }

    _copyableText.Reset();
    _text = utf8_stringview();
    _lastCharacter = 0;
    return true;
  }

  namespace
  {
    KRYS_NODISCARD bool ShouldEmitTabBeforeNode(dom::Node &node) noexcept
    {
      auto *cell = DynamicDowncast<RenderTableCell>(node.Renderer());

      // Table cells are delimited by tabs.
      if (!cell)
      {
        return false;
      }

      // Want a tab before every cell other than the first one.
      auto *table = cell->Table();
      return table && (table->CellBefore(cell) || table->CellAbove(cell));
    }

    KRYS_NODISCARD bool ShouldEmitNewlineForNode(dom::Node *node, bool emitsOriginalText) noexcept
    {
      auto *renderer = node->Renderer();
      if (!(renderer ? renderer->IsBR() : IsHTMLElementOfType(*node, html::TagName::br)))
      {
        return false;
      }
      auto *shadowHost = dom::TreeQueries::ShadowHost(*node);
      return emitsOriginalText
             || !(node->IsInShadowTree() && shadowHost
                  && IsHTMLElementOfType(*shadowHost, html::TagName::input));
    }

    KRYS_NODISCARD bool HasHeaderTag(const dom::Node &element) noexcept
    {
      return IsHTMLElementOfType(element, html::TagName::h1)
             || IsHTMLElementOfType(element, html::TagName::h2)
             || IsHTMLElementOfType(element, html::TagName::h3)
             || IsHTMLElementOfType(element, html::TagName::h4)
             || IsHTMLElementOfType(element, html::TagName::h5)
             || IsHTMLElementOfType(element, html::TagName::h6);
    }

    KRYS_NODISCARD bool ShouldEmitReplacementInsteadOfNode(const dom::Node &) noexcept
    {
      // Placeholders should eventually disappear, so treating them as a line break doesn't make sense
      // as when they are removed the text after it is combined with the text before it.
      // FIXME: Text placeholder elements, once there are any.
      return false;
    }
  }

  bool ShouldEmitNewlinesBeforeAndAfterNode(dom::Node &node) noexcept
  {
    // Block flow (versus inline flow) is represented by having
    // a newline both before and after the element.
    auto *renderer = node.Renderer();
    if (!renderer)
    {
      if (HasDisplayContents(node))
      {
        return false;
      }
      return HasHeaderTag(node) || IsHTMLElementOfType(node, html::TagName::blockquote)
             || IsHTMLElementOfType(node, html::TagName::dd) || IsHTMLElementOfType(node, html::TagName::div)
             || IsHTMLElementOfType(node, html::TagName::dl) || IsHTMLElementOfType(node, html::TagName::dt)
             || IsHTMLElementOfType(node, html::TagName::hr) || IsHTMLElementOfType(node, html::TagName::li)
             || IsHTMLElementOfType(node, html::TagName::listing)
             || IsHTMLElementOfType(node, html::TagName::ol) || IsHTMLElementOfType(node, html::TagName::p)
             || IsHTMLElementOfType(node, html::TagName::pre) || IsHTMLElementOfType(node, html::TagName::tr)
             || IsHTMLElementOfType(node, html::TagName::ul);
    }

    // Need to make an exception for table cells, because they are blocks, but we
    // want them tab-delimited rather than having newlines before and after.
    if (IsTableCell(node))
    {
      return false;
    }

    // Need to make an exception for table row elements, because they are neither
    // "inline" or "RenderBlock", but we want newlines for them.
    if (auto *tableRow = DynamicDowncast<RenderTableRow>(*renderer))
    {
      auto *table = tableRow->Table();
      if (table && !table->IsInline())
      {
        return true;
      }
    }

    if (ShouldEmitReplacementInsteadOfNode(node))
    {
      return false;
    }

    return !renderer->IsInline() && Is<RenderBlock>(*renderer) && !renderer->IsFloatingOrOutOfFlowPositioned()
           && !renderer->IsBody();
  }

  namespace
  {
    KRYS_NODISCARD bool
      ShouldEmitNewlineAfterNode(dom::Node &node,
                                 bool emitsCharactersBetweenAllVisiblePositions = false) noexcept
    {
      // FIXME: It should be better but slower to create a VisiblePosition here.
      if (!ShouldEmitNewlinesBeforeAndAfterNode(node))
      {
        return false;
      }

      // Don't emit a new line at the end of the document unless we're matching the behavior of
      // VisiblePosition.
      if (emitsCharactersBetweenAllVisiblePositions)
      {
        return true;
      }
      dom::Node *subsequentNode = &node;
      while ((subsequentNode = dom::TreeQueries::NextSkippingChildren(*subsequentNode)))
      {
        if (subsequentNode->Renderer())
        {
          return true;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool ShouldEmitNewlineBeforeNode(dom::Node &node) noexcept
    {
      return ShouldEmitNewlinesBeforeAndAfterNode(node);
    }

    KRYS_NODISCARD bool ShouldEmitExtraNewlineForNode(dom::Node &node) noexcept
    {
      // When there is a significant collapsed bottom margin, emit an extra
      // newline for a more realistic result. We end up getting the right
      // result even without margin collapsing. For example: <div><p>text</p></div>
      // will work right even if both the <div> and the <p> have bottom margins.

      auto *renderBox = DynamicDowncast<RenderBox>(node.Renderer());
      if (!renderBox || !renderBox->Height())
      {
        return false;
      }

      // NOTE: We only do this for a select set of nodes, and WinIE appears not to do this at all.
      if (!HasHeaderTag(node) && !IsHTMLElementOfType(node, html::TagName::p))
      {
        return false;
      }

      auto bottomMargin = renderBox->CollapsedMarginAfter();
      auto fontSize = renderBox->Style().GetFontDescription().ComputedSize();
      return bottomMargin * 2 >= fontSize;
    }

    KRYS_NODISCARD int32 CollapsedSpaceLength(const RenderText &renderer, int32 textEnd) noexcept
    {
      const auto &text = renderer.Text();
      uint32 length = static_cast<uint32>(text.size());
      for (uint32 i = static_cast<uint32>(textEnd); i < length; ++i)
      {
        if (!renderer.Style().IsCollapsibleWhiteSpace(text[i]))
        {
          return static_cast<int32>(i) - textEnd;
        }
      }
      return static_cast<int32>(length) - textEnd;
    }

    KRYS_NODISCARD int32 MaxOffsetIncludingCollapsedSpaces(dom::Node &node) noexcept
    {
      int32 offset = CaretMaxOffset(node);
      if (auto *renderText = DynamicDowncast<RenderText>(node.Renderer()))
      {
        offset += CollapsedSpaceLength(*renderText, offset);
      }
      return offset;
    }
  }

  // Whether or not we should emit a character as we enter m_currentNode (if it's a container) or as we hit it
  // (if it's atomic).
  bool TextIterator::ShouldRepresentNodeOffsetZero() noexcept
  {
    if (_behaviors.Contains(TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions))
    {
      if (auto *renderer = _currentNode->Renderer(); renderer && renderer->IsRenderTable())
      {
        return true;
      }
    }

    // Leave element positioned flush with start of a paragraph
    // (e.g. do not insert tab before a table cell at the start of a paragraph)
    if (_lastCharacter == '\n')
    {
      return false;
    }

    // Otherwise, show the position if we have emitted any characters
    if (_hasEmitted)
    {
      return true;
    }

    // We've not emitted anything yet. Generally, there is no need for any positioning then.
    // The only exception is when the element is visually not in the same line as
    // the start of the range (e.g. the range starts at the end of the previous paragraph).
    // NOTE: Creating VisiblePositions and comparing them is relatively expensive, so we
    // make quicker checks to possibly avoid that. Another check that we could make is
    // is whether the inline vs block flow changed since the previous visible element.
    // I think we're already in a special enough case that that won't be needed, tho.

    // No character needed if this is the first node in the range.
    if (_currentNode == _startContainer)
    {
      return false;
    }

    // If we are outside the start container's subtree, assume we need to emit.
    // FIXME: m_startContainer could be an inline block
    auto &currentNode = *_currentNode;
    if (!currentNode.IsDescendantOf(_startContainer.get()))
    {
      return true;
    }

    // If we started as m_startContainer offset 0 and the current node is a descendant of
    // the start container, we already had enough context to correctly decide whether to
    // emit after a preceding block. We chose not to emit (m_hasEmitted is false),
    // so don't second guess that now.
    // NOTE: Is this really correct when m_currentNode is not a leftmost descendant? Probably
    // immaterial since we likely would have already emitted something by now.
    if (_startOffset == 0)
    {
      return false;
    }

    // If this node is unrendered or invisible the VisiblePosition checks below won't have much meaning.
    // Additionally, if the range we are iterating over contains huge sections of unrendered content,
    // we would create VisiblePositions on every call to this function without this check.
    if (!currentNode.Renderer() || currentNode.Renderer()->Style().GetVisibility() != Visibility::Visible)
    {
      return false;
    }

    if (auto *renderBlockFlow = DynamicDowncast<RenderBlockFlow>(*currentNode.Renderer()))
    {
      if (!renderBlockFlow->Height() && !IsHTMLElementOfType(currentNode, html::TagName::body))
      {
        return false;
      }
    }

    // The startPos.isNotNull() check is needed because the start could be before the body,
    // and in that case we'll get null. We don't want to put in newlines at the start in that case.
    // The currPos.isNotNull() check is needed because positions in non-HTML content
    // (like SVG) do not have visible positions, and we don't want to emit for them either.
    VisiblePosition startPos =
      VisiblePosition(dom::Position(RefPtr<dom::Node>(_startContainer), static_cast<uint32>(_startOffset),
                                    dom::Position::AnchorType::PositionIsOffsetInAnchor));
    VisiblePosition currPos = VisiblePosition(dom::PositionBeforeNode(&currentNode));
    return startPos.IsNotNull() && currPos.IsNotNull() && !InSameLine(startPos, currPos);
  }

  bool TextIterator::ShouldEmitSpaceBeforeAndAfterNode(dom::Node &node) noexcept
  {
    return node.Renderer() && node.Renderer()->IsRenderTable()
           && (node.Renderer()->IsInline()
               || _behaviors.Contains(TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions));
  }

  void TextIterator::RepresentNodeOffsetZero() noexcept
  {
    // Emit a character to show the positioning of m_currentNode.

    // When we haven't been emitting any characters, shouldRepresentNodeOffsetZero() can
    // create VisiblePositions, which is expensive. So, we perform the inexpensive checks
    // on m_currentNode to see if it necessitates emitting a character first and will early return
    // before encountering shouldRepresentNodeOffsetZero()s worse case behavior.
    RefPtr<dom::Node> currentNode = _currentNode;
    if (ShouldEmitTabBeforeNode(*currentNode))
    {
      if (ShouldRepresentNodeOffsetZero())
      {
        auto parentNode = ShareRefPtr<dom::Node>(currentNode->ParentNode());
        EmitCharacter('\t', krys::move(parentNode), krys::move(currentNode), 0, 0);
      }
    }
    else if (ShouldEmitNewlineBeforeNode(*currentNode))
    {
      if (ShouldRepresentNodeOffsetZero())
      {
        auto parentNode = ShareRefPtr<dom::Node>(currentNode->ParentNode());
        EmitCharacter('\n', krys::move(parentNode), krys::move(currentNode), 0, 0);
      }
    }
    else if (ShouldEmitSpaceBeforeAndAfterNode(*currentNode))
    {
      if (ShouldRepresentNodeOffsetZero())
      {
        auto parentNode = ShareRefPtr<dom::Node>(currentNode->ParentNode());
        EmitCharacter(' ', krys::move(parentNode), krys::move(currentNode), 0, 0);
      }
    }
    else if (ShouldEmitReplacementInsteadOfNode(*currentNode))
    {
      if (ShouldRepresentNodeOffsetZero())
      {
        auto parentNode = ShareRefPtr<dom::Node>(currentNode->ParentNode());
        EmitCharacter(ObjectReplacementCharacter, krys::move(parentNode), krys::move(currentNode), 0, 0);
      }
    }
  }

  bool TextIterator::HandleNonTextNode() noexcept
  {
    RefPtr<dom::Node> currentNode = _currentNode;
    if (ShouldEmitNewlineForNode(currentNode.get(),
                                 _behaviors.Contains(TextIteratorBehavior::EmitsOriginalText)))
    {
      auto parentNode = ShareRefPtr<dom::Node>(currentNode->ParentNode());
      EmitCharacter('\n', krys::move(parentNode), krys::move(currentNode), 0, 1);
    }
    else if (_behaviors.Contains(TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions)
             && currentNode->Renderer() && currentNode->Renderer()->IsHR())
    {
      auto parentNode = ShareRefPtr<dom::Node>(currentNode->ParentNode());
      EmitCharacter(' ', krys::move(parentNode), krys::move(currentNode), 0, 1);
    }
    else
    {
      RepresentNodeOffsetZero();
    }

    return true;
  }

  void TextIterator::ExitNode(dom::Node *exitedNode) noexcept
  {
    // prevent emitting a newline when exiting a collapsed block at beginning of the range
    // FIXME: !m_hasEmitted does not necessarily mean there was a collapsed block... it could
    // have been an hr (e.g.). Also, a collapsed block could have height (e.g. a table) and
    // therefore look like a blank line.
    if (!_hasEmitted)
    {
      return;
    }

    // Emit with a position *inside* m_currentNode, after m_currentNode's contents, in
    // case it is a block, because the run should start where the
    // emitted character is positioned visually.
    RefPtr<dom::Node> baseNode = ShareRefPtr(exitedNode);
    // FIXME: This shouldn't require the m_lastTextNode to be true, but we can't change that without making
    // the logic in _web_attributedStringFromRange match. We'll get that for free when we switch to use
    // TextIterator in _web_attributedStringFromRange.
    // See <rdar://problem/5428427> for an example of how this mismatch will cause problems.
    if (_lastTextNode
        && ShouldEmitNewlineAfterNode(
          *_currentNode,
          _behaviors.Contains(TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions)))
    {
      // use extra newline to represent margin bottom, as needed
      bool addNewline = ShouldEmitExtraNewlineForNode(*_currentNode);

      // FIXME: We need to emit a '\n' as we leave an empty block(s) that
      // contain a VisiblePosition when doing selection preservation.
      if (_lastCharacter != '\n')
      {
        // insert a newline with a position following this block's contents.
        EmitCharacter('\n', ShareRefPtr<dom::Node>(baseNode->ParentNode()), RefPtr<dom::Node>(baseNode), 1,
                      1);
        // remember whether to later add a newline for the current node
        krys_debug_assert(!_nodeForAdditionalNewline);
        if (addNewline)
        {
          _nodeForAdditionalNewline = baseNode;
        }
      }
      else if (addNewline)
      {
        // insert a newline with a position following this block's contents.
        EmitCharacter('\n', ShareRefPtr<dom::Node>(baseNode->ParentNode()), RefPtr<dom::Node>(baseNode), 1,
                      1);
      }
    }

    // If nothing was emitted, see if we need to emit a space.
    if (!_positionNode && ShouldEmitSpaceBeforeAndAfterNode(*_currentNode))
    {
      auto parentNode = ShareRefPtr<dom::Node>(baseNode->ParentNode());
      EmitCharacter(' ', krys::move(parentNode), krys::move(baseNode), 1, 1);
    }
  }

  void TextIterator::EmitCharacter(char32 character, RefPtr<dom::Node> &&characterNode,
                                   RefPtr<dom::Node> &&offsetBaseNode, int32 textStartOffset,
                                   int32 textEndOffset) noexcept
  {
    krys_debug_assert(characterNode);
    _hasEmitted = true;

    // remember information with which to construct the TextIterator::range()
    _positionNode = krys::move(characterNode);
    _positionOffsetBaseNode = krys::move(offsetBaseNode);
    _positionStartOffset = textStartOffset;
    _positionEndOffset = textEndOffset;

    _copyableText.Set(character);
    _text = _copyableText.Text();
    _lastCharacter = character;
    _lastTextNodeEndedWithCollapsedSpace = false;
  }

  void TextIterator::EmitText(dom::Text &textNode, const RenderText &renderer, int32 textStartOffset,
                              int32 textEndOffset) noexcept
  {
    krys_debug_assert(textStartOffset >= 0);
    krys_debug_assert(textEndOffset >= 0);
    krys_debug_assert(textStartOffset <= textEndOffset);

    // FIXME: Ignoring full size kana (IgnoresFullSizeKana), once accessibility asks for it.

    // FIXME: This probably yields the wrong offsets when text-transform: lowercase turns a single character
    // into two characters.
    utf8_string string = _behaviors.Contains(TextIteratorBehavior::EmitsOriginalText)
                           ? renderer.OriginalText()
                           : (_behaviors.Contains(TextIteratorBehavior::EmitsTextsWithoutTranscoding)
                                ? renderer.TextWithoutConvertingBackslashToYenSymbol()
                                : renderer.Text());

    krys_debug_assert(_behaviors.Contains(TextIteratorBehavior::EmitsOriginalText)
                      || string.size() >= static_cast<size_t>(textEndOffset));

    textEndOffset = std::min(static_cast<int32>(string.size()), textEndOffset);

    _positionNode = ShareRefPtr<dom::Node>(&textNode);
    _positionOffsetBaseNode = nullptr;
    _positionStartOffset = textStartOffset;
    _positionEndOffset = textEndOffset;

    _lastCharacter =
      text::LastCodePoint(utf8_stringview(string).substr(0, static_cast<size_t>(textEndOffset)));
    _copyableText.Set(krys::move(string), static_cast<uint32>(textStartOffset),
                      static_cast<uint32>(textEndOffset - textStartOffset));
    _text = _copyableText.Text();

    _lastTextNodeEndedWithCollapsedSpace = false;
    _hasEmitted = true;
  }

  dom::SimpleRange TextIterator::Range() const noexcept
  {
    krys_debug_assert(!AtEnd());
    // Use the current run information, if we have it.
    if (_positionOffsetBaseNode)
    {
      uint32 index = _positionOffsetBaseNode->ComputeNodeIndex();
      _positionStartOffset += static_cast<int32>(index);
      _positionEndOffset += static_cast<int32>(index);
      _positionOffsetBaseNode = nullptr;
    }
    return {{ShareRef(*_positionNode), static_cast<uint64>(_positionStartOffset)},
            {ShareRef(*_positionNode), static_cast<uint64>(_positionEndOffset)}};
  }

  dom::Node *TextIterator::GetNode() const noexcept
  {
    auto start = Range().Start;
    if (Is<dom::CharacterData>(*start.Container))
    {
      return start.Container.get();
    }
    return start.Container->TraverseToChildAt(static_cast<uint32>(start.Offset));
  }

  // --------

  SimplifiedBackwardsTextIterator::SimplifiedBackwardsTextIterator(const dom::SimpleRange &range) noexcept
  {
    range.Start.GetDocument().UpdateLayoutIgnorePendingStylesheets();

    dom::Node *startNode = range.Start.Container.get();
    dom::Node *endNode = range.End.Container.get();
    uint32 startOffset = static_cast<uint32>(range.Start.Offset);
    uint32 endOffset = static_cast<uint32>(range.End.Offset);

    if (!Is<dom::CharacterData>(*startNode))
    {
      if (startOffset < startNode->CountChildNodes())
      {
        startNode = startNode->TraverseToChildAt(startOffset);
        startOffset = 0;
      }
    }
    if (!Is<dom::CharacterData>(*endNode))
    {
      if (endOffset > 0 && endOffset <= endNode->CountChildNodes())
      {
        endNode = endNode->TraverseToChildAt(endOffset - 1);
        endOffset = endNode->Length();
      }
    }

    _node = ShareRefPtr(endNode);
    SetUpFullyClippedStack(_fullyClippedStack, *_node, _behaviors);
    _offset = static_cast<int32>(endOffset);
    _handledNode = false;
    _handledChildren = endOffset == 0;

    _startContainer = ShareRefPtr(startNode);
    _startOffset = static_cast<int32>(startOffset);
    _endContainer = ShareRefPtr(endNode);
    _endOffset = static_cast<int32>(endOffset);

    _positionNode = ShareRefPtr(endNode);

    _lastTextNode = nullptr;
    _lastCharacter = '\n';

    _havePassedStartContainer = false;

    Advance();
  }

  void SimplifiedBackwardsTextIterator::Advance() noexcept
  {
    krys_debug_assert(!AtEnd());

    _positionNode = nullptr;
    _copyableText.Reset();
    _text = utf8_stringview();

    while (_node && !_havePassedStartContainer)
    {
      // Don't handle node if we start iterating at [node, 0].
      if (!_handledNode && !(_node == _endContainer && !_endOffset))
      {
        auto *renderer = _node->Renderer();
        if (auto *renderText = DynamicDowncast<RenderText>(renderer))
        {
          if (renderText->Style().GetVisibility() == Visibility::Visible && _offset > 0)
          {
            _handledNode = HandleTextNode();
          }
        }
        else if (IsRendererReplacedElement(renderer, _behaviors))
        {
          if (Downcast<RenderElement>(*renderer).Style().GetVisibility() == Visibility::Visible
              && _offset > 0)
          {
            _handledNode = HandleReplacedElement();
          }
        }
        else
        {
          _handledNode = HandleNonTextNode();
        }
        if (_positionNode)
        {
          return;
        }
      }

      if (!_handledChildren && _node->HasChildNodes())
      {
        _node = ShareRefPtr(_node->LastChild());
        PushFullyClippedState(_fullyClippedStack, *_node, _behaviors);
      }
      else
      {
        // Exit empty containers as we pass over them or containers
        // where [container, 0] is where we started iterating.
        if (!_handledNode && CanHaveChildrenForEditing(*_node) && _node->ParentNode()
            && (!_node->LastChild() || (_node == _endContainer && !_endOffset)))
        {
          ExitNode();
          if (_positionNode)
          {
            _handledNode = true;
            _handledChildren = true;
            return;
          }
        }

        // Exit all other containers.
        while (!_node->PreviousSibling())
        {
          if (!AdvanceRespectingRange(_node->ParentOrShadowHostNode()))
          {
            break;
          }
          _fullyClippedStack.Pop();
          ExitNode();
          if (_positionNode)
          {
            _handledNode = true;
            _handledChildren = true;
            return;
          }
        }

        _fullyClippedStack.Pop();
        if (AdvanceRespectingRange(_node->PreviousSibling()))
        {
          PushFullyClippedState(_fullyClippedStack, *_node, _behaviors);
        }
        else
        {
          _node = nullptr;
        }
      }

      // For the purpose of word boundary detection,
      // we should iterate all visible text and trailing (collapsed) whitespaces.
      _offset = _node ? MaxOffsetIncludingCollapsedSpaces(*_node) : 0;
      _handledNode = false;
      _handledChildren = false;

      if (_positionNode)
      {
        return;
      }
    }
  }

  bool SimplifiedBackwardsTextIterator::HandleTextNode() noexcept
  {
    _lastTextNode = ShareRefPtr(&Downcast<dom::Text>(*_node));

    int32 startOffset;
    int32 offsetInNode;
    auto *renderer = HandleFirstLetter(startOffset, offsetInNode);
    if (!renderer)
    {
      return true;
    }

    utf8_string text = renderer->Text();
    if (!renderer->HasRenderedText() && text.size())
    {
      return true;
    }

    if (startOffset + offsetInNode == _offset)
    {
      krys_debug_assert(!_shouldHandleFirstLetter);
      return true;
    }

    _positionEndOffset = _offset;
    _offset = startOffset + offsetInNode;
    _positionNode = _node;
    _positionStartOffset = _offset;

    krys_debug_assert(_positionStartOffset < _positionEndOffset);
    krys_debug_assert(_positionStartOffset - offsetInNode >= 0);
    krys_debug_assert(_positionEndOffset - offsetInNode > 0);
    krys_debug_assert(_positionEndOffset - offsetInNode <= static_cast<int32>(text.size()));

    _lastCharacter = text::LastCodePoint(
      utf8_stringview(text).substr(0, static_cast<size_t>(_positionEndOffset - offsetInNode)));
    _copyableText.Set(krys::move(text), static_cast<uint32>(_positionStartOffset - offsetInNode),
                      static_cast<uint32>(_positionEndOffset - _positionStartOffset));
    _text = _copyableText.Text();

    return !_shouldHandleFirstLetter;
  }

  const RenderText *SimplifiedBackwardsTextIterator::HandleFirstLetter(int32 &startOffset,
                                                                       int32 &offsetInNode) noexcept
  {
    auto &renderer = Downcast<RenderText>(*_node->Renderer());
    startOffset = (_node == _startContainer) ? _startOffset : 0;

    auto *fragment = DynamicDowncast<RenderTextFragment>(renderer);
    if (!fragment)
    {
      offsetInNode = 0;
      return &renderer;
    }

    int32 offsetAfterFirstLetter = static_cast<int32>(fragment->Start());
    if (startOffset >= offsetAfterFirstLetter)
    {
      krys_debug_assert(!_shouldHandleFirstLetter);
      offsetInNode = offsetAfterFirstLetter;
      return &renderer;
    }

    if (!_shouldHandleFirstLetter && startOffset + offsetAfterFirstLetter < _offset)
    {
      _shouldHandleFirstLetter = true;
      offsetInNode = offsetAfterFirstLetter;
      return &renderer;
    }

    _shouldHandleFirstLetter = false;
    offsetInNode = 0;
    auto *firstLetterRenderer = FirstRenderTextInFirstLetter(fragment->FirstLetter());

    _offset = firstLetterRenderer->CaretMaxOffset();
    _offset += CollapsedSpaceLength(*firstLetterRenderer, _offset);

    return firstLetterRenderer;
  }

  bool SimplifiedBackwardsTextIterator::HandleReplacedElement() noexcept
  {
    int32 index = static_cast<int32>(_node->ComputeNodeIndex());
    // We want replaced elements to behave like punctuation for boundary
    // finding, and to simply take up space for the selection preservation
    // code in moveParagraphs, so we use a comma. Unconditionally emit
    // here because this iterator is only used for boundary finding.
    EmitCharacter(',', ShareRefPtr<dom::Node>(_node->ParentNode()), index, index + 1);
    return true;
  }

  bool SimplifiedBackwardsTextIterator::HandleNonTextNode() noexcept
  {
    RefPtr<dom::Node> currentNode = _node;
    if (ShouldEmitTabBeforeNode(*currentNode))
    {
      int32 index = static_cast<int32>(currentNode->ComputeNodeIndex());
      EmitCharacter('\t', ShareRefPtr<dom::Node>(currentNode->ParentNode()), index + 1, index + 1);
    }
    else if (ShouldEmitNewlineForNode(currentNode.get(),
                                      _behaviors.Contains(TextIteratorBehavior::EmitsOriginalText))
             || ShouldEmitNewlineAfterNode(*_node))
    {
      if (_lastCharacter != '\n')
      {
        // Corresponds to the same check in TextIterator::exitNode.
        int32 index = static_cast<int32>(currentNode->ComputeNodeIndex());
        // The start of this emitted range is wrong. Ensuring correctness would require
        // VisiblePositions and so would be slow. previousBoundary expects this.
        EmitCharacter('\n', ShareRefPtr<dom::Node>(currentNode->ParentNode()), index + 1, index + 1);
      }
    }
    return true;
  }

  void SimplifiedBackwardsTextIterator::ExitNode() noexcept
  {
    RefPtr<dom::Node> node = _node;
    if (ShouldEmitTabBeforeNode(*node))
    {
      EmitCharacter('\t', krys::move(node), 0, 0);
    }
    else if (ShouldEmitNewlineForNode(node.get(),
                                      _behaviors.Contains(TextIteratorBehavior::EmitsOriginalText))
             || ShouldEmitNewlineBeforeNode(*_node))
    {
      // The start of this emitted range is wrong. Ensuring correctness would require
      // VisiblePositions and so would be slow. previousBoundary expects this.
      EmitCharacter('\n', krys::move(node), 0, 0);
    }
  }

  void SimplifiedBackwardsTextIterator::EmitCharacter(char32 character, RefPtr<dom::Node> &&node,
                                                      int32 startOffset, int32 endOffset) noexcept
  {
    krys_debug_assert(node);
    _positionNode = krys::move(node);
    _positionStartOffset = startOffset;
    _positionEndOffset = endOffset;
    _copyableText.Set(character);
    _text = _copyableText.Text();
    _lastCharacter = character;
  }

  bool SimplifiedBackwardsTextIterator::AdvanceRespectingRange(dom::Node *next) noexcept
  {
    if (!next)
    {
      return false;
    }
    _havePassedStartContainer |= _node == _startContainer;
    if (_havePassedStartContainer)
    {
      return false;
    }
    _node = ShareRefPtr(next);
    return true;
  }

  dom::SimpleRange SimplifiedBackwardsTextIterator::Range() const noexcept
  {
    krys_debug_assert(!AtEnd());

    return {{ShareRef(*_positionNode), static_cast<uint64>(_positionStartOffset)},
            {ShareRef(*_positionNode), static_cast<uint64>(_positionEndOffset)}};
  }

  // --------

  CharacterIterator::CharacterIterator(const dom::SimpleRange &range,
                                       TextIteratorBehaviors behaviors) noexcept
      : _underlyingIterator(range, behaviors)
  {
    while (!AtEnd() && !_underlyingIterator.Text().size())
    {
      _underlyingIterator.Advance();
    }
  }

  dom::SimpleRange CharacterIterator::Range() const noexcept
  {
    dom::SimpleRange range = _underlyingIterator.Range();
    if (!_underlyingIterator.AtEnd())
    {
      if (_underlyingIterator.Text().size() <= 1)
      {
        krys_debug_assert(_runOffset == 0);
      }
      else
      {
        uint64 offset = range.StartOffset() + _runOffset;
        range = {{range.Start.Container, offset}, {range.Start.Container, offset + 1}};
      }
    }
    return range;
  }

  void CharacterIterator::Advance(int32 count) noexcept
  {
    if (count <= 0)
    {
      krys_debug_assert(count == 0);
      return;
    }

    _atBreak = false;

    // easy if there is enough left in the current m_underlyingIterator run
    int32 remaining = static_cast<int32>(_underlyingIterator.Text().size()) - static_cast<int32>(_runOffset);
    if (count < remaining)
    {
      _runOffset += static_cast<uint32>(count);
      _offset += static_cast<uint32>(count);
      return;
    }

    // exhaust the current m_underlyingIterator run
    count -= remaining;
    _offset += static_cast<uint32>(remaining);

    // move to a subsequent m_underlyingIterator run
    for (_underlyingIterator.Advance(); !AtEnd(); _underlyingIterator.Advance())
    {
      int32 runLength = static_cast<int32>(_underlyingIterator.Text().size());
      if (!runLength)
      {
        _atBreak = true;
      }
      else
      {
        // see whether this is m_underlyingIterator to use
        if (count < runLength)
        {
          _runOffset = static_cast<uint32>(count);
          _offset += static_cast<uint32>(count);
          return;
        }

        // exhaust this m_underlyingIterator run
        count -= runLength;
        _offset += static_cast<uint32>(runLength);
      }
    }

    // ran to the end of the m_underlyingIterator... no more runs left
    _atBreak = true;
    _runOffset = 0;
  }

  BackwardsCharacterIterator::BackwardsCharacterIterator(const dom::SimpleRange &range) noexcept
      : _underlyingIterator(range)
  {
    while (!AtEnd() && !_underlyingIterator.Text().size())
    {
      _underlyingIterator.Advance();
    }
  }

  dom::SimpleRange BackwardsCharacterIterator::Range() const noexcept
  {
    auto range = _underlyingIterator.Range();
    if (!_underlyingIterator.AtEnd())
    {
      if (_underlyingIterator.Text().size() <= 1)
      {
        krys_debug_assert(_runOffset == 0);
      }
      else
      {
        uint64 offset = range.End.Offset - _runOffset;
        range = {{range.Start.Container, offset - 1}, {range.Start.Container, offset}};
      }
    }
    return range;
  }

  void BackwardsCharacterIterator::Advance(int32 count) noexcept
  {
    if (count <= 0)
    {
      krys_debug_assert(!count);
      return;
    }

    _atBreak = false;

    int32 remaining = static_cast<int32>(_underlyingIterator.Text().size()) - static_cast<int32>(_runOffset);
    if (count < remaining)
    {
      _runOffset += static_cast<uint32>(count);
      _offset += static_cast<uint32>(count);
      return;
    }

    count -= remaining;
    _offset += static_cast<uint32>(remaining);

    for (_underlyingIterator.Advance(); !AtEnd(); _underlyingIterator.Advance())
    {
      int32 runLength = static_cast<int32>(_underlyingIterator.Text().size());
      if (runLength == 0)
      {
        _atBreak = true;
      }
      else
      {
        if (count < runLength)
        {
          _runOffset = static_cast<uint32>(count);
          _offset += static_cast<uint32>(count);
          return;
        }

        count -= runLength;
        _offset += static_cast<uint32>(runLength);
      }
    }

    _atBreak = true;
    _runOffset = 0;
  }

  // --------

  WordAwareIterator::WordAwareIterator(const dom::SimpleRange &range) noexcept : _underlyingIterator(range)
  {
    Advance(); // get in position over the first chunk of text
  }

  namespace
  {
    KRYS_NODISCARD constexpr bool DeprecatedIsSpaceOrNewline(char8 c) noexcept
    {
      // Use isASCIISpace() for basic Latin-1.
      // This will include newlines, which aren't included in Unicode DirWS.
      return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
    }
  }

  // We're always in one of these modes:
  // - The current chunk in the text iterator is our current chunk
  //      (typically its a piece of whitespace, or text that ended with whitespace)
  // - The previous chunk in the text iterator is our current chunk
  //      (we looked ahead to the next chunk and found a word boundary)
  // - We built up our own chunk of text from many chunks from the text iterator

  // FIXME: Performance could be bad for huge spans next to each other that don't fall on word boundaries.

  void WordAwareIterator::Advance() noexcept
  {
    _previousText.Reset();
    _buffer.clear();

    // If last time we did a look-ahead, start with that looked-ahead chunk now
    if (!_didLookAhead)
    {
      krys_debug_assert(!_underlyingIterator.AtEnd());
      _underlyingIterator.Advance();
    }
    _didLookAhead = false;

    // Go to next non-empty chunk
    while (!_underlyingIterator.AtEnd() && !_underlyingIterator.Text().size())
    {
      _underlyingIterator.Advance();
    }
    if (_underlyingIterator.AtEnd())
    {
      return;
    }

    while (true)
    {
      // If this chunk ends in whitespace we can just use it as our chunk.
      if (DeprecatedIsSpaceOrNewline(_underlyingIterator.Text().back()))
      {
        return;
      }

      // If this is the first chunk that failed, save it in previousText before look ahead
      if (_buffer.empty())
      {
        _previousText = _underlyingIterator.CopyableText();
      }

      // Look ahead to next chunk. If it is whitespace or a break, we can use the previous stuff
      _underlyingIterator.Advance();
      if (_underlyingIterator.AtEnd() || !_underlyingIterator.Text().size()
          || DeprecatedIsSpaceOrNewline(_underlyingIterator.Text()[0]))
      {
        _didLookAhead = true;
        return;
      }

      if (_buffer.empty())
      {
        // Start gobbling chunks until we get to a suitable stopping point.
        _buffer.append(_previousText.Text());
        _previousText.Reset();
      }
      _buffer.append(_underlyingIterator.Text());
    }
  }

  utf8_stringview WordAwareIterator::Text() const noexcept
  {
    if (!_buffer.empty())
    {
      return _buffer;
    }
    if (_previousText.Text().size())
    {
      return _previousText.Text();
    }
    return _underlyingIterator.Text();
  }

  // --------

  uint64 CharacterCount(const dom::SimpleRange &range, TextIteratorBehaviors behaviors) noexcept
  {
    auto adjustedRange = range;
    auto ordering = dom::TreeOrder<dom::TreeType::ComposedTree>(range.Start, range.End);
    if (std::is_gt(ordering))
    {
      std::swap(adjustedRange.Start, adjustedRange.End);
    }
    else if (!std::is_lt(ordering))
    {
      return 0;
    }
    uint64 length = 0;
    for (TextIterator it(adjustedRange, behaviors); !it.AtEnd(); it.Advance())
    {
      length += it.Text().size();
    }
    return length;
  }

  namespace
  {
    KRYS_NODISCARD inline bool IsInsideReplacedElement(TextIterator &iterator,
                                                       TextIteratorBehaviors behaviors) noexcept
    {
      krys_debug_assert(!iterator.AtEnd());
      krys_debug_assert(iterator.Text().size() == 1);
      auto *node = iterator.GetNode();
      return node && IsRendererReplacedElement(node->Renderer(), behaviors);
    }

    KRYS_NODISCARD constexpr uint64 ClampedAdd(uint64 a, uint64 b) noexcept
    {
      auto sum = a + b;
      return sum >= a ? sum : std::numeric_limits<uint64>::max();
    }
  }

  dom::SimpleRange ResolveCharacterRange(const dom::SimpleRange &scope, CharacterRange range,
                                         TextIteratorBehaviors behaviors) noexcept
  {
    auto resultRange = dom::SimpleRange {range.Location ? scope.End : scope.Start,
                                         (range.Location || range.Length) ? scope.End : scope.Start};
    auto rangeEnd = ClampedAdd(range.Location, range.Length);
    uint64 location = 0;
    for (TextIterator it(scope, behaviors); !it.AtEnd(); it.Advance())
    {
      uint64 length = it.Text().size();
      auto textRunRange = it.Range();

      auto found = [&](uint64 targetLocation) -> bool
      {
        return targetLocation >= location && targetLocation - location <= length;
      };
      bool foundStart = found(range.Location);
      bool foundEnd = found(rangeEnd);

      if (foundEnd)
      {
        // FIXME: This is a workaround for the fact that the end of a run is often at the wrong position for
        // emitted
        // '\n's or if the renderer of the current node is a replaced element.
        // FIXME: consider controlling this with TextIteratorBehavior instead of doing it unconditionally to
        // help us eventually phase it out everywhere.
        if (length == 1 && (it.Text()[0] == '\n' || IsInsideReplacedElement(it, behaviors)))
        {
          it.Advance();
          if (!it.AtEnd())
          {
            textRunRange.End = it.Range().Start;
          }
          else
          {
            if (auto end = dom::MakeBoundaryPoint(
                  VisiblePosition(dom::MakeDeprecatedLegacyPosition(textRunRange.Start))
                    .Next()
                    .DeepEquivalent()))
            {
              textRunRange.End = *end;
            }
          }
        }
      }

      auto boundary = [&](uint64 targetLocation) -> dom::BoundaryPoint
      {
        if (auto *textNode = DynamicDowncast<dom::Text>(*textRunRange.Start.Container))
        {
          krys_debug_assert(targetLocation - location <= textNode->Length());
          uint64 offset = textRunRange.Start.Offset + targetLocation - location;
          return {ShareRef<dom::Node>(*textNode), offset};
        }
        return targetLocation == location ? textRunRange.Start : textRunRange.End;
      };

      if (foundStart)
      {
        resultRange.Start = boundary(range.Location);
      }
      if (foundEnd)
      {
        resultRange.End = boundary(rangeEnd);
        break;
      }

      location += length;
    }
    return resultRange;
  }

  // --------

  bool HasAnyPlainText(const dom::SimpleRange &range, TextIteratorBehaviors behaviors,
                       IgnoreCollapsedRanges ignoreCollapsedRanges) noexcept
  {
    for (TextIterator iterator {range, behaviors}; !iterator.AtEnd(); iterator.Advance())
    {
      if (ignoreCollapsedRanges == IgnoreCollapsedRanges::Yes && iterator.Range().Collapsed())
      {
        continue;
      }

      if (!iterator.Text().empty())
      {
        return true;
      }
    }
    return false;
  }

  utf8_string PlainText(const dom::SimpleRange &range, TextIteratorBehaviors defaultBehavior,
                        bool isDisplayString) noexcept
  {
    // The initial buffer size can be critical for performance.
    constexpr uint32 initialCapacity = 1 << 15;

    utf8_string builder;
    builder.reserve(initialCapacity);
    TextIteratorBehaviors behaviors = defaultBehavior;
    if (!isDisplayString)
    {
      behaviors.Add(TextIteratorBehavior::EmitsTextsWithoutTranscoding);
    }

    for (TextIterator it(range, behaviors); !it.AtEnd(); it.Advance())
    {
      it.AppendTextToString(builder);
    }

    // FIXME: Modifying display strings by the document's encoding (displayStringModifiedByEncoding), once
    // documents have encodings other than UTF-8.
    builder.shrink_to_fit();
    return builder;
  }

  utf8_string PlainTextReplacingNoBreakSpace(const dom::SimpleRange &range,
                                             TextIteratorBehaviors defaultBehaviors,
                                             bool isDisplayString) noexcept
  {
    auto text = PlainText(range, defaultBehaviors, isDisplayString);
    // U+00A0 is C2 A0 in UTF-8.
    utf8_string result;
    result.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i)
    {
      if (text[i] == 0xC2 && i + 1 < text.size() && static_cast<char32>(text[i + 1]) == 0xA0)
      {
        result.push_back(u8' ');
        ++i;
        continue;
      }
      result.push_back(text[i]);
    }
    return result;
  }
}
