#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

struct UBreakIterator;

// Every offset here is a byte offset into UTF-8 text.

namespace krys::text
{
  /// @brief How strictly line breaks follow the line breaking rules (`line-break`).
  enum class TextBreakLineBehavior : uint8
  {
    Default,
    Loose,
    Normal,
    Strict,
  };

  /// @brief The boundaries a break iterator finds, as UAX #29 and UAX #14 define them.
  enum class TextBreakUnit : uint8
  {
    /// @brief Extended grapheme clusters: what a user sees as one character, and the caret steps over.
    GraphemeCluster,
    Word,
    Sentence,
    /// @brief Line break opportunities.
    Line,
  };

  /// @brief Finds the boundaries of a unit of text with ICU. It moves through them like ICU's break
  /// iterators: each move makes the boundary it reaches the current one.
  /// @note The text (but not the prior context, which is copied) must outlive the iterator. Iterators are
  /// pooled: making one reuses a released iterator for the same unit and locale when there is one.
  class TextBreakIterator
  {
    KRYS_NON_COPYABLE(TextBreakIterator);

  private:
    UBreakIterator *_iterator {nullptr};
    TextBreakUnit _unit {TextBreakUnit::GraphemeCluster};
    // The locale ICU opened the iterator with, with any line breaking keyword.
    utf8_string _localeId;
    // The prior context followed by the text, when there's a prior context: ICU needs them together.
    List<char8> _textWithPriorContext;
    uint32 _priorContextLength {0};

  public:
    /// @brief A null iterator, with no text.
    TextBreakIterator() noexcept = default;

    /// @param priorContext The text that comes before @p text: it affects the boundaries at the start of
    /// @p text, but isn't part of it.
    TextBreakIterator(utf8_stringview text, TextBreakUnit unit, const StringAtom &locale = null,
                      TextBreakLineBehavior lineBehavior = TextBreakLineBehavior::Default,
                      utf8_stringview priorContext = {}) noexcept;

    TextBreakIterator(TextBreakIterator &&other) noexcept;

    TextBreakIterator &operator=(TextBreakIterator &&other) noexcept;

    ~TextBreakIterator() noexcept;

    /// @brief Whether ICU made the iterator.
    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _iterator;
    }

    /// @brief Moves to the start of the text, and returns it.
    uint32 First() const noexcept;

    /// @brief Moves to the end of the text, and returns it.
    uint32 Last() const noexcept;

    /// @brief Moves to the next boundary, if there is one.
    Maybe<uint32> Next() const noexcept;

    /// @brief Moves to the previous boundary, if there is one.
    Maybe<uint32> Previous() const noexcept;

    /// @brief Moves to the first boundary after @p offset, if there is one.
    /// @note Offsets wrap around, so `uint32(-1)` is the last byte of the prior context.
    Maybe<uint32> Following(uint32 offset) const noexcept;

    /// @brief Moves to the last boundary before @p offset, if there is one. A boundary in the prior context
    /// counts as the start of the text.
    Maybe<uint32> Preceding(uint32 offset) const noexcept;

    /// @brief Whether @p offset is a boundary. Moves to it if it is, or else to the first boundary after it.
    bool IsBoundary(uint32 offset) const noexcept;

    /// @brief The current boundary.
    KRYS_NODISCARD uint32 Current() const noexcept;

    /// @brief For words: whether the text before the current boundary is a word, rather than spaces or
    /// punctuation.
    KRYS_NODISCARD bool IsWordBreak() const noexcept;

  private:
    void SetText(utf8_stringview text, utf8_stringview priorContext) noexcept;

    KRYS_NODISCARD Maybe<uint32> FromICU(int32 index) const noexcept;

    void Release() noexcept;
  };

  /// @brief A word break iterator over @p text, in the default locale.
  KRYS_NODISCARD TextBreakIterator WordBreakIterator(utf8_stringview text) noexcept;

  /// @brief A sentence break iterator over @p text, in the default locale.
  KRYS_NODISCARD TextBreakIterator SentenceBreakIterator(utf8_stringview text) noexcept;

  /// @brief Makes a line break iterator over a string when it's first needed, and again when the code points
  /// before the string change.
  class CachedLineBreakIteratorFactory
  {
  public:
    /// @brief The last code points before the string, which affect its first break opportunities.
    class PriorContext
    {
    public:
      constexpr static uint32 Length = 2;

    private:
      // The code points, the last one at the end; unused leading ones are 0.
      Array<char32, Length> _codePoints {};
      Array<char8, Length * 4> _encoded {};
      uint32 _encodedLength {0};

    public:
      KRYS_NODISCARD char32 LastCharacter() const noexcept
      {
        return _codePoints[Length - 1];
      }

      KRYS_NODISCARD char32 SecondToLastCharacter() const noexcept
      {
        return _codePoints[Length - 2];
      }

      void Set(Array<char32, Length> codePoints) noexcept;

      /// @brief Appends @p last, dropping the oldest code point.
      void Update(char32 last) noexcept;

      void Reset() noexcept;

      /// @brief The number of code points in the prior context.
      KRYS_NODISCARD uint32 GetLength() const noexcept;

      /// @brief The prior context, UTF-8 encoded.
      KRYS_NODISCARD utf8_stringview Characters() const noexcept
      {
        return {_encoded.data(), _encodedLength};
      }

      KRYS_NODISCARD friend bool operator==(const PriorContext &a, const PriorContext &b) noexcept
      {
        return a._codePoints == b._codePoints;
      }

    private:
      void Encode() noexcept;
    };

  private:
    utf8_stringview _string;
    StringAtom _locale {null};
    TextBreakLineBehavior _mode {TextBreakLineBehavior::Default};
    PriorContext _priorContext;
    // The prior context the iterator was made with.
    PriorContext _iteratorPriorContext;
    Maybe<TextBreakIterator> _iterator;

  public:
    CachedLineBreakIteratorFactory() noexcept = default;

    explicit CachedLineBreakIteratorFactory(
      utf8_stringview string, StringAtom locale = null,
      TextBreakLineBehavior mode = TextBreakLineBehavior::Default) noexcept
        : _string(string), _locale(krys::move(locale)), _mode(mode)
    {
    }

    KRYS_NODISCARD utf8_stringview StringView() const noexcept
    {
      return _string;
    }

    KRYS_NODISCARD TextBreakLineBehavior Mode() const noexcept
    {
      return _mode;
    }

    /// @brief The iterator for the string and the current prior context.
    KRYS_NODISCARD TextBreakIterator &operator*() noexcept;

    KRYS_NODISCARD TextBreakIterator *operator->() noexcept
    {
      return &**this;
    }

    KRYS_NODISCARD const PriorContext &GetPriorContext() const noexcept
    {
      return _priorContext;
    }

    KRYS_NODISCARD PriorContext &GetPriorContext() noexcept
    {
      return _priorContext;
    }
  };

  /// @brief The number of extended grapheme clusters in @p text.
  /// @see https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
  KRYS_NODISCARD uint32 NumGraphemeClusters(utf8_stringview text) noexcept;

  /// @brief The number of bytes in the first @p count grapheme clusters of @p text, or its length if it has
  /// fewer.
  KRYS_NODISCARD uint32 NumCodeUnitsInGraphemeClusters(utf8_stringview text, uint32 count) noexcept;
}
