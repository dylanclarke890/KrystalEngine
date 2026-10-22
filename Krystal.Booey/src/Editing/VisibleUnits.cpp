#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Core/TextBoundaries.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Editing/RenderedPosition.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <unicode/ubrk.h>
#include <unicode/utf8.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsHTMLBRElement(const dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element && element->TagQualifiedName().TagName() == html::TagName::br;
    }

    KRYS_NODISCARD RefPtr<dom::Node> PreviousLeafWithSameEditability(dom::Node *node,
                                                                     EditableType editableType) noexcept
    {
      bool editable = HasEditableStyle(*node, editableType);
      auto *previousNode = PreviousLeafNode(node);
      while (previousNode)
      {
        if (editable == HasEditableStyle(*previousNode, editableType))
        {
          return ShareRefPtr(previousNode);
        }
        previousNode = PreviousLeafNode(previousNode);
      }
      return nullptr;
    }

    KRYS_NODISCARD RefPtr<dom::Node> NextLeafWithSameEditability(dom::Node *node,
                                                                 EditableType editableType) noexcept
    {
      if (!node)
      {
        return nullptr;
      }

      bool editable = HasEditableStyle(*node, editableType);
      auto *nextNode = NextLeafNode(node);
      while (nextNode)
      {
        if (editable == HasEditableStyle(*nextNode, editableType))
        {
          return ShareRefPtr(nextNode);
        }
        nextNode = NextLeafNode(nextNode);
      }
      return nullptr;
    }

    // FIXME: consolidate with code in previousLinePosition.
    KRYS_NODISCARD dom::Position PreviousLineCandidatePosition(dom::Node *node,
                                                               const VisiblePosition &visiblePosition,
                                                               EditableType editableType) noexcept
    {
      auto highestRoot = HighestEditableRoot(visiblePosition.DeepEquivalent(), editableType);
      auto previousNode = PreviousLeafWithSameEditability(node, editableType);

      while (
        previousNode
        && (!previousNode->Renderer()
            || InSameLine(VisiblePosition(FirstPositionInOrBeforeNode(previousNode.get())), visiblePosition)))
      {
        previousNode = PreviousLeafWithSameEditability(previousNode.get(), editableType);
      }

      while (previousNode && !Is<dom::ShadowRoot>(*previousNode))
      {
        if (HighestEditableRoot(FirstPositionInOrBeforeNode(previousNode.get()), editableType) != highestRoot)
        {
          break;
        }

        dom::Position pos =
          IsHTMLBRElement(*previousNode)
            ? dom::PositionBeforeNode(previousNode.get())
            : dom::MakeDeprecatedLegacyPosition(RefPtr<dom::Node>(previousNode),
                                                static_cast<uint32>(CaretMaxOffset(*previousNode)));

        if (pos.IsCandidate())
        {
          return pos;
        }

        previousNode = PreviousLeafWithSameEditability(previousNode.get(), editableType);
      }
      return dom::Position();
    }

    KRYS_NODISCARD dom::Position NextLineCandidatePosition(dom::Node *node,
                                                           const VisiblePosition &visiblePosition,
                                                           EditableType editableType) noexcept
    {
      auto highestRoot = HighestEditableRoot(visiblePosition.DeepEquivalent(), editableType);
      auto nextNode = NextLeafWithSameEditability(node, editableType);
      while (
        nextNode
        && (!nextNode->Renderer()
            || InSameLine(VisiblePosition(FirstPositionInOrBeforeNode(nextNode.get())), visiblePosition)))
      {
        nextNode = NextLeafWithSameEditability(nextNode.get(), EditableType::ContentIsEditable);
      }

      while (nextNode && !Is<dom::ShadowRoot>(*nextNode))
      {
        if (HighestEditableRoot(FirstPositionInOrBeforeNode(nextNode.get()), editableType) != highestRoot)
        {
          break;
        }

        dom::Position pos;
        pos = dom::MakeDeprecatedLegacyPosition(RefPtr<dom::Node>(nextNode),
                                                static_cast<uint32>(CaretMinOffset(*nextNode)));

        if (pos.IsCandidate())
        {
          return pos;
        }

        nextNode = NextLeafWithSameEditability(nextNode.get(), editableType);
      }
      return dom::Position();
    }

    KRYS_NODISCARD bool IsTextOrLineBreakBox(const inline_iterator::LeafBoxIterator &box) noexcept
    {
      return box && (box->IsText() || box->Renderer().IsBR());
    }

    KRYS_NODISCARD inline_iterator::LeafBoxIterator
      PreviousTextOrLineBreakBox(inline_iterator::LeafBoxIterator box,
                                 inline_iterator::LineLogicalOrderCache &orderCache) noexcept
    {
      while (box)
      {
        box = inline_iterator::PreviousLeafOnLineInLogicalOrder(box, orderCache);
        if (IsTextOrLineBreakBox(box))
        {
          return box;
        }
      }
      return {};
    }

    KRYS_NODISCARD inline_iterator::LeafBoxIterator
      NextTextOrLineBreakBox(inline_iterator::LeafBoxIterator box,
                             inline_iterator::LineLogicalOrderCache &orderCache) noexcept
    {
      while (box)
      {
        box = inline_iterator::NextLeafOnLineInLogicalOrder(box, orderCache);
        if (IsTextOrLineBreakBox(box))
        {
          return box;
        }
      }
      return {};
    }

    KRYS_NODISCARD inline_iterator::LeafBoxIterator
      StartTextOrLineBreakBox(const inline_iterator::LineBoxIterator &lineBox,
                              inline_iterator::LineLogicalOrderCache &orderCache) noexcept
    {
      auto box = inline_iterator::FirstLeafOnLineInLogicalOrder(lineBox, orderCache);
      if (IsTextOrLineBreakBox(box))
      {
        return box;
      }
      return NextTextOrLineBreakBox(box, orderCache);
    }

    KRYS_NODISCARD inline_iterator::LeafBoxIterator
      EndTextOrLineBreakBox(const inline_iterator::LineBoxIterator &lineBox,
                            inline_iterator::LineLogicalOrderCache &orderCache) noexcept
    {
      auto box = inline_iterator::LastLeafOnLineInLogicalOrder(lineBox, orderCache);
      if (IsTextOrLineBreakBox(box))
      {
        return box;
      }
      return PreviousTextOrLineBreakBox(box, orderCache);
    }

    KRYS_NODISCARD inline_iterator::LeafBoxIterator
      LogicallyPreviousBox(const VisiblePosition &visiblePosition, inline_iterator::LeafBoxIterator startBox,
                           inline_iterator::LineLogicalOrderCache &orderCache,
                           bool &previousBoxInDifferentLine) noexcept
    {
      if (auto previousBox = PreviousTextOrLineBreakBox(startBox, orderCache))
      {
        return previousBox;
      }

      if (auto previousLineBox = startBox->GetLineBox()->Previous())
      {
        // FIXME: Why isn't previousBoxInDifferentLine set here?
        if (auto previousBox = EndTextOrLineBreakBox(previousLineBox, orderCache))
        {
          return previousBox;
        }
      }

      while (true)
      {
        auto *startNode = startBox->Renderer().NonPseudoNode();
        if (!startNode)
        {
          break;
        }

        dom::Position position =
          PreviousLineCandidatePosition(startNode, visiblePosition, EditableType::ContentIsEditable);
        if (position.IsNull())
        {
          break;
        }

        RenderedPosition renderedPosition(position, Affinity::Downstream);
        auto previousLineBox = renderedPosition.LineBox();
        if (!previousLineBox)
        {
          break;
        }

        if (previousLineBox != startBox->GetLineBox())
        {
          if (auto previousBox = EndTextOrLineBreakBox(previousLineBox, orderCache))
          {
            previousBoxInDifferentLine = true;
            return previousBox;
          }
        }

        startBox = inline_iterator::FirstLeafOnLineInLogicalOrder(previousLineBox, orderCache);
      }
      return {};
    }

    KRYS_NODISCARD inline_iterator::LeafBoxIterator
      LogicallyNextBox(const VisiblePosition &visiblePosition, inline_iterator::LeafBoxIterator startBox,
                       inline_iterator::LineLogicalOrderCache &orderCache,
                       bool &nextBoxInDifferentLine) noexcept
    {
      if (auto nextBox = NextTextOrLineBreakBox(startBox, orderCache))
      {
        return nextBox;
      }

      if (auto nextLineBox = startBox->GetLineBox()->Next())
      {
        // FIXME: Why isn't previousBoxInDifferentLine set here?
        if (auto nextBox = StartTextOrLineBreakBox(nextLineBox, orderCache))
        {
          return nextBox;
        }
      }

      while (true)
      {
        auto *startNode = startBox->Renderer().NonPseudoNode();
        if (!startNode)
        {
          break;
        }

        dom::Position position =
          NextLineCandidatePosition(startNode, visiblePosition, EditableType::ContentIsEditable);
        if (position.IsNull())
        {
          break;
        }

        RenderedPosition renderedPosition(position, Affinity::Downstream);
        auto nextLineBox = renderedPosition.LineBox();
        if (!nextLineBox)
        {
          break;
        }

        if (nextLineBox != startBox->GetLineBox())
        {
          if (auto nextBox = StartTextOrLineBreakBox(nextLineBox, orderCache))
          {
            nextBoxInDifferentLine = true;
            return nextBox;
          }
        }

        startBox = inline_iterator::LastLeafOnLineInLogicalOrderWithNode(nextLineBox, orderCache);
      }
      return {};
    }

    KRYS_NODISCARD UBreakIterator *WordBreakIteratorForMinOffsetBoundary(
      const VisiblePosition &visiblePosition, const inline_iterator::TextBoxIterator &textBox,
      uint32 &previousBoxLength, bool &previousBoxInDifferentLine, utf8_string &string) noexcept
    {
      previousBoxInDifferentLine = false;

      inline_iterator::LineLogicalOrderCache orderCache;
      auto previousBox =
        LogicallyPreviousBox(visiblePosition, textBox, orderCache, previousBoxInDifferentLine);
      while (previousBox && !previousBox->IsText())
      {
        krys_debug_assert(previousBox->Renderer().IsBR());
        previousBoxInDifferentLine = true;
        previousBox =
          LogicallyPreviousBox(visiblePosition, previousBox, orderCache, previousBoxInDifferentLine);
      }

      string.clear();

      if (previousBox)
      {
        inline_iterator::TextBoxIterator previousTextBox(*previousBox);
        previousBoxLength = previousTextBox->Length();
        string.append(previousTextBox->OriginalText());
      }
      string.append(textBox->OriginalText());

      return WordBreakIterator(string);
    }

    KRYS_NODISCARD UBreakIterator *
      WordBreakIteratorForMaxOffsetBoundary(const VisiblePosition &visiblePosition,
                                            const inline_iterator::TextBoxIterator &textBox,
                                            bool &nextBoxInDifferentLine, utf8_string &string) noexcept
    {
      nextBoxInDifferentLine = false;

      inline_iterator::LineLogicalOrderCache orderCache;
      auto nextBox = LogicallyNextBox(visiblePosition, textBox, orderCache, nextBoxInDifferentLine);
      while (nextBox && !nextBox->IsText())
      {
        krys_debug_assert(nextBox->Renderer().IsBR());
        nextBoxInDifferentLine = true;
        nextBox = LogicallyNextBox(visiblePosition, nextBox, orderCache, nextBoxInDifferentLine);
      }

      string.clear();
      string.append(textBox->OriginalText());

      if (nextBox)
      {
        inline_iterator::TextBoxIterator nextTextBox(*nextBox);
        string.append(nextTextBox->OriginalText());
      }

      return WordBreakIterator(string);
    }

    KRYS_NODISCARD bool IsLogicalStartOfWord(UBreakIterator *iter, int32 position,
                                             bool hardLineBreak) noexcept
    {
      bool boundary = hardLineBreak ? true : ubrk_isBoundary(iter, position);
      if (!boundary)
      {
        return false;
      }

      ubrk_following(iter, position);
      // isWordTextBreak returns true after moving across a word and false after moving across a
      // punctuation/space.
      return IsWordTextBreak(iter);
    }

    KRYS_NODISCARD bool IslogicalEndOfWord(UBreakIterator *iter, int32 position, bool hardLineBreak) noexcept
    {
      bool boundary = ubrk_isBoundary(iter, position);
      return (hardLineBreak || boundary) && IsWordTextBreak(iter);
    }

    enum class CursorMovementDirection : bool
    {
      MoveLeft,
      MoveRight
    };

    KRYS_NODISCARD VisiblePosition VisualWordPosition(const VisiblePosition &visiblePosition,
                                                      CursorMovementDirection direction,
                                                      bool skipsSpaceWhenMovingRight) noexcept
    {
      if (visiblePosition.IsNull() || !visiblePosition.DeepEquivalent().GetDocument())
      {
        return VisiblePosition();
      }

      visiblePosition.DeepEquivalent().GetDocument()->UpdateLayoutIgnorePendingStylesheets();

      TextDirection blockDirection = DirectionOfEnclosingBlock(visiblePosition.DeepEquivalent());
      inline_iterator::LeafBoxIterator previouslyVisitedBox;
      VisiblePosition current = visiblePosition;
      Maybe<VisiblePosition> previousPosition;
      UBreakIterator *iter = nullptr;

      utf8_string string;

      while (true)
      {
        VisiblePosition adjacentCharacterPosition =
          direction == CursorMovementDirection::MoveRight ? current.Right(true) : current.Left(true);
        if (adjacentCharacterPosition == current || adjacentCharacterPosition.IsNull())
        {
          return VisiblePosition();
        }
        // FIXME: This is a workaround for webkit.org/b/167138.
        if (previousPosition && adjacentCharacterPosition == *previousPosition)
        {
          return VisiblePosition();
        }

        // FIXME: Why force the use of upstream affinity here instead of VisiblePosition::inlineBoxAndOffset,
        // which will get affinity from adjacentCharacterPosition?
        auto [box, offsetInBox] =
          adjacentCharacterPosition.DeepEquivalent().GetInlineBoxAndOffset(Affinity::Upstream);

        if (!box)
        {
          break;
        }
        if (!box->IsText())
        {
          current = adjacentCharacterPosition;
          continue;
        }

        inline_iterator::TextBoxIterator textBox(*box);
        uint32 previousBoxLength = 0;
        bool previousBoxInDifferentLine = false;
        bool nextBoxInDifferentLine = false;
        bool movingIntoNewBox = previouslyVisitedBox != box;

        if (offsetInBox == textBox->MinimumCaretOffset())
        {
          iter = WordBreakIteratorForMinOffsetBoundary(adjacentCharacterPosition, textBox, previousBoxLength,
                                                       previousBoxInDifferentLine, string);
        }
        else if (offsetInBox == textBox->MaximumCaretOffset())
        {
          iter = WordBreakIteratorForMaxOffsetBoundary(adjacentCharacterPosition, textBox,
                                                       nextBoxInDifferentLine, string);
        }
        else if (movingIntoNewBox)
        {
          iter = WordBreakIterator(textBox->OriginalText());
          previouslyVisitedBox = box;
        }

        if (!iter)
        {
          break;
        }

        ubrk_first(iter);
        int32 offsetInIterator = static_cast<int32>(offsetInBox - textBox->Start() + previousBoxLength);

        bool isWordBreak;
        bool boxHasSameDirectionalityAsBlock = box->Direction() == blockDirection;
        bool movingBackward =
          (direction == CursorMovementDirection::MoveLeft && box->Direction() == TextDirection::LTR)
          || (direction == CursorMovementDirection::MoveRight && box->Direction() == TextDirection::RTL);
        if ((skipsSpaceWhenMovingRight && boxHasSameDirectionalityAsBlock)
            || (!skipsSpaceWhenMovingRight && movingBackward))
        {
          bool logicalStartInRenderer = offsetInBox == textBox->Start() && previousBoxInDifferentLine;
          isWordBreak = IsLogicalStartOfWord(iter, offsetInIterator, logicalStartInRenderer);
          if (isWordBreak && offsetInBox == box->MaximumCaretOffset() && nextBoxInDifferentLine)
          {
            isWordBreak = false;
          }
        }
        else
        {
          bool logicalEndInRenderer = offsetInBox == textBox->End() && nextBoxInDifferentLine;
          isWordBreak = IslogicalEndOfWord(iter, offsetInIterator, logicalEndInRenderer);
          if (isWordBreak && offsetInBox == box->MinimumCaretOffset() && previousBoxInDifferentLine)
          {
            isWordBreak = false;
          }
        }

        if (isWordBreak)
        {
          return adjacentCharacterPosition;
        }

        previousPosition = current;
        current = adjacentCharacterPosition;
      }
      return VisiblePosition();
    }
  }

  VisiblePosition LeftWordPosition(const VisiblePosition &visiblePosition,
                                   bool skipsSpaceWhenMovingRight) noexcept
  {
    VisiblePosition leftWordBreak =
      VisualWordPosition(visiblePosition, CursorMovementDirection::MoveLeft, skipsSpaceWhenMovingRight);
    leftWordBreak = visiblePosition.HonorEditingBoundaryAtOrBefore(leftWordBreak);

    // FIXME: How should we handle a non-editable position?
    if (leftWordBreak.IsNull() && IsEditablePosition(visiblePosition.DeepEquivalent()))
    {
      TextDirection blockDirection = DirectionOfEnclosingBlock(visiblePosition.DeepEquivalent());
      leftWordBreak = blockDirection == TextDirection::LTR ? StartOfEditableContent(visiblePosition)
                                                           : EndOfEditableContent(visiblePosition);
    }
    return leftWordBreak;
  }

  VisiblePosition RightWordPosition(const VisiblePosition &visiblePosition,
                                    bool skipsSpaceWhenMovingRight) noexcept
  {
    VisiblePosition rightWordBreak =
      VisualWordPosition(visiblePosition, CursorMovementDirection::MoveRight, skipsSpaceWhenMovingRight);
    rightWordBreak = visiblePosition.HonorEditingBoundaryAtOrBefore(rightWordBreak);

    // FIXME: How should we handle a non-editable position?
    if (rightWordBreak.IsNull() && IsEditablePosition(visiblePosition.DeepEquivalent()))
    {
      TextDirection blockDirection = DirectionOfEnclosingBlock(visiblePosition.DeepEquivalent());
      rightWordBreak = blockDirection == TextDirection::LTR ? EndOfEditableContent(visiblePosition)
                                                            : StartOfEditableContent(visiblePosition);
    }
    return rightWordBreak;
  }

  namespace
  {
    void Prepend(utf8_string &buffer, utf8_stringview string) noexcept
    {
      buffer.insert(0, string);
    }

    void PrependRepeatedCharacter(utf8_string &buffer, char8 character, uint32 count) noexcept
    {
      buffer.insert(0, count, character);
    }

    void AppendRepeatedCharacter(utf8_string &buffer, char8 character, uint32 count) noexcept
    {
      buffer.append(count, character);
    }

    KRYS_NODISCARD bool IsInTextSecurityMode(dom::Node *node) noexcept
    {
      return node && node->Renderer() && node->Renderer()->Style().GetTextSecurity() != TextSecurity::None;
    }
  }

  uint32 SuffixLengthForRange(const dom::SimpleRange &forwardsScanRange, utf8_string &string) noexcept
  {
    uint32 suffixLength = 0;
    TextIterator forwardsIterator(forwardsScanRange);
    while (!forwardsIterator.AtEnd())
    {
      utf8_stringview text = forwardsIterator.Text();
      uint32 i = EndOfFirstWordBoundaryContext(text);
      string.append(text.substr(0, i));
      suffixLength += i;
      if (i < text.size())
      {
        break;
      }
      forwardsIterator.Advance();
    }
    return suffixLength;
  }

  uint32 PrefixLengthForRange(const dom::SimpleRange &backwardsScanRange, utf8_string &string) noexcept
  {
    uint32 prefixLength = 0;
    SimplifiedBackwardsTextIterator backwardsIterator(backwardsScanRange);
    while (!backwardsIterator.AtEnd())
    {
      utf8_stringview text = backwardsIterator.Text();
      uint32 i = StartOfLastWordBoundaryContext(text);
      Prepend(string, text.substr(i));
      prefixLength += static_cast<uint32>(text.size()) - i;
      if (i > 0)
      {
        break;
      }
      backwardsIterator.Advance();
    }
    return prefixLength;
  }

  uint32 BackwardSearchForBoundaryWithTextIterator(SimplifiedBackwardsTextIterator &it, utf8_string &string,
                                                   uint32 suffixLength,
                                                   BoundarySearchFunction searchFunction) noexcept
  {
    uint32 next = 0;
    bool needMoreContext = false;
    while (!it.AtEnd())
    {
      bool inTextSecurityMode = IsInTextSecurityMode(it.GetNode());
      // iterate to get chunks until the searchFunction returns a non-zero value.
      if (!inTextSecurityMode)
      {
        Prepend(string, it.Text());
      }
      else
      {
        // Treat bullets used in the text security mode as regular characters when looking for boundaries
        PrependRepeatedCharacter(string, 'x', static_cast<uint32>(it.Text().size()));
      }
      if (string.size() > suffixLength)
      {
        next = searchFunction(string, static_cast<uint32>(string.size()) - suffixLength,
                              BoundarySearchContextAvailability::MayHaveMoreContext, needMoreContext);
        if (next > 1) // FIXME: This is a work around for https://webkit.org/b/115070. We need to provide more
                      // contexts in general case.
        {
          break;
        }
      }
      it.Advance();
    }
    if (needMoreContext && string.size() > suffixLength)
    {
      // The last search returned the beginning of the buffer and asked for more context,
      // but there is no earlier text. Force a search with what's available.
      next = searchFunction(string, static_cast<uint32>(string.size()) - suffixLength,
                            BoundarySearchContextAvailability::DontHaveMoreContext, needMoreContext);
      krys_debug_assert(!needMoreContext);
    }

    return next;
  }

  uint32 ForwardSearchForBoundaryWithTextIterator(TextIterator &it, utf8_string &string, uint32 prefixLength,
                                                  BoundarySearchFunction searchFunction) noexcept
  {
    uint32 next = 0;
    bool needMoreContext = false;
    while (!it.AtEnd())
    {
      bool inTextSecurityMode = IsInTextSecurityMode(it.GetNode());
      // Keep asking the iterator for chunks until the search function
      // returns an end value not equal to the length of the string passed to it.
      if (!inTextSecurityMode)
      {
        string.append(it.Text());
      }
      else
      {
        // Treat bullets used in the text security mode as regular characters when looking for boundaries
        AppendRepeatedCharacter(string, 'x', static_cast<uint32>(it.Text().size()));
      }
      if (string.size() > prefixLength)
      {
        next = searchFunction(string, prefixLength, BoundarySearchContextAvailability::MayHaveMoreContext,
                              needMoreContext);
        if (next != string.size())
        {
          break;
        }
      }
      it.Advance();
    }
    if (needMoreContext && string.size() > prefixLength)
    {
      // The last search returned the end of the buffer and asked for more context,
      // but there is no further text. Force a search with what's available.
      next = searchFunction(string, prefixLength, BoundarySearchContextAvailability::DontHaveMoreContext,
                            needMoreContext);
      krys_debug_assert(!needMoreContext);
    }

    return next;
  }

  namespace
  {
    enum class NeedsContextAtParagraphStart : bool
    {
      No,
      Yes
    };

    KRYS_NODISCARD VisiblePosition PreviousBoundary(
      const VisiblePosition &position, BoundarySearchFunction searchFunction,
      NeedsContextAtParagraphStart needsContextAtParagraphStart = NeedsContextAtParagraphStart::No) noexcept
    {
      auto boundary = position.DeepEquivalent().ParentEditingBoundary();
      if (!boundary)
      {
        return {};
      }

      utf8_string string;
      uint32 suffixLength = 0;

      auto searchRange = dom::MakeSimpleRange(dom::MakeBoundaryPointBeforeNodeContents(*boundary), position);
      if (!searchRange)
      {
        return {};
      }

      if (needsContextAtParagraphStart == NeedsContextAtParagraphStart::Yes && IsStartOfParagraph(position))
      {
        auto forwardsScanRange = dom::MakeSimpleRange(searchRange->Start, EndOfParagraph(position));
        if (!forwardsScanRange)
        {
          return {};
        }
        for (TextIterator forwardsIterator(*forwardsScanRange); !forwardsIterator.AtEnd();
             forwardsIterator.Advance())
        {
          string.append(forwardsIterator.Text());
        }
        suffixLength = static_cast<uint32>(string.size());
      }
      else if (RequiresContextForWordBoundary(position.CharacterBefore()))
      {
        auto forwardsScanRange =
          dom::MakeSimpleRange(searchRange->End, dom::MakeBoundaryPointAfterNodeContents(*boundary));
        suffixLength = SuffixLengthForRange(forwardsScanRange, string);
      }

      SimplifiedBackwardsTextIterator it(*searchRange);
      uint32 next = BackwardSearchForBoundaryWithTextIterator(it, string, suffixLength, searchFunction);

      if (!next)
      {
        return it.AtEnd() ? VisiblePosition(dom::MakeDeprecatedLegacyPosition(searchRange->Start)) : position;
      }

      auto &node = *(it.AtEnd() ? *searchRange : it.Range()).Start.Container;
      auto *textNode = DynamicDowncast<dom::Text>(node);
      if (textNode && !suffixLength && next <= textNode->Length())
      {
        // The next variable contains a usable index into a text node.
        return VisiblePosition(dom::MakeDeprecatedLegacyPosition(ShareRefPtr(&node), next));
      }

      // Use the character iterator to translate the next value into a DOM position.
      BackwardsCharacterIterator charIt(*searchRange);
      if (next < string.size() - suffixLength)
      {
        charIt.Advance(static_cast<int32>(string.size() - suffixLength - next));
      }
      // FIXME: charIt can get out of shadow host.
      return VisiblePosition(dom::MakeDeprecatedLegacyPosition(charIt.Range().End));
    }

    KRYS_NODISCARD VisiblePosition NextBoundary(const VisiblePosition &c,
                                                BoundarySearchFunction searchFunction) noexcept
    {
      dom::Position pos = c.DeepEquivalent();
      auto boundary = pos.ParentEditingBoundary();
      if (!boundary)
      {
        return VisiblePosition();
      }

      auto &boundaryDocument = boundary->NodeDocument();

      utf8_string string;
      uint32 prefixLength = 0;

      if (RequiresContextForWordBoundary(c.CharacterAfter()))
      {
        auto backwardsScanRange =
          dom::MakeSimpleRange(dom::MakeBoundaryPointBeforeNodeContents(boundaryDocument), c);
        if (!backwardsScanRange)
        {
          return {};
        }
        prefixLength = PrefixLengthForRange(*backwardsScanRange, string);
      }

      auto searchRange = dom::MakeSimpleRange(c, dom::MakeBoundaryPointAfterNodeContents(*boundary));
      if (!searchRange)
      {
        return {};
      }

      TextIterator it(*searchRange, TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions);
      uint32 next = ForwardSearchForBoundaryWithTextIterator(it, string, prefixLength, searchFunction);

      if (it.AtEnd() && next == string.size())
      {
        pos = dom::MakeDeprecatedLegacyPosition(searchRange->End);
      }
      else if (next > prefixLength)
      {
        // Use the character iterator to translate the next value into a DOM position.
        CharacterIterator charIt(*searchRange,
                                 TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions);
        charIt.Advance(static_cast<int32>(next - prefixLength - 1));
        if (charIt.AtEnd())
        {
          return {};
        }

        auto characterRange = charIt.Range();
        pos = dom::MakeDeprecatedLegacyPosition(characterRange.End);

        if (charIt.Text()[0] == '\n')
        {
          // FIXME: workaround for collapsed range (where only start position is correct) emitted for some
          // emitted newlines (see rdar://5192593)
          if (VisiblePosition(pos)
              == VisiblePosition(dom::MakeDeprecatedLegacyPosition(characterRange.Start)))
          {
            charIt.Advance(1);
            pos = dom::MakeDeprecatedLegacyPosition(charIt.Range().Start);
          }
        }
      }

      return VisiblePosition(pos, Affinity::Upstream);
    }
  }

  // ---------

  uint32 StartWordBoundary(utf8_stringview text, uint32 offset,
                           BoundarySearchContextAvailability mayHaveMoreContext,
                           bool &needMoreContext) noexcept
  {
    krys_debug_assert(offset);
    if (mayHaveMoreContext == BoundarySearchContextAvailability::MayHaveMoreContext
        && !StartOfLastWordBoundaryContext(text.substr(0, offset)))
    {
      needMoreContext = true;
      return 0;
    }
    needMoreContext = false;
    int32 start, end;
    int32 position = static_cast<int32>(offset);
    U8_BACK_1(reinterpret_cast<const uint8_t *>(text.data()), 0, position);
    FindWordBoundary(text, position, &start, &end);
    return static_cast<uint32>(start);
  }

  VisiblePosition StartOfWord(const VisiblePosition &c, WordSide side) noexcept
  {
    // FIXME: This returns a null VP for c at the start of the document
    // and side == LeftWordIfOnBoundary
    VisiblePosition p = c;
    if (side == WordSide::RightWordIfOnBoundary)
    {
      // at paragraph end, the startofWord is the current position
      if (IsEndOfParagraph(c))
      {
        return c;
      }

      p = c.Next();
      if (p.IsNull())
      {
        return c;
      }
    }
    return PreviousBoundary(p, StartWordBoundary);
  }

  uint32 EndWordBoundary(utf8_stringview text, uint32 offset,
                         BoundarySearchContextAvailability mayHaveMoreContext, bool &needMoreContext) noexcept
  {
    krys_debug_assert(offset <= text.size());
    if (mayHaveMoreContext == BoundarySearchContextAvailability::MayHaveMoreContext
        && EndOfFirstWordBoundaryContext(text.substr(offset)) == text.size() - offset)
    {
      needMoreContext = true;
      return static_cast<uint32>(text.size());
    }
    needMoreContext = false;
    int32 end;
    FindEndWordBoundary(text, static_cast<int32>(offset), &end);
    return static_cast<uint32>(end);
  }

  VisiblePosition EndOfWord(const VisiblePosition &c, WordSide side) noexcept
  {
    VisiblePosition p = c;
    if (side == WordSide::LeftWordIfOnBoundary)
    {
      if (IsStartOfParagraph(c))
      {
        return c;
      }

      p = c.Previous();
      if (p.IsNull())
      {
        return c;
      }
    }
    else if (IsEndOfParagraph(c))
    {
      return c;
    }

    return NextBoundary(p, EndWordBoundary);
  }

  namespace
  {
    KRYS_NODISCARD uint32 PreviousWordPositionBoundary(utf8_stringview text, uint32 offset,
                                                       BoundarySearchContextAvailability mayHaveMoreContext,
                                                       bool &needMoreContext) noexcept
    {
      if (mayHaveMoreContext == BoundarySearchContextAvailability::MayHaveMoreContext
          && !StartOfLastWordBoundaryContext(text.substr(0, offset)))
      {
        needMoreContext = true;
        return 0;
      }
      needMoreContext = false;
      return static_cast<uint32>(FindNextWordFromIndex(text, static_cast<int32>(offset), false));
    }

    KRYS_NODISCARD uint32 NextWordPositionBoundary(utf8_stringview text, uint32 offset,
                                                   BoundarySearchContextAvailability mayHaveMoreContext,
                                                   bool &needMoreContext) noexcept
    {
      if (mayHaveMoreContext == BoundarySearchContextAvailability::MayHaveMoreContext
          && EndOfFirstWordBoundaryContext(text.substr(offset)) == text.size() - offset)
      {
        needMoreContext = true;
        return static_cast<uint32>(text.size());
      }
      needMoreContext = false;
      return static_cast<uint32>(FindNextWordFromIndex(text, static_cast<int32>(offset), true));
    }
  }

  VisiblePosition PreviousWordPosition(const VisiblePosition &position) noexcept
  {
    return position.HonorEditingBoundaryAtOrBefore(PreviousBoundary(position, PreviousWordPositionBoundary));
  }

  VisiblePosition NextWordPosition(const VisiblePosition &position) noexcept
  {
    return position.HonorEditingBoundaryAtOrAfter(NextBoundary(position, NextWordPositionBoundary));
  }

  bool IsStartOfWord(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p == StartOfWord(p, WordSide::RightWordIfOnBoundary);
  }

  // ---------

  namespace
  {
    enum class LineEndpointComputationMode : bool
    {
      UseLogicalOrdering,
      UseInlineBoxOrdering
    };

    KRYS_NODISCARD VisiblePosition StartPositionForLine(const VisiblePosition &c,
                                                        LineEndpointComputationMode mode) noexcept
    {
      if (c.IsNull())
      {
        return VisiblePosition();
      }

      auto lineBox = RenderedPosition(c).LineBox();
      if (!lineBox)
      {
        // There are VisiblePositions at offset 0 in blocks without
        // RootInlineBoxes, like empty editable blocks and bordered blocks.
        dom::Position p = c.DeepEquivalent();
        if (p.DeprecatedNode()->Renderer() && p.DeprecatedNode()->Renderer()->IsRenderBlock()
            && !p.DeprecatedEditingOffset())
        {
          return c;
        }

        return VisiblePosition();
      }

      inline_iterator::LineLogicalOrderCache orderCache;

      dom::Node *startNode = nullptr;
      auto startBox = mode == LineEndpointComputationMode::UseLogicalOrdering
                        ? inline_iterator::FirstLeafOnLineInLogicalOrderWithNode(lineBox, orderCache)
                        : lineBox->LineLeftmostLeafBox();
      // Generated content (e.g. list markers and CSS :before and :after pseudoelements) have no corresponding
      // DOM element, and so cannot be represented by a VisiblePosition. Use whatever follows instead.
      while (true)
      {
        if (!startBox)
        {
          return VisiblePosition();
        }

        startNode = startBox->Renderer().NonPseudoNode();
        if (startNode)
        {
          break;
        }

        if (mode == LineEndpointComputationMode::UseLogicalOrdering)
        {
          startBox = inline_iterator::NextLeafOnLineInLogicalOrder(startBox, orderCache);
        }
        else
        {
          startBox.TraverseLineRightwardOnLine();
        }
      }

      auto *startTextNode = DynamicDowncast<dom::Text>(*startNode);
      return startTextNode
               ? VisiblePosition(dom::Position(ShareRefPtr(startTextNode),
                                               inline_iterator::TextBoxIterator(*startBox)->Start()))
               : VisiblePosition(dom::PositionBeforeNode(startNode));
    }

    KRYS_NODISCARD VisiblePosition StartOfLine(const VisiblePosition &c, LineEndpointComputationMode mode,
                                               bool *reachedBoundary) noexcept
    {
      if (reachedBoundary)
      {
        *reachedBoundary = false;
      }
      // TODO: this is the current behavior that might need to be fixed.
      // Please refer to https://bugs.webkit.org/show_bug.cgi?id=49107 for detail.
      VisiblePosition visPos = StartPositionForLine(c, mode);

      if (mode == LineEndpointComputationMode::UseLogicalOrdering)
      {
        if (auto editableRoot = HighestEditableRoot(c.DeepEquivalent()))
        {
          if (!editableRoot->Contains(visPos.DeepEquivalent().GetContainerNode()))
          {
            VisiblePosition newPosition = dom::FirstPositionInNode(editableRoot.get());
            if (reachedBoundary)
            {
              *reachedBoundary = c == newPosition;
            }
            return newPosition;
          }
        }
      }

      return c.HonorEditingBoundaryAtOrBefore(visPos, reachedBoundary);
    }
  }

  // FIXME: Rename this function to reflect the fact it ignores bidi levels.
  VisiblePosition StartOfLine(const VisiblePosition &currentPosition) noexcept
  {
    return StartOfLine(currentPosition, LineEndpointComputationMode::UseInlineBoxOrdering, nullptr);
  }

  VisiblePosition LogicalStartOfLine(const VisiblePosition &currentPosition, bool *reachedBoundary) noexcept
  {
    return StartOfLine(currentPosition, LineEndpointComputationMode::UseLogicalOrdering, reachedBoundary);
  }

  namespace
  {
    KRYS_NODISCARD VisiblePosition EndPositionForLine(const VisiblePosition &c,
                                                      LineEndpointComputationMode mode) noexcept
    {
      if (c.IsNull())
      {
        return VisiblePosition();
      }

      auto lineBox = RenderedPosition(c).LineBox();
      if (!lineBox)
      {
        // There are VisiblePositions at offset 0 in blocks without
        // RootInlineBoxes, like empty editable blocks and bordered blocks.
        dom::Position p = c.DeepEquivalent();
        if (p.DeprecatedNode()->Renderer() && p.DeprecatedNode()->Renderer()->IsRenderBlock()
            && !p.DeprecatedEditingOffset())
        {
          return c;
        }
        return VisiblePosition();
      }

      inline_iterator::LineLogicalOrderCache orderCache;

      dom::Node *endNode = nullptr;
      auto endBox = mode == LineEndpointComputationMode::UseLogicalOrdering
                      ? inline_iterator::LastLeafOnLineInLogicalOrder(lineBox, orderCache)
                      : lineBox->LineRightmostLeafBox();
      // Generated content (e.g. list markers and CSS :before and :after pseudoelements) have no corresponding
      // DOM element, and so cannot be represented by a VisiblePosition. Use whatever precedes instead.
      while (true)
      {
        if (!endBox)
        {
          return VisiblePosition();
        }

        endNode = endBox->Renderer().NonPseudoNode();
        if (endNode)
        {
          break;
        }

        if (mode == LineEndpointComputationMode::UseLogicalOrdering)
        {
          endBox = inline_iterator::PreviousLeafOnLineInLogicalOrder(endBox, orderCache);
        }
        else
        {
          endBox.TraverseLineLeftwardOnLine();
        }
      }

      dom::Position pos;
      if (IsHTMLBRElement(*endNode))
      {
        pos = dom::PositionBeforeNode(endNode);
      }
      else if (auto *endTextNode = DynamicDowncast<dom::Text>(*endNode); endTextNode && endBox->IsText())
      {
        inline_iterator::TextBoxIterator endTextBox(*endBox);
        uint32 endOffset = endTextBox->Start();
        if (!endTextBox->IsLineBreak())
        {
          endOffset += endTextBox->Length();
        }
        pos = dom::Position(ShareRefPtr(endTextNode), endOffset);
      }
      else
      {
        pos = dom::PositionAfterNode(endNode);
      }

      return VisiblePosition(pos, Affinity::Upstream);
    }

    KRYS_NODISCARD bool InSameLogicalLine(const VisiblePosition &a, const VisiblePosition &b) noexcept
    {
      return a.IsNotNull() && LogicalStartOfLine(a) == LogicalStartOfLine(b);
    }

    KRYS_NODISCARD VisiblePosition EndOfLine(const VisiblePosition &c, LineEndpointComputationMode mode,
                                             bool *reachedBoundary) noexcept
    {
      if (reachedBoundary)
      {
        *reachedBoundary = false;
      }
      // TODO: this is the current behavior that might need to be fixed.
      // Please refer to https://bugs.webkit.org/show_bug.cgi?id=49107 for detail.
      VisiblePosition visPos = EndPositionForLine(c, mode);

      if (mode == LineEndpointComputationMode::UseLogicalOrdering)
      {
        // Make sure the end of line is at the same line as the given input position. For a wrapping line, the
        // logical end position for the not-last-2-lines might incorrectly hand back the logical beginning of
        // the next line. For example, <div contenteditable dir="rtl"
        // style="line-break:before-white-space">abcdefg abcdefg abcdefg a abcdefg abcdefg abcdefg abcdefg
        // abcdefg abcdefg abcdefg abcdefg abcdefg abcdefg </div> In this case, use the previous position of
        // the computed logical end position.
        if (!InSameLogicalLine(c, visPos))
        {
          visPos = visPos.Previous();
        }

        if (auto editableRoot = HighestEditableRoot(c.DeepEquivalent()))
        {
          if (!editableRoot->Contains(visPos.DeepEquivalent().GetContainerNode()))
          {
            VisiblePosition newPosition = dom::LastPositionInNode(editableRoot.get());
            if (reachedBoundary)
            {
              *reachedBoundary = c == newPosition;
            }
            return newPosition;
          }
        }

        return c.HonorEditingBoundaryAtOrAfter(visPos, reachedBoundary);
      }

      // Make sure the end of line is at the same line as the given input position. Else use the previous
      // position to obtain end of line. This condition happens when the input position is before the space
      // character at the end of a soft-wrapped non-editable line. In this scenario, endPositionForLine would
      // incorrectly hand back a position in the next line instead. This fix is to account for the discrepancy
      // between lines with webkit-line-break:after-white-space style versus lines without that style, which
      // would break before a space by default.
      if (!InSameLine(c, visPos))
      {
        visPos = c.Previous();
        if (visPos.IsNull())
        {
          return VisiblePosition();
        }
        visPos = EndPositionForLine(visPos, LineEndpointComputationMode::UseInlineBoxOrdering);
      }

      return c.HonorEditingBoundaryAtOrAfter(visPos, reachedBoundary);
    }
  }

  // FIXME: Rename this function to reflect the fact it ignores bidi levels.
  VisiblePosition EndOfLine(const VisiblePosition &currentPosition) noexcept
  {
    return EndOfLine(currentPosition, LineEndpointComputationMode::UseInlineBoxOrdering, nullptr);
  }

  VisiblePosition LogicalEndOfLine(const VisiblePosition &currentPosition, bool *reachedBoundary) noexcept
  {
    return EndOfLine(currentPosition, LineEndpointComputationMode::UseLogicalOrdering, reachedBoundary);
  }

  bool InSameLine(const VisiblePosition &a, const VisiblePosition &b) noexcept
  {
    return a.IsNotNull() && StartOfLine(a) == StartOfLine(b);
  }

  bool IsStartOfLine(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p == StartOfLine(p);
  }

  bool IsEndOfLine(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p == EndOfLine(p);
  }

  bool IsLogicalEndOfLine(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p == LogicalEndOfLine(p);
  }

  namespace
  {
    KRYS_NODISCARD inline LayoutPoint
      AbsoluteLineDirectionPointToLocalPointInBlock(const inline_iterator::LineBoxIterator &lineBox,
                                                    LayoutUnit lineDirectionPoint) noexcept
    {
      auto &root = lineBox->FormattingContextRoot();
      auto absoluteBlockPoint =
        root.LocalToAbsolute(FloatPoint()) - ToFloatSize(FloatPoint(root.GetScrollPosition()));

      if (root.IsHorizontalWritingMode())
      {
        return LayoutPoint(lineDirectionPoint - LayoutUnit(absoluteBlockPoint.X()),
                           LayoutUnit(inline_iterator::ContentStartInBlockDirection(*lineBox)));
      }

      return LayoutPoint(LayoutUnit(inline_iterator::ContentStartInBlockDirection(*lineBox)),
                         lineDirectionPoint - LayoutUnit(absoluteBlockPoint.Y()));
    }

    KRYS_NODISCARD dom::Element *RootEditableOrDocumentElement(dom::Node &node,
                                                               EditableType editableType) noexcept
    {
      if (HasEditableStyle(node, editableType))
      {
        return EditableRootForPosition(FirstPositionInOrBeforeNode(&node), editableType);
      }
      return node.NodeDocument().DocumentElement().get();
    }

    KRYS_NODISCARD VisiblePosition PositionInLineBox(const inline_iterator::LineBoxIterator &lineBox,
                                                     LayoutUnit lineDirectionPoint,
                                                     const dom::Position &p) noexcept
    {
      // FIXME: Can be wrong for multi-column layout and with transforms.
      auto pointInLine = AbsoluteLineDirectionPointToLocalPointInBlock(lineBox, lineDirectionPoint);
      auto box = inline_iterator::ClosestBoxForHorizontalPosition(
        *lineBox, (lineBox->IsHorizontal() ? pointInLine.X() : pointInLine.Y()).ToFloat(),
        IsEditablePosition(p));
      if (!box)
      {
        return VisiblePosition();
      }
      auto &renderer = box->Renderer();
      auto *node = renderer.GetNode();
      if (node && EditingIgnoresContent(*node))
      {
        return dom::PositionInParentBeforeNode(node);
      }
      // FIXME: The HitTestSource state should be propagated down from calls into JavaScript bindings.
      // For the time being, just err on the side of passing in `Bindings`.
      auto *renderBox = DynamicDowncast<RenderBox>(renderer);
      auto localOffset = renderBox ? renderBox->LocationOffset() : LayoutSize {};
      return const_cast<RenderObject &>(renderer).VisiblePositionForPoint(pointInLine - localOffset,
                                                                          HitTestSource::Script);
    }
  }

  VisiblePosition PreviousLinePosition(const VisiblePosition &visiblePosition, LayoutUnit lineDirectionPoint,
                                       EditableType editableType) noexcept
  {
    dom::Position p = visiblePosition.DeepEquivalent();
    auto *node = p.DeprecatedNode();

    if (!node)
    {
      return VisiblePosition();
    }

    node->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return VisiblePosition();
    }

    inline_iterator::LineBoxIterator lineBox;
    if (auto box = visiblePosition.GetInlineBoxAndOffset().Box)
    {
      lineBox = box->GetLineBox()->Previous();
      // We want to skip zero height boxes.
      // This could happen in case it is a LegacyRootInlineBox with trailing floats.
      if (!lineBox || !lineBox->LogicalHeight() || !lineBox->LineLeftmostLeafBox())
      {
        lineBox = {};
      }
    }

    if (!lineBox)
    {
      dom::Position position = PreviousLineCandidatePosition(node, visiblePosition, editableType);
      if (position.IsNotNull())
      {
        RenderedPosition renderedPosition(VisiblePosition {position});
        lineBox = renderedPosition.LineBox();
        if (!lineBox)
        {
          return position;
        }
      }
    }

    if (lineBox)
    {
      return PositionInLineBox(lineBox, lineDirectionPoint, p);
    }

    // Could not find a previous line. This means we must already be on the first line.
    // Move to the start of the content in this block, which effectively moves us
    // to the start of the line we're on.
    auto *rootElement = RootEditableOrDocumentElement(*node, editableType);
    if (!rootElement)
    {
      return VisiblePosition();
    }
    return dom::FirstPositionInNode(rootElement);
  }

  VisiblePosition NextLinePosition(const VisiblePosition &visiblePosition, LayoutUnit lineDirectionPoint,
                                   EditableType editableType) noexcept
  {
    dom::Position p = visiblePosition.DeepEquivalent();
    auto *node = p.DeprecatedNode();
    if (!node)
    {
      return VisiblePosition();
    }

    node->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (!node->Renderer())
    {
      return VisiblePosition();
    }

    inline_iterator::LineBoxIterator lineBox;
    if (auto box = visiblePosition.GetInlineBoxAndOffset().Box)
    {
      lineBox = box->GetLineBox()->Next();
      // We want to skip zero height boxes.
      // This could happen in case it is a LegacyRootInlineBox with trailing floats.
      if (!lineBox || !lineBox->LogicalHeight() || !lineBox->LineLeftmostLeafBox())
      {
        lineBox = {};
      }
    }

    if (!lineBox)
    {
      // FIXME: We need do the same in previousLinePosition.
      if (auto *child = node->TraverseToChildAt(static_cast<uint32>(p.DeprecatedEditingOffset())))
      {
        node = child;
      }
      else
      {
        node = node->LastDescendant();
      }
      dom::Position position = NextLineCandidatePosition(node, visiblePosition, editableType);
      if (position.IsNotNull())
      {
        RenderedPosition renderedPosition(VisiblePosition {position});
        lineBox = renderedPosition.LineBox();
        if (!lineBox)
        {
          return position;
        }
      }
    }

    if (lineBox)
    {
      return PositionInLineBox(lineBox, lineDirectionPoint, p);
    }

    // Could not find a next line. This means we must already be on the last line.
    // Move to the end of the content in this block, which effectively moves us
    // to the end of the line we're on.
    auto *rootElement = RootEditableOrDocumentElement(*node, editableType);
    if (!rootElement)
    {
      return VisiblePosition();
    }
    return dom::LastPositionInNode(rootElement);
  }

  // ---------

  uint32 StartSentenceBoundary(utf8_stringview text, uint32, BoundarySearchContextAvailability,
                               bool &) noexcept
  {
    // FIXME: The following function can return -1; we don't handle that.
    return static_cast<uint32>(ubrk_preceding(SentenceBreakIterator(text), static_cast<int32>(text.size())));
  }

  VisiblePosition StartOfSentence(const VisiblePosition &position) noexcept
  {
    return PreviousBoundary(position, StartSentenceBoundary, NeedsContextAtParagraphStart::Yes);
  }

  uint32 EndSentenceBoundary(utf8_stringview text, uint32, BoundarySearchContextAvailability, bool &) noexcept
  {
    return static_cast<uint32>(ubrk_next(SentenceBreakIterator(text)));
  }

  VisiblePosition EndOfSentence(const VisiblePosition &position) noexcept
  {
    // FIXME: This includes the space after the punctuation that marks the end of the sentence.
    return NextBoundary(position, EndSentenceBoundary);
  }

  namespace
  {
    KRYS_NODISCARD uint32 PreviousSentencePositionBoundary(utf8_stringview text, uint32,
                                                           BoundarySearchContextAvailability, bool &) noexcept
    {
      // FIXME: This is identical to startSentenceBoundary. I'm pretty sure that's not right.
      // FIXME: The following function can return -1; we don't handle that.
      return static_cast<uint32>(
        ubrk_preceding(SentenceBreakIterator(text), static_cast<int32>(text.size())));
    }

    KRYS_NODISCARD uint32 NextSentencePositionBoundary(utf8_stringview text, uint32,
                                                       BoundarySearchContextAvailability, bool &) noexcept
    {
      // FIXME: This is identical to endSentenceBoundary.
      // That isn't right. This function needs to move to the equivalent position in the following sentence.
      return static_cast<uint32>(ubrk_following(SentenceBreakIterator(text), 0));
    }
  }

  VisiblePosition PreviousSentencePosition(const VisiblePosition &position) noexcept
  {
    return position.HonorEditingBoundaryAtOrBefore(
      PreviousBoundary(position, PreviousSentencePositionBoundary));
  }

  VisiblePosition NextSentencePosition(const VisiblePosition &position) noexcept
  {
    return position.HonorEditingBoundaryAtOrAfter(NextBoundary(position, NextSentencePositionBoundary));
  }

  RefPtr<dom::Node> FindStartOfParagraph(dom::Node *startNode, dom::Node *highestRoot, dom::Node *startBlock,
                                         int32 &offset, dom::Position::AnchorType &type,
                                         EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    dom::Node *node = startNode;
    dom::Node *n = startNode;
    bool startNodeIsEditable = startNode->HasEditableStyle();
    while (n)
    {
      if (boundaryCrossingRule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary
          && !dom::Position::NodeIsUserSelectAll(n) && n->HasEditableStyle() != startNodeIsEditable)
      {
        break;
      }
      if (boundaryCrossingRule == EditingBoundaryCrossingRule::CanSkipOverEditingBoundary)
      {
        while (n && n->HasEditableStyle() != startNodeIsEditable)
        {
          n = dom::TreeQueries::PreviousPostOrder(*n, startBlock);
        }
        if (!n || !n->IsDescendantOf(highestRoot))
        {
          break;
        }
      }
      auto *r = n->Renderer();
      if (!r)
      {
        n = dom::TreeQueries::PreviousPostOrder(*n, startBlock);
        continue;
      }
      const RenderStyle &style = r->Style();
      if (style.GetVisibility() != Visibility::Visible)
      {
        n = dom::TreeQueries::PreviousPostOrder(*n, startBlock);
        continue;
      }

      if (r->IsBR() || IsBlock(*n))
      {
        break;
      }

      if (auto *renderText = DynamicDowncast<RenderText>(*r); renderText && renderText->HasRenderedText())
      {
        krys_debug_assert(Is<dom::Text>(*n));
        type = dom::Position::AnchorType::PositionIsOffsetInAnchor;
        if (style.PreserveNewline())
        {
          const auto &text = renderText->Text();
          int32 i = static_cast<int32>(text.size());
          int32 o = offset;
          if (n == startNode && o < i)
          {
            i = std::max(0, o);
          }
          while (--i >= 0)
          {
            if (text[static_cast<size_t>(i)] == '\n')
            {
              offset = i + 1;
              return ShareRefPtr(n);
            }
          }
        }
        node = n;
        offset = 0;
        n = dom::TreeQueries::PreviousPostOrder(*n, startBlock);
      }
      else if (EditingIgnoresContent(*n) || IsRenderedTable(n))
      {
        node = n;
        type = dom::Position::AnchorType::PositionIsBeforeAnchor;
        n = n->PreviousSibling() ? n->PreviousSibling() : dom::TreeQueries::PreviousPostOrder(*n, startBlock);
      }
      else
      {
        n = dom::TreeQueries::PreviousPostOrder(*n, startBlock);
      }
    }

    return ShareRefPtr(node);
  }

  RefPtr<dom::Node> FindEndOfParagraph(dom::Node *startNode, dom::Node *highestRoot,
                                       dom::Node *stayInsideBlock, int32 &offset,
                                       dom::Position::AnchorType &type,
                                       EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    dom::Node *node = startNode;
    dom::Node *n = startNode;
    bool startNodeIsEditable = startNode->HasEditableStyle();
    while (n)
    {
      if (boundaryCrossingRule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary
          && !dom::Position::NodeIsUserSelectAll(n) && n->HasEditableStyle() != startNodeIsEditable)
      {
        break;
      }
      if (boundaryCrossingRule == EditingBoundaryCrossingRule::CanSkipOverEditingBoundary)
      {
        while (n && n->HasEditableStyle() != startNodeIsEditable)
        {
          n = dom::TreeQueries::Next(*n, stayInsideBlock);
        }
        if (!n || !n->IsDescendantOf(highestRoot))
        {
          break;
        }
      }

      auto *r = n->Renderer();
      if (!r)
      {
        n = dom::TreeQueries::Next(*n, stayInsideBlock);
        continue;
      }
      const RenderStyle &style = r->Style();
      if (style.GetVisibility() != Visibility::Visible)
      {
        n = dom::TreeQueries::Next(*n, stayInsideBlock);
        continue;
      }

      // FIXME: This is wrong when startNode is a block. We should return a position after the block.
      if (r->IsBR() || IsHTMLBRElement(*n) || IsBlock(*n))
      {
        break;
      }

      // FIXME: We avoid returning a position where the renderer can't accept the caret.
      if (auto *renderText = DynamicDowncast<RenderText>(*r); renderText && renderText->HasRenderedText())
      {
        krys_debug_assert(Is<dom::Text>(*n));
        type = dom::Position::AnchorType::PositionIsOffsetInAnchor;
        if (style.PreserveNewline())
        {
          const auto &text = renderText->Text();
          int32 o = n == startNode ? offset : 0;
          int32 length = static_cast<int32>(text.size());
          for (int32 i = o; i < length; ++i)
          {
            if (text[static_cast<size_t>(i)] == '\n')
            {
              offset = i;
              return ShareRefPtr(n);
            }
          }
        }
        node = n;
        offset = r->CaretMaxOffset();
        n = dom::TreeQueries::Next(*n, stayInsideBlock);
      }
      else if (EditingIgnoresContent(*n) || IsRenderedTable(n))
      {
        node = n;
        type = dom::Position::AnchorType::PositionIsAfterAnchor;
        n = dom::TreeQueries::NextSkippingChildren(*n, stayInsideBlock);
      }
      else
      {
        n = dom::TreeQueries::Next(*n, stayInsideBlock);
      }
    }
    return ShareRefPtr(node);
  }

  namespace
  {
    KRYS_NODISCARD VisiblePosition PositionForParagraphNode(RefPtr<dom::Node> &&node, int32 offset,
                                                            dom::Position::AnchorType type) noexcept
    {
      if (auto *textNode = DynamicDowncast<dom::Text>(node.get()))
      {
        return dom::Position(ShareRefPtr(textNode), static_cast<uint32>(offset));
      }

      if (type == dom::Position::AnchorType::PositionIsOffsetInAnchor)
      {
        return dom::Position(krys::move(node), static_cast<uint32>(offset), type);
      }

      return dom::Position(krys::move(node), type);
    }
  }

  VisiblePosition StartOfParagraph(const VisiblePosition &c,
                                   EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    auto p = c.DeepEquivalent();
    auto *startNode = p.DeprecatedNode();

    if (!startNode)
    {
      return VisiblePosition();
    }

    if (IsRenderedAsNonInlineTableImageOrHR(startNode))
    {
      return dom::PositionBeforeNode(startNode);
    }

    auto startBlock = EnclosingBlock(ShareRefPtr(startNode));

    auto highestRoot = HighestEditableRoot(p);
    int32 offset = p.DeprecatedEditingOffset();
    auto type = p.GetAnchorType();

    auto node = FindStartOfParagraph(startNode, highestRoot.get(), startBlock.get(), offset, type,
                                     boundaryCrossingRule);

    return PositionForParagraphNode(krys::move(node), offset, type);
  }

  VisiblePosition EndOfParagraph(const VisiblePosition &c,
                                 EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    if (c.IsNull())
    {
      return VisiblePosition();
    }

    auto p = c.DeepEquivalent();
    auto *startNode = p.DeprecatedNode();

    if (IsRenderedAsNonInlineTableImageOrHR(startNode))
    {
      return dom::PositionAfterNode(startNode);
    }

    auto stayInsideBlock = EnclosingBlock(ShareRefPtr(startNode));

    auto highestRoot = HighestEditableRoot(p);
    int32 offset = p.DeprecatedEditingOffset();
    auto type = p.GetAnchorType();

    auto node = FindEndOfParagraph(startNode, highestRoot.get(), stayInsideBlock.get(), offset, type,
                                   boundaryCrossingRule);

    return PositionForParagraphNode(krys::move(node), offset, type);
  }

  // FIXME: isStartOfParagraph(startOfNextParagraph(pos)) is not always true
  VisiblePosition StartOfNextParagraph(const VisiblePosition &visiblePosition) noexcept
  {
    VisiblePosition paragraphEnd(
      EndOfParagraph(visiblePosition, EditingBoundaryCrossingRule::CanSkipOverEditingBoundary));
    VisiblePosition afterParagraphEnd(
      paragraphEnd.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary));
    // The position after the last position in the last cell of a table
    // is not the start of the next paragraph.
    if (IsFirstPositionAfterTable(afterParagraphEnd))
    {
      return afterParagraphEnd.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    }
    return afterParagraphEnd;
  }

  bool InSameParagraph(const VisiblePosition &a, const VisiblePosition &b,
                       EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    return a.IsNotNull()
           && StartOfParagraph(a, boundaryCrossingRule) == StartOfParagraph(b, boundaryCrossingRule);
  }

  bool IsStartOfParagraph(const VisiblePosition &pos,
                          EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    return pos.IsNotNull() && pos == StartOfParagraph(pos, boundaryCrossingRule);
  }

  bool IsEndOfParagraph(const VisiblePosition &pos, EditingBoundaryCrossingRule boundaryCrossingRule) noexcept
  {
    return pos.IsNotNull() && pos == EndOfParagraph(pos, boundaryCrossingRule);
  }

  bool IsBlankParagraph(const VisiblePosition &position) noexcept
  {
    return IsStartOfParagraph(position) && StartOfParagraph(position.Next()) != StartOfParagraph(position);
  }

  VisiblePosition PreviousParagraphPosition(const VisiblePosition &p, LayoutUnit x) noexcept
  {
    VisiblePosition pos = p;
    do
    {
      VisiblePosition n = PreviousLinePosition(pos, x);
      if (n.IsNull() || n == pos)
      {
        break;
      }
      pos = n;
    } while (InSameParagraph(p, pos));
    return pos;
  }

  VisiblePosition NextParagraphPosition(const VisiblePosition &p, LayoutUnit x) noexcept
  {
    VisiblePosition pos = p;
    do
    {
      VisiblePosition n = NextLinePosition(pos, x);
      if (n.IsNull() || n == pos)
      {
        break;
      }
      pos = n;
    } while (InSameParagraph(p, pos));
    return pos;
  }

  // ---------

  VisiblePosition StartOfBlock(const VisiblePosition &visiblePosition,
                               EditingBoundaryCrossingRule rule) noexcept
  {
    dom::Position position = visiblePosition.DeepEquivalent();
    RefPtr<dom::Element> startBlock;
    if (!position.GetContainerNode()
        || !(startBlock = EnclosingBlock(ShareRefPtr(position.GetContainerNode()), rule)))
    {
      return VisiblePosition();
    }
    return dom::FirstPositionInNode(startBlock.get());
  }

  VisiblePosition EndOfBlock(const VisiblePosition &visiblePosition,
                             EditingBoundaryCrossingRule rule) noexcept
  {
    dom::Position position = visiblePosition.DeepEquivalent();
    RefPtr<dom::Element> endBlock;
    if (!position.GetContainerNode()
        || !(endBlock = EnclosingBlock(ShareRefPtr(position.GetContainerNode()), rule)))
    {
      return VisiblePosition();
    }
    return dom::LastPositionInNode(endBlock.get());
  }

  bool InSameBlock(const VisiblePosition &a, const VisiblePosition &b) noexcept
  {
    return !a.IsNull()
           && EnclosingBlock(ShareRefPtr(a.DeepEquivalent().GetContainerNode()))
                == EnclosingBlock(ShareRefPtr(b.DeepEquivalent().GetContainerNode()));
  }

  bool IsStartOfBlock(const VisiblePosition &pos) noexcept
  {
    return pos.IsNotNull() && pos == StartOfBlock(pos, EditingBoundaryCrossingRule::CanCrossEditingBoundary);
  }

  bool IsEndOfBlock(const VisiblePosition &pos) noexcept
  {
    return pos.IsNotNull() && pos == EndOfBlock(pos, EditingBoundaryCrossingRule::CanCrossEditingBoundary);
  }

  // ---------

  VisiblePosition StartOfDocument(const dom::Node *node) noexcept
  {
    if (!node || !node->NodeDocument().DocumentElement())
    {
      return VisiblePosition();
    }

    // The canonicalization of the position at (documentElement, 0) can turn the visible
    // position to null, even when there's a valid candidate to be had, because the root HTML element
    // is not content editable.  So we construct directly from the valid candidate.
    auto documentElement = node->NodeDocument().DocumentElement();
    dom::Position firstCandidate =
      NextCandidate(dom::MakeDeprecatedLegacyPosition(RefPtr<dom::Node>(documentElement), 0));
    if (firstCandidate.IsNull())
    {
      return VisiblePosition();
    }
    return VisiblePosition(firstCandidate);
  }

  VisiblePosition StartOfDocument(const VisiblePosition &c) noexcept
  {
    return StartOfDocument(c.DeepEquivalent().DeprecatedNode());
  }

  VisiblePosition EndOfDocument(const dom::Node *node) noexcept
  {
    if (!node || !node->NodeDocument().DocumentElement())
    {
      return VisiblePosition();
    }

    // (As above, in startOfDocument.)  The canonicalization can reject valid visible positions
    // when descending from the root element, so we construct the visible position directly from a
    // valid candidate.
    auto documentElement = node->NodeDocument().DocumentElement();
    dom::Position lastPosition = dom::MakeDeprecatedLegacyPosition(RefPtr<dom::Node>(documentElement),
                                                                   documentElement->CountChildNodes());
    dom::Position lastCandidate = PreviousCandidate(lastPosition);
    if (lastCandidate.IsNull())
    {
      return VisiblePosition();
    }
    return VisiblePosition(lastCandidate);
  }

  VisiblePosition EndOfDocument(const VisiblePosition &c) noexcept
  {
    return EndOfDocument(c.DeepEquivalent().DeprecatedNode());
  }

  bool IsStartOfDocument(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p.Previous(EditingBoundaryCrossingRule::CanCrossEditingBoundary).IsNull();
  }

  bool IsEndOfDocument(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p.Next(EditingBoundaryCrossingRule::CanCrossEditingBoundary).IsNull();
  }

  // ---------

  VisiblePosition StartOfEditableContent(const VisiblePosition &visiblePosition) noexcept
  {
    auto highestRoot = HighestEditableRoot(visiblePosition.DeepEquivalent());
    return highestRoot ? VisiblePosition(dom::FirstPositionInNode(highestRoot.get())) : VisiblePosition {};
  }

  VisiblePosition EndOfEditableContent(const VisiblePosition &visiblePosition) noexcept
  {
    auto highestRoot = HighestEditableRoot(visiblePosition.DeepEquivalent());
    return highestRoot ? VisiblePosition(dom::LastPositionInNode(highestRoot.get())) : VisiblePosition {};
  }

  bool IsEndOfEditableOrNonEditableContent(const VisiblePosition &p) noexcept
  {
    return p.IsNotNull() && p.Next().IsNull();
  }

  VisiblePosition LeftBoundaryOfLine(const VisiblePosition &c, TextDirection direction,
                                     bool *reachedBoundary) noexcept
  {
    return direction == TextDirection::LTR ? LogicalStartOfLine(c, reachedBoundary)
                                           : LogicalEndOfLine(c, reachedBoundary);
  }

  VisiblePosition RightBoundaryOfLine(const VisiblePosition &c, TextDirection direction,
                                      bool *reachedBoundary) noexcept
  {
    return direction == TextDirection::LTR ? LogicalEndOfLine(c, reachedBoundary)
                                           : LogicalStartOfLine(c, reachedBoundary);
  }
}
