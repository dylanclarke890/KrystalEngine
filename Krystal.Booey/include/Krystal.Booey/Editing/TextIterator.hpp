#pragma once

#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/CharacterRange.hpp"
#include "Krystal.Booey/Editing/TextIteratorBehavior.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Node;
  class Text;
}

namespace krys::boo
{
  class RenderObject;
  class RenderText;
  class RenderTextFragment;

  // Character ranges based on characters from the text iterator. Characters are bytes of its UTF-8 text.
  KRYS_NODISCARD uint64 CharacterCount(const dom::SimpleRange &range,
                                       TextIteratorBehaviors behaviors = {}) noexcept;
  KRYS_NODISCARD CharacterRange GetCharacterRange(const dom::BoundaryPoint &start,
                                                  const dom::SimpleRange &range,
                                                  TextIteratorBehaviors behaviors = {}) noexcept;
  KRYS_NODISCARD CharacterRange GetCharacterRange(const dom::SimpleRange &scope,
                                                  const dom::SimpleRange &range,
                                                  TextIteratorBehaviors behaviors = {}) noexcept;
  KRYS_NODISCARD dom::BoundaryPoint ResolveCharacterLocation(const dom::SimpleRange &scope, uint64 location,
                                                             TextIteratorBehaviors behaviors = {}) noexcept;
  KRYS_NODISCARD dom::SimpleRange ResolveCharacterRange(const dom::SimpleRange &scope, CharacterRange range,
                                                        TextIteratorBehaviors behaviors = {}) noexcept;

  // Text from the text iterator.
  KRYS_NODISCARD utf8_string PlainText(const dom::SimpleRange &range, TextIteratorBehaviors behaviors = {},
                                       bool isDisplayString = false) noexcept;

  enum class IgnoreCollapsedRanges : bool
  {
    No,
    Yes
  };

  KRYS_NODISCARD bool
    HasAnyPlainText(const dom::SimpleRange &range, TextIteratorBehaviors behaviors = {},
                    IgnoreCollapsedRanges ignoreCollapsedRanges = IgnoreCollapsedRanges::No) noexcept;
  KRYS_NODISCARD utf8_string PlainTextReplacingNoBreakSpace(const dom::SimpleRange &range,
                                                            TextIteratorBehaviors behaviors = {},
                                                            bool isDisplayString = false) noexcept;

  /// @brief @p stringToFold with its quotation marks (curly, low, angle and corner quotes, primes) replaced
  /// by
  /// `'` and `"`, so searches match either.
  /// @param sourceOffsets If given, receives the offset in @p stringToFold of each byte of the result, and of
  /// its end.
  KRYS_NODISCARD utf8_string FoldQuoteMarks(utf8_stringview stringToFold,
                                            List<uint32> *sourceOffsets = nullptr) noexcept;

  // FIXME: Move this somewhere else in the editing directory. It doesn't belong in the header with
  // TextIterator.
  KRYS_NODISCARD bool IsRendererReplacedElement(const RenderObject *renderer,
                                                TextIteratorBehaviors behaviors = {}) noexcept;

  // FIXME: Move each iterator class into a separate header file.

  /// @brief A stack of bits.
  class BitStack
  {
  private:
    uint32 _size {0};
    List<uint32> _words;

  public:
    void Push(bool bit) noexcept;
    void Pop() noexcept;
    KRYS_NODISCARD bool Top() const noexcept;
  };

  /// @brief The text a text iterator emits: one character, or part of a renderer's text.
  class TextIteratorCopyableText
  {
  private:
    char8 _singleCharacter[4] {};
    uint32 _singleCharacterLength {0};
    utf8_string _string;
    uint32 _offset {0};
    uint32 _length {0};

  public:
    KRYS_NODISCARD utf8_stringview Text() const noexcept
    {
      if (_singleCharacterLength)
      {
        return utf8_stringview(_singleCharacter, _singleCharacterLength);
      }
      return utf8_stringview(_string).substr(_offset, _length);
    }

    void AppendToString(utf8_string &builder) const noexcept;
    void Reset() noexcept;
    void Set(utf8_string &&string) noexcept;
    void Set(utf8_string &&string, uint32 offset, uint32 length) noexcept;
    void Set(char32 singleCharacter) noexcept;
  };

  KRYS_NODISCARD bool ShouldEmitNewlinesBeforeAndAfterNode(dom::Node &node) noexcept;

  /// @brief Iterates through the DOM range, returning all the text, and 0-length boundaries at points where
  /// replaced elements break up the text flow. The text is delivered in the chunks it's already stored in, to
  /// avoid copying any text.
  class TextIterator
  {
  private:
    const TextIteratorBehaviors _behaviors;

    // Current position, not necessarily of the text being returned, but position as we walk through the DOM
    // tree.
    RefPtr<dom::Node> _currentNode;
    int32 _offset {0};
    bool _handledNode {false};
    bool _handledChildren {false};
    BitStack _fullyClippedStack;

    // The range.
    RefPtr<dom::Node> _startContainer;
    int32 _startOffset {0};
    RefPtr<dom::Node> _endContainer;
    int32 _endOffset {0};
    RefPtr<dom::Node> _pastEndNode;

    // The current text and its position, in the form to be returned from the iterator.
    RefPtr<dom::Node> _positionNode;
    mutable RefPtr<dom::Node> _positionOffsetBaseNode;
    mutable int32 _positionStartOffset {0};
    mutable int32 _positionEndOffset {0};
    TextIteratorCopyableText _copyableText;
    utf8_stringview _text;

    // Used when there is still some pending text from the current node; when these are false and null, we go
    // back to normal iterating.
    RefPtr<dom::Node> _nodeForAdditionalNewline;
    inline_iterator::TextBoxIterator _textRun;
    inline_iterator::TextLogicalOrderCache _textRunLogicalOrderCache;

    // Used when iterating over :first-letter text to save pointer to remaining text box.
    inline_iterator::TextBoxIterator _remainingTextRun;
    inline_iterator::TextLogicalOrderCache _remainingTextRunLogicalOrderCache;

    // Used to point to RenderText object for :first-letter.
    WeakPtr<RenderText> _firstLetterText;

    // Used to do the whitespace collapsing logic.
    RefPtr<dom::Text> _lastTextNode;
    bool _lastTextNodeEndedWithCollapsedSpace {false};
    char32 _lastCharacter {0};

    // Used when deciding whether to emit a "positioning" (e.g. newline) before any other content
    bool _hasEmitted {false};

    // Used when deciding text fragment created by :first-letter should be looked into.
    bool _handledFirstLetter {false};

  public:
    explicit TextIterator(const dom::SimpleRange &range, TextIteratorBehaviors behaviors = {}) noexcept;
    ~TextIterator() noexcept;

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_positionNode;
    }
    void Advance() noexcept;

    KRYS_NODISCARD utf8_stringview Text() const noexcept
    {
      krys_debug_assert(!AtEnd());
      return _text;
    }
    KRYS_NODISCARD dom::SimpleRange Range() const noexcept;
    KRYS_NODISCARD dom::Node *GetNode() const noexcept;

    KRYS_NODISCARD const TextIteratorCopyableText &CopyableText() const noexcept
    {
      krys_debug_assert(!AtEnd());
      return _copyableText;
    }
    void AppendTextToString(utf8_string &builder) const noexcept
    {
      CopyableText().AppendToString(builder);
    }

    KRYS_NODISCARD utf8_string RendererTextForBehavior(const RenderText &renderer) const noexcept;

  private:
    void Init() noexcept;
    void ExitNode(dom::Node *exitedNode) noexcept;
    KRYS_NODISCARD bool ShouldRepresentNodeOffsetZero() noexcept;
    KRYS_NODISCARD bool ShouldEmitSpaceBeforeAndAfterNode(dom::Node &node) noexcept;
    void RepresentNodeOffsetZero() noexcept;
    KRYS_NODISCARD bool HandleTextNode() noexcept;
    KRYS_NODISCARD bool HandleReplacedElement() noexcept;
    KRYS_NODISCARD bool HandleNonTextNode() noexcept;
    void HandleTextRun() noexcept;
    void HandleTextNodeFirstLetter(RenderTextFragment &renderer) noexcept;
    void EmitCharacter(char32 character, RefPtr<dom::Node> &&characterNode,
                       RefPtr<dom::Node> &&offsetBaseNode, int32 textStartOffset,
                       int32 textEndOffset) noexcept;
    void EmitText(dom::Text &textNode, const RenderText &renderer, int32 textStartOffset,
                  int32 textEndOffset) noexcept;
    void RevertToRemainingTextRun() noexcept;
  };

  /// @brief Iterates through the DOM range, returning all the text, and 0-length boundaries at points where
  /// replaced elements break up the text flow. The text comes back in chunks so as to optimize for
  /// performance of the iteration.
  class SimplifiedBackwardsTextIterator
  {
  private:
    const TextIteratorBehaviors _behaviors;

    // Current position, not necessarily of the text being returned, but position as we walk through the DOM
    // tree.
    RefPtr<dom::Node> _node;
    int32 _offset {0};
    bool _handledNode {false};
    bool _handledChildren {false};
    BitStack _fullyClippedStack;

    // The range.
    RefPtr<dom::Node> _startContainer;
    int32 _startOffset {0};
    RefPtr<dom::Node> _endContainer;
    int32 _endOffset {0};

    // The current text and its position, in the form to be returned from the iterator.
    RefPtr<dom::Node> _positionNode;
    int32 _positionStartOffset {0};
    int32 _positionEndOffset {0};
    TextIteratorCopyableText _copyableText;
    utf8_stringview _text;

    // Used to do the whitespace logic.
    RefPtr<dom::Text> _lastTextNode;
    char32 _lastCharacter {0};

    // Whether _node has advanced beyond the iteration range (i.e. _startContainer).
    bool _havePassedStartContainer {false};

    // Should handle first-letter renderer in the next call to handleTextNode.
    bool _shouldHandleFirstLetter {false};

  public:
    explicit SimplifiedBackwardsTextIterator(const dom::SimpleRange &range) noexcept;

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_positionNode;
    }
    void Advance() noexcept;

    KRYS_NODISCARD utf8_stringview Text() const noexcept
    {
      krys_debug_assert(!AtEnd());
      return _text;
    }
    KRYS_NODISCARD dom::SimpleRange Range() const noexcept;
    KRYS_NODISCARD dom::Node *GetNode() const noexcept
    {
      krys_debug_assert(!AtEnd());
      return _node.get();
    }

  private:
    void ExitNode() noexcept;
    KRYS_NODISCARD bool HandleTextNode() noexcept;
    KRYS_NODISCARD const RenderText *HandleFirstLetter(int32 &startOffset, int32 &offsetInNode) noexcept;
    KRYS_NODISCARD bool HandleReplacedElement() noexcept;
    KRYS_NODISCARD bool HandleNonTextNode() noexcept;
    void EmitCharacter(char32 character, RefPtr<dom::Node> &&node, int32 startOffset,
                       int32 endOffset) noexcept;
    KRYS_NODISCARD bool AdvanceRespectingRange(dom::Node *next) noexcept;
  };

  /// @brief Builds on the text iterator, adding a character position so we can walk one character (byte) at a
  /// time, or faster, as needed. Useful for searching.
  class CharacterIterator
  {
  private:
    TextIterator _underlyingIterator;
    uint32 _offset {0};
    uint32 _runOffset {0};
    bool _atBreak {true};

  public:
    explicit CharacterIterator(const dom::SimpleRange &range, TextIteratorBehaviors behaviors = {}) noexcept;

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return _underlyingIterator.AtEnd();
    }
    void Advance(int32 numCharacters) noexcept;

    KRYS_NODISCARD utf8_stringview Text() const noexcept
    {
      return _underlyingIterator.Text().substr(_runOffset);
    }
    KRYS_NODISCARD dom::SimpleRange Range() const noexcept;

    KRYS_NODISCARD bool AtBreak() const noexcept
    {
      return _atBreak;
    }
    KRYS_NODISCARD uint32 CharacterOffset() const noexcept
    {
      return _offset;
    }
  };

  class BackwardsCharacterIterator
  {
  private:
    SimplifiedBackwardsTextIterator _underlyingIterator;
    uint32 _offset {0};
    uint32 _runOffset {0};
    bool _atBreak {true};

  public:
    explicit BackwardsCharacterIterator(const dom::SimpleRange &range) noexcept;

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return _underlyingIterator.AtEnd();
    }
    void Advance(int32 numCharacters) noexcept;

    KRYS_NODISCARD utf8_stringview Text() const noexcept
    {
      return _underlyingIterator.Text().substr(0, _underlyingIterator.Text().size() - _runOffset);
    }
    KRYS_NODISCARD dom::SimpleRange Range() const noexcept;
  };

  /// @brief Similar to the TextIterator, except that the chunks of text returned are "well behaved", meaning
  /// they never split up a word. This is useful for spell checking and perhaps one day for searching as well.
  class WordAwareIterator
  {
  private:
    TextIterator _underlyingIterator;
    // Text from the previous chunk from the text iterator.
    TextIteratorCopyableText _previousText;
    // Many chunks from text iterator concatenated.
    utf8_string _buffer;
    // Did we have to look ahead in the text iterator to confirm the current chunk?
    bool _didLookAhead {true};

  public:
    explicit WordAwareIterator(const dom::SimpleRange &range) noexcept;

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_didLookAhead && _underlyingIterator.AtEnd();
    }
    void Advance() noexcept;
    KRYS_NODISCARD utf8_stringview Text() const noexcept;
  };

  KRYS_NODISCARD inline CharacterRange GetCharacterRange(const dom::BoundaryPoint &start,
                                                         const dom::SimpleRange &range,
                                                         TextIteratorBehaviors behaviors) noexcept
  {
    return {CharacterCount({start, range.Start}, behaviors), CharacterCount(range, behaviors)};
  }

  KRYS_NODISCARD inline CharacterRange GetCharacterRange(const dom::SimpleRange &scope,
                                                         const dom::SimpleRange &range,
                                                         TextIteratorBehaviors behaviors) noexcept
  {
    return GetCharacterRange(scope.Start, range, behaviors);
  }

  KRYS_NODISCARD inline dom::BoundaryPoint ResolveCharacterLocation(const dom::SimpleRange &scope,
                                                                    uint64 location,
                                                                    TextIteratorBehaviors behaviors) noexcept
  {
    return ResolveCharacterRange(scope, {location, 0}, behaviors).Start;
  }
}
