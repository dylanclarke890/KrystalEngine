/*
 * Copyright (C) 2006 Lars Knoll <lars@trolltech.com>
 * Copyright (C) 2007-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Utils/Move.hpp"

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

  /// @brief Finds text boundaries with an ICU break iterator. The text can be preceded by up to two code
  /// points of prior context, which affect the boundaries but aren't part of the text.
  class TextBreakIteratorICU
  {
  public:
    struct LineMode
    {
      TextBreakLineBehavior Behavior;
    };

    struct CharacterMode
    {
    };

    using Mode = Variant<LineMode, CharacterMode>;

  private:
    UBreakIterator *_iterator {nullptr};
    uint32 _priorContextLength {0};
    // The prior context followed by the text, when there's a prior context: the iterator needs them together.
    // A list keeps its characters in place when it's moved (a short string doesn't).
    List<char8> _textWithPriorContext;

  public:
    TextBreakIteratorICU(utf8_stringview string, utf8_stringview priorContext, Mode mode,
                         const StringAtom &locale) noexcept;

    TextBreakIteratorICU() = delete;

    TextBreakIteratorICU(const TextBreakIteratorICU &) = delete;

    TextBreakIteratorICU &operator=(const TextBreakIteratorICU &) = delete;

    TextBreakIteratorICU(TextBreakIteratorICU &&other) noexcept;

    TextBreakIteratorICU &operator=(TextBreakIteratorICU &&other) noexcept;

    ~TextBreakIteratorICU() noexcept;

    /// @note @p string must outlive the iterator, or the next `SetText`.
    void SetText(utf8_stringview string, utf8_stringview priorContext) noexcept;

    KRYS_NODISCARD Maybe<uint32> Preceding(uint32 location) const noexcept;

    KRYS_NODISCARD Maybe<uint32> Following(uint32 location) const noexcept;

    KRYS_NODISCARD bool IsBoundary(uint32 location) const noexcept;

  private:
    KRYS_NODISCARD static StringAtom MakeLocaleWithBreakKeyword(const StringAtom &locale,
                                                                TextBreakLineBehavior behavior) noexcept;
  };

  class TextBreakIteratorCache;

  /// @brief Finds line break opportunities or grapheme cluster boundaries. Use `CachedTextBreakIterator` to
  /// make one.
  class TextBreakIterator
  {
    friend class CachedTextBreakIterator;
    friend class TextBreakIteratorCache;

  public:
    struct LineMode
    {
      TextBreakLineBehavior Behavior;

      friend bool operator==(LineMode, LineMode) = default;
    };

    struct CaretMode
    {
      friend bool operator==(CaretMode, CaretMode) = default;
    };

    struct DeleteMode
    {
      friend bool operator==(DeleteMode, DeleteMode) = default;
    };

    struct CharacterMode
    {
      friend bool operator==(CharacterMode, CharacterMode) = default;
    };

    using Mode = Variant<LineMode, CaretMode, DeleteMode, CharacterMode>;

    enum class ContentAnalysis : bool
    {
      Linguistic,
      Mechanical,
    };

  private:
    TextBreakIteratorICU _backing;
    Mode _mode;
    ContentAnalysis _contentAnalysis {ContentAnalysis::Mechanical};
    StringAtom _locale;

    // Use CachedTextBreakIterator instead of constructing one of these directly.
    TextBreakIterator(utf8_stringview string, utf8_stringview priorContext, Mode mode,
                      ContentAnalysis contentAnalysis, const StringAtom &locale) noexcept;

  public:
    TextBreakIterator() = delete;

    TextBreakIterator(const TextBreakIterator &) = delete;

    TextBreakIterator(TextBreakIterator &&) noexcept = default;

    TextBreakIterator &operator=(const TextBreakIterator &) = delete;

    TextBreakIterator &operator=(TextBreakIterator &&) noexcept = default;

    KRYS_NODISCARD Maybe<uint32> Preceding(uint32 location) const noexcept
    {
      return _backing.Preceding(location);
    }

    KRYS_NODISCARD Maybe<uint32> Following(uint32 location) const noexcept
    {
      return _backing.Following(location);
    }

    KRYS_NODISCARD bool IsBoundary(uint32 location) const noexcept
    {
      return _backing.IsBoundary(location);
    }

  private:
    KRYS_NODISCARD static TextBreakIteratorICU MapModeToBackingIterator(utf8_stringview string,
                                                                        utf8_stringview priorContext,
                                                                        Mode mode, ContentAnalysis,
                                                                        const StringAtom &locale) noexcept;

    void SetText(utf8_stringview string, utf8_stringview priorContext) noexcept
    {
      _backing.SetText(string, priorContext);
    }

    KRYS_NODISCARD const Mode &GetMode() const noexcept
    {
      return _mode;
    }

    KRYS_NODISCARD ContentAnalysis GetContentAnalysis() const noexcept
    {
      return _contentAnalysis;
    }

    KRYS_NODISCARD const StringAtom &GetLocale() const noexcept
    {
      return _locale;
    }
  };

  /// @brief Keeps the last few break iterators for reuse. Use `CachedTextBreakIterator` instead of dealing
  /// with the cache directly.
  class TextBreakIteratorCache
  {
    friend class CachedTextBreakIterator;
    KRYS_NON_COPY_MOVABLE(TextBreakIteratorCache);

    constexpr static uint32 Capacity = 2;

    // FIXME: Break this up into different Vectors per mode.
    SmallList<TextBreakIterator, Capacity> _unused;

  public:
    TextBreakIteratorCache() noexcept = default;

  private:
    KRYS_NODISCARD static TextBreakIteratorCache &Singleton() noexcept;

    KRYS_NODISCARD TextBreakIterator Take(utf8_stringview string, utf8_stringview priorContext,
                                          TextBreakIterator::Mode mode,
                                          TextBreakIterator::ContentAnalysis contentAnalysis,
                                          const StringAtom &locale) noexcept;

    void Put(TextBreakIterator &&iterator) noexcept;
  };

  /// @brief A break iterator from the cache, returned to it on destruction.
  class CachedTextBreakIterator
  {
    KRYS_NON_COPYABLE(CachedTextBreakIterator);

  private:
    Maybe<TextBreakIterator> _backing;

  public:
    CachedTextBreakIterator(utf8_stringview string, utf8_stringview priorContext,
                            TextBreakIterator::Mode mode, const StringAtom &locale,
                            TextBreakIterator::ContentAnalysis contentAnalysis =
                              TextBreakIterator::ContentAnalysis::Mechanical) noexcept;

    ~CachedTextBreakIterator() noexcept;

    CachedTextBreakIterator() = delete;

    CachedTextBreakIterator(CachedTextBreakIterator &&other) noexcept;

    CachedTextBreakIterator &operator=(CachedTextBreakIterator &&other) noexcept;

    KRYS_NODISCARD Maybe<uint32> Preceding(uint32 location) const noexcept
    {
      return _backing->Preceding(location);
    }

    KRYS_NODISCARD Maybe<uint32> Following(uint32 location) const noexcept
    {
      return _backing->Following(location);
    }

    KRYS_NODISCARD bool IsBoundary(uint32 location) const noexcept
    {
      return _backing->IsBoundary(location);
    }
  };

  class BreakIteratorView;

  // FIXME: Delete this in favor of CachedTextBreakIterator.
  /// @brief The shared word break iterator, set to @p string; null if ICU couldn't make one.
  /// @note It's shared: the next call resets it.
  KRYS_NODISCARD BreakIteratorView WordBreakIterator(utf8_stringview string) noexcept;

  // FIXME: Delete this in favor of CachedTextBreakIterator.
  /// @brief The shared sentence break iterator, set to @p string; null if ICU couldn't make one.
  /// @note It's shared: the next call resets it.
  KRYS_NODISCARD BreakIteratorView SentenceBreakIterator(utf8_stringview string) noexcept;

  /// @brief A view of a break iterator over some text. Its boundaries are byte offsets into the text; moving
  /// to one makes it the current boundary.
  class BreakIteratorView
  {
    friend BreakIteratorView WordBreakIterator(utf8_stringview string) noexcept;
    friend BreakIteratorView SentenceBreakIterator(utf8_stringview string) noexcept;

  protected:
    UBreakIterator *_iterator {nullptr};

    constexpr explicit BreakIteratorView(UBreakIterator *iterator) noexcept : _iterator(iterator)
    {
    }

  public:
    /// @brief A null view.
    constexpr BreakIteratorView() noexcept = default;

    constexpr explicit operator bool() const noexcept
    {
      return _iterator;
    }

    /// @brief Moves to the start of the text, and returns it (0).
    uint32 First() const noexcept;

    /// @brief Moves to the end of the text, and returns it.
    uint32 Last() const noexcept;

    /// @brief Moves to the next boundary, if there is one.
    Maybe<uint32> Next() const noexcept;

    /// @brief Moves to the previous boundary, if there is one.
    Maybe<uint32> Previous() const noexcept;

    /// @brief Moves to the first boundary after @p offset, if there is one.
    Maybe<uint32> Following(uint32 offset) const noexcept;

    /// @brief Moves to the last boundary before @p offset, if there is one.
    Maybe<uint32> Preceding(uint32 offset) const noexcept;

    /// @brief Whether @p offset is a boundary. Moves to it if it is, or to the first boundary after it.
    bool IsBoundary(uint32 offset) const noexcept;

    /// @brief The current boundary.
    KRYS_NODISCARD uint32 Current() const noexcept;

    /// @brief For a word break iterator: whether the text before the current boundary is a word, rather than
    /// spaces or punctuation.
    KRYS_NODISCARD bool IsWordBreak() const noexcept;
  };

  /// @brief Makes line break iterators over a string on demand, with the code points that precede the string.
  class CachedLineBreakIteratorFactory
  {
  public:
    /// @brief The last code points before the string, which affect its first break opportunities.
    class PriorContext
    {
    public:
      constexpr static uint32 Length = 2;

    private:
      Array<char32, Length> _priorContext;
      // The prior context, UTF-8 encoded, at the end of the buffer.
      Array<char8, Length * 4> _encoded;
      uint32 _encodedLength {0};

    public:
      PriorContext() noexcept
      {
        Reset();
      }

      KRYS_NODISCARD char32 LastCharacter() const noexcept
      {
        static_assert(Length >= 1);
        return _priorContext[_priorContext.size() - 1];
      }

      KRYS_NODISCARD char32 SecondToLastCharacter() const noexcept
      {
        static_assert(Length >= 2);
        return _priorContext[_priorContext.size() - 2];
      }

      void Set(Array<char32, Length> &&newPriorContext) noexcept;

      void Update(char32 last) noexcept;

      void Reset() noexcept;

      /// @brief The number of code points in the prior context.
      KRYS_NODISCARD uint32 GetLength() const noexcept;

      /// @brief The prior context, UTF-8 encoded.
      KRYS_NODISCARD utf8_stringview Characters() const noexcept
      {
        return {_encoded.data() + _encoded.size() - _encodedLength, _encodedLength};
      }

      friend bool operator==(const PriorContext &a, const PriorContext &b) noexcept
      {
        return a._priorContext == b._priorContext;
      }

    private:
      void Encode() noexcept;
    };

  private:
    utf8_stringview _stringView;
    StringAtom _locale {null};
    Maybe<CachedTextBreakIterator> _iterator;
    const char8 *_cachedPriorContext {nullptr};
    TextBreakLineBehavior _mode {TextBreakLineBehavior::Default};
    TextBreakIterator::ContentAnalysis _contentAnalysis {TextBreakIterator::ContentAnalysis::Mechanical};
    PriorContext _priorContext;

  public:
    CachedLineBreakIteratorFactory() noexcept = default;

    explicit CachedLineBreakIteratorFactory(utf8_stringview stringView, StringAtom locale = null,
                                            TextBreakLineBehavior mode = TextBreakLineBehavior::Default,
                                            TextBreakIterator::ContentAnalysis contentAnalysis =
                                              TextBreakIterator::ContentAnalysis::Mechanical) noexcept
        : _stringView(stringView), _locale(krys::move(locale)), _mode(mode), _contentAnalysis(contentAnalysis)
    {
    }

    KRYS_NODISCARD utf8_stringview StringView() const noexcept
    {
      return _stringView;
    }

    KRYS_NODISCARD TextBreakLineBehavior Mode() const noexcept
    {
      return _mode;
    }

    KRYS_NODISCARD TextBreakIterator::ContentAnalysis GetContentAnalysis() const noexcept
    {
      return _contentAnalysis;
    }

    KRYS_NODISCARD CachedTextBreakIterator &operator*() noexcept;

    KRYS_NODISCARD CachedTextBreakIterator *operator->() noexcept
    {
      return &**this;
    }

    void ResetStringAndReleaseIterator(utf8_stringview stringView, const StringAtom &locale,
                                       TextBreakLineBehavior mode,
                                       TextBreakIterator::ContentAnalysis contentAnalysis =
                                         TextBreakIterator::ContentAnalysis::Mechanical) noexcept;

    KRYS_NODISCARD const PriorContext &GetPriorContext() const noexcept
    {
      return _priorContext;
    }

    KRYS_NODISCARD PriorContext &GetPriorContext() noexcept
    {
      return _priorContext;
    }
  };

  /// @brief Iterates over extended grapheme clusters, as defined in UAX #29. Use this for general text
  /// processing, e.g. string truncation.
  /// @see https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
  class NonSharedCharacterBreakIterator : public BreakIteratorView
  {
    KRYS_NON_COPYABLE(NonSharedCharacterBreakIterator);

  public:
    explicit NonSharedCharacterBreakIterator(utf8_stringview string) noexcept;

    ~NonSharedCharacterBreakIterator() noexcept;

    NonSharedCharacterBreakIterator(NonSharedCharacterBreakIterator &&other) noexcept;
  };

  /// @brief Iterates over sentences, as defined in UAX #29.
  class NonSharedSentenceBreakIterator : public BreakIteratorView
  {
    KRYS_NON_COPYABLE(NonSharedSentenceBreakIterator);

  public:
    explicit NonSharedSentenceBreakIterator(utf8_stringview string) noexcept;

    ~NonSharedSentenceBreakIterator() noexcept;

    NonSharedSentenceBreakIterator(NonSharedSentenceBreakIterator &&other) noexcept;
  };

  /// @brief The number of extended grapheme clusters in @p string. A sequence of a non-combining character
  /// and the combining characters that follow it is one grapheme cluster.
  /// @see https://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
  KRYS_NODISCARD uint32 NumGraphemeClusters(utf8_stringview string) noexcept;

  /// @brief The number of bytes that make up the first @p numGraphemeClusters grapheme clusters of @p string,
  /// or the length of @p string if it has fewer.
  KRYS_NODISCARD uint32 NumCodeUnitsInGraphemeClusters(utf8_stringview string,
                                                       uint32 numGraphemeClusters) noexcept;

  /// @brief The locale break iterators use when none is given: ICU's default locale.
  KRYS_NODISCARD const char *CurrentTextBreakLocaleID() noexcept;
}
