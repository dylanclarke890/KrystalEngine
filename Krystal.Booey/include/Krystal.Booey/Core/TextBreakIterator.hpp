#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/Variant.hpp"

struct UBreakIterator;

// Every offset here is a byte offset into UTF-8 text.

namespace krys::boo
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

    static constexpr uint32 Capacity = 2;

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

  // FIXME: Delete this in favor of CachedTextBreakIterator.
  KRYS_NODISCARD UBreakIterator *WordBreakIterator(utf8_stringview string) noexcept;
  // FIXME: Delete this in favor of CachedTextBreakIterator.
  KRYS_NODISCARD UBreakIterator *SentenceBreakIterator(utf8_stringview string) noexcept;

  KRYS_NODISCARD bool IsWordTextBreak(UBreakIterator *iterator) noexcept;

  /// @brief Makes line break iterators over a string on demand, with the code points that precede the string.
  class CachedLineBreakIteratorFactory
  {
  public:
    /// @brief The last code points before the string, which affect its first break opportunities.
    class PriorContext
    {
    public:
      static constexpr uint32 Length = 2;

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
    StringAtom _locale;
    Maybe<CachedTextBreakIterator> _iterator;
    const char8 *_cachedPriorContext {nullptr};
    TextBreakLineBehavior _mode {TextBreakLineBehavior::Default};
    TextBreakIterator::ContentAnalysis _contentAnalysis {TextBreakIterator::ContentAnalysis::Mechanical};
    PriorContext _priorContext;

  public:
    CachedLineBreakIteratorFactory() noexcept = default;

    explicit CachedLineBreakIteratorFactory(utf8_stringview stringView,
                                            const StringAtom &locale = StringAtom::Null(),
                                            TextBreakLineBehavior mode = TextBreakLineBehavior::Default,
                                            TextBreakIterator::ContentAnalysis contentAnalysis =
                                              TextBreakIterator::ContentAnalysis::Mechanical) noexcept
        : _stringView(stringView), _locale(locale), _mode(mode), _contentAnalysis(contentAnalysis)
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

    KRYS_NODISCARD CachedTextBreakIterator &Get() noexcept;

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
  class NonSharedCharacterBreakIterator
  {
    KRYS_NON_COPYABLE(NonSharedCharacterBreakIterator);

  private:
    UBreakIterator *_iterator;

  public:
    explicit NonSharedCharacterBreakIterator(utf8_stringview string) noexcept;
    ~NonSharedCharacterBreakIterator() noexcept;

    NonSharedCharacterBreakIterator(NonSharedCharacterBreakIterator &&other) noexcept;

    operator UBreakIterator *() const noexcept
    {
      return _iterator;
    }
  };

  /// @brief Iterates over sentences, as defined in UAX #29.
  class NonSharedSentenceBreakIterator
  {
    KRYS_NON_COPYABLE(NonSharedSentenceBreakIterator);

  private:
    UBreakIterator *_iterator;

  public:
    explicit NonSharedSentenceBreakIterator(utf8_stringview string) noexcept;
    ~NonSharedSentenceBreakIterator() noexcept;

    NonSharedSentenceBreakIterator(NonSharedSentenceBreakIterator &&other) noexcept;

    operator UBreakIterator *() const noexcept
    {
      return _iterator;
    }
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
