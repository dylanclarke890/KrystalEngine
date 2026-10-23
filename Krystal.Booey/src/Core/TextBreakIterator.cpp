#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>
#include <unicode/ubrk.h>
#include <unicode/uloc.h>
#include <unicode/utext.h>
#include <unicode/utf8.h>
#include <utility>

namespace krys::boo
{
  /// @brief Gives @p iterator the UTF-8 @p string. The iterator keeps a reference to the string's memory.
  KRYS_NODISCARD static bool SetUTF8Text(UBreakIterator &iterator, utf8_stringview string) noexcept
  {
    UErrorCode status = U_ZERO_ERROR;
    UText text = UTEXT_INITIALIZER;
    utext_openUTF8(&text, reinterpret_cast<const char *>(string.data()), static_cast<int64_t>(string.size()),
                   &status);
    if (U_FAILURE(status))
    {
      return false;
    }

    // The iterator makes a shallow clone of the text.
    ubrk_setUText(&iterator, &text, &status);
    utext_close(&text);
    return U_SUCCESS(status);
  }

  // MARK: - TextBreakIteratorICU

  TextBreakIteratorICU::TextBreakIteratorICU(utf8_stringview string, utf8_stringview priorContext, Mode mode,
                                             const StringAtom &locale) noexcept
  {
    auto type =
      SwitchOn(mode, [](LineMode) { return UBRK_LINE; }, [](CharacterMode) { return UBRK_CHARACTER; });

    auto localeWithOptionalBreakKeyword = SwitchOn(
      mode, [&locale](LineMode lineMode) { return MakeLocaleWithBreakKeyword(locale, lineMode.Behavior); },
      [&locale](CharacterMode) { return locale; });

    UErrorCode status = U_ZERO_ERROR;
    utf8_string localeID {localeWithOptionalBreakKeyword.View()};
    _iterator = ubrk_open(type, reinterpret_cast<const char *>(localeID.c_str()), nullptr, 0, &status);
    if (!_iterator || U_FAILURE(status))
    {
      status = U_ZERO_ERROR;
      // There's no reason for this to ever fail, unless there's an allocation failure.
      _iterator = ubrk_open(type, "", nullptr, 0, &status);
    }
    krys_assert(_iterator);
    krys_assert(U_SUCCESS(status));

    SetText(string, priorContext);
  }

  TextBreakIteratorICU::TextBreakIteratorICU(TextBreakIteratorICU &&other) noexcept
      : _iterator(std::exchange(other._iterator, nullptr)), _priorContextLength(other._priorContextLength),
        _textWithPriorContext(krys::move(other._textWithPriorContext))
  {
  }

  TextBreakIteratorICU &TextBreakIteratorICU::operator=(TextBreakIteratorICU &&other) noexcept
  {
    if (_iterator)
    {
      ubrk_close(_iterator);
    }
    _iterator = std::exchange(other._iterator, nullptr);
    _priorContextLength = other._priorContextLength;
    _textWithPriorContext = krys::move(other._textWithPriorContext);
    return *this;
  }

  TextBreakIteratorICU::~TextBreakIteratorICU() noexcept
  {
    if (_iterator)
    {
      ubrk_close(_iterator); // FIXME: Use an RAII wrapper for this
    }
  }

  void TextBreakIteratorICU::SetText(utf8_stringview string, utf8_stringview priorContext) noexcept
  {
    krys_debug_assert(_iterator);

    // ICU iterates over UTF-8 text in place, so a prior context must be copied in front of the text.
    auto text = string;
    if (priorContext.empty())
    {
      _textWithPriorContext.clear();
    }
    else
    {
      _textWithPriorContext.assign(priorContext.begin(), priorContext.end());
      _textWithPriorContext.insert(_textWithPriorContext.end(), string.begin(), string.end());
      text = utf8_stringview {_textWithPriorContext.data(), _textWithPriorContext.size()};
    }

    if (SetUTF8Text(*_iterator, text))
    {
      _priorContextLength = static_cast<uint32>(priorContext.size());
    }
    else
    {
      krys_debug_assert_msg(false, "ICU couldn't take the text");
      _priorContextLength = 0;
    }
  }

  Maybe<uint32> TextBreakIteratorICU::Preceding(uint32 location) const noexcept
  {
    if (!location)
    {
      return null;
    }
    auto result = ubrk_preceding(_iterator, static_cast<int32_t>(location + _priorContextLength));
    if (result == UBRK_DONE)
    {
      return null;
    }
    return std::max(static_cast<uint32>(result), _priorContextLength) - _priorContextLength;
  }

  Maybe<uint32> TextBreakIteratorICU::Following(uint32 location) const noexcept
  {
    auto result = ubrk_following(_iterator, static_cast<int32_t>(location + _priorContextLength));
    if (result == UBRK_DONE)
    {
      return null;
    }
    return static_cast<uint32>(result) - _priorContextLength;
  }

  bool TextBreakIteratorICU::IsBoundary(uint32 location) const noexcept
  {
    return ubrk_isBoundary(_iterator, static_cast<int32_t>(location + _priorContextLength));
  }

  StringAtom TextBreakIteratorICU::MakeLocaleWithBreakKeyword(const StringAtom &locale,
                                                              TextBreakLineBehavior behavior) noexcept
  {
    if (behavior == TextBreakLineBehavior::Default)
    {
      return locale;
    }

    // The uloc functions model locales as char*.
    auto utf8Locale = locale.View();
    if (utf8Locale.empty())
    {
      return locale;
    }

    const char *keywordValue = nullptr;
    switch (behavior)
    {
      case TextBreakLineBehavior::Default:
        // nullptr will cause any existing values to be removed.
        krys_unreachable();
        break;
      case TextBreakLineBehavior::Loose:  keywordValue = "loose"; break;
      case TextBreakLineBehavior::Normal: keywordValue = "normal"; break;
      case TextBreakLineBehavior::Strict: keywordValue = "strict"; break;
    }

    List<char> scratchBuffer(utf8Locale.size() + 11, 0);
    std::ranges::copy(utf8Locale, scratchBuffer.begin());

    UErrorCode status = U_ZERO_ERROR;
    int32_t lengthNeeded = uloc_setKeywordValue("lb", keywordValue, scratchBuffer.data(),
                                                static_cast<int32_t>(scratchBuffer.size()), &status);
    if (status == U_BUFFER_OVERFLOW_ERROR || status == U_STRING_NOT_TERMINATED_WARNING)
    {
      scratchBuffer.resize(static_cast<size_t>(lengthNeeded) + 1);
      std::fill(scratchBuffer.begin() + static_cast<ptrdiff_t>(utf8Locale.size()), scratchBuffer.end(), 0);
      status = U_ZERO_ERROR;
      int32_t lengthNeeded2 = uloc_setKeywordValue("lb", keywordValue, scratchBuffer.data(),
                                                   static_cast<int32_t>(scratchBuffer.size()), &status);
      if (!U_SUCCESS(status) || lengthNeeded != lengthNeeded2)
      {
        return locale;
      }
    }
    else if (!U_SUCCESS(status))
    {
      return locale;
    }
    return StringAtom {utf8_stringview {reinterpret_cast<const char8 *>(scratchBuffer.data()),
                                        static_cast<size_t>(lengthNeeded)}};
  }

  // MARK: - TextBreakIterator

  TextBreakIteratorICU TextBreakIterator::MapModeToBackingIterator(utf8_stringview string,
                                                                   utf8_stringview priorContext, Mode mode,
                                                                   ContentAnalysis,
                                                                   const StringAtom &locale) noexcept
  {
    return SwitchOn(
      mode,
      [&](TextBreakIterator::LineMode lineMode)
      {
        return TextBreakIteratorICU(string, priorContext, TextBreakIteratorICU::LineMode {lineMode.Behavior},
                                    locale);
      },
      [&](TextBreakIterator::CaretMode)
      { return TextBreakIteratorICU(string, priorContext, TextBreakIteratorICU::CharacterMode {}, locale); },
      [&](TextBreakIterator::DeleteMode)
      { return TextBreakIteratorICU(string, priorContext, TextBreakIteratorICU::CharacterMode {}, locale); },
      [&](TextBreakIterator::CharacterMode)
      { return TextBreakIteratorICU(string, priorContext, TextBreakIteratorICU::CharacterMode {}, locale); });
  }

  TextBreakIterator::TextBreakIterator(utf8_stringview string, utf8_stringview priorContext, Mode mode,
                                       ContentAnalysis contentAnalysis, const StringAtom &locale) noexcept
      : _backing(MapModeToBackingIterator(string, priorContext, mode, contentAnalysis, locale)), _mode(mode),
        _locale(locale)
  {
  }

  // MARK: - TextBreakIteratorCache

  TextBreakIteratorCache &TextBreakIteratorCache::Singleton() noexcept
  {
    static MainThreadNeverDestroyed<TextBreakIteratorCache> cache;
    return cache;
  }

  TextBreakIterator TextBreakIteratorCache::Take(utf8_stringview string, utf8_stringview priorContext,
                                                 TextBreakIterator::Mode mode,
                                                 TextBreakIterator::ContentAnalysis contentAnalysis,
                                                 const StringAtom &locale) noexcept
  {
    auto iter = std::ranges::find_if(_unused,
                                     [&](TextBreakIterator &candidate)
                                     {
                                       return candidate.GetMode() == mode
                                              && candidate.GetContentAnalysis() == contentAnalysis
                                              && candidate.GetLocale() == locale;
                                     });
    if (iter == _unused.end())
    {
      return TextBreakIterator(string, priorContext, mode, contentAnalysis, locale);
    }
    auto result = krys::move(*iter);
    _unused.erase(iter);
    result.SetText(string, priorContext);
    return result;
  }

  void TextBreakIteratorCache::Put(TextBreakIterator &&iterator) noexcept
  {
    _unused.push_back(krys::move(iterator));
    if (_unused.size() > Capacity)
    {
      _unused.erase(_unused.begin());
    }
  }

  // MARK: - CachedTextBreakIterator

  CachedTextBreakIterator::CachedTextBreakIterator(
    utf8_stringview string, utf8_stringview priorContext, TextBreakIterator::Mode mode,
    const StringAtom &locale, TextBreakIterator::ContentAnalysis contentAnalysis) noexcept
      : _backing(
          TextBreakIteratorCache::Singleton().Take(string, priorContext, mode, contentAnalysis, locale))
  {
  }

  CachedTextBreakIterator::~CachedTextBreakIterator() noexcept
  {
    if (_backing)
    {
      TextBreakIteratorCache::Singleton().Put(krys::move(*_backing));
    }
  }

  CachedTextBreakIterator::CachedTextBreakIterator(CachedTextBreakIterator &&other) noexcept
      : _backing(std::exchange(other._backing, null))
  {
  }

  CachedTextBreakIterator &CachedTextBreakIterator::operator=(CachedTextBreakIterator &&other) noexcept
  {
    if (this != &other)
    {
      if (_backing)
      {
        TextBreakIteratorCache::Singleton().Put(krys::move(*_backing));
      }
      _backing = std::exchange(other._backing, null);
    }
    return *this;
  }

  // MARK: - Static iterators

  KRYS_NODISCARD static UBreakIterator *
    InitializeIterator(UBreakIteratorType type, const char *locale = CurrentTextBreakLocaleID()) noexcept
  {
    UErrorCode openStatus = U_ZERO_ERROR;
    UBreakIterator *iterator = ubrk_open(type, locale, nullptr, 0, &openStatus);
    krys_debug_assert_msg(U_SUCCESS(openStatus), "ICU could not open a break iterator");
    return iterator;
  }

  KRYS_NODISCARD static UBreakIterator *SetTextForIterator(UBreakIterator &iterator,
                                                           utf8_stringview string) noexcept
  {
    if (!SetUTF8Text(iterator, string))
    {
      return nullptr;
    }
    return &iterator;
  }

  UBreakIterator *WordBreakIterator(utf8_stringview string) noexcept
  {
    static UBreakIterator *staticWordBreakIterator = InitializeIterator(UBRK_WORD);
    if (!staticWordBreakIterator)
    {
      return nullptr;
    }

    return SetTextForIterator(*staticWordBreakIterator, string);
  }

  UBreakIterator *SentenceBreakIterator(utf8_stringview string) noexcept
  {
    static UBreakIterator *staticSentenceBreakIterator = InitializeIterator(UBRK_SENTENCE);
    if (!staticSentenceBreakIterator)
    {
      return nullptr;
    }

    return SetTextForIterator(*staticSentenceBreakIterator, string);
  }

  bool IsWordTextBreak(UBreakIterator *iterator) noexcept
  {
    int ruleStatus = ubrk_getRuleStatus(iterator);
    return ruleStatus != UBRK_WORD_NONE;
  }

  // MARK: - CachedLineBreakIteratorFactory

  void CachedLineBreakIteratorFactory::PriorContext::Set(Array<char32, Length> &&newPriorContext) noexcept
  {
    _priorContext = krys::move(newPriorContext);
    Encode();
  }

  void CachedLineBreakIteratorFactory::PriorContext::Update(char32 last) noexcept
  {
    for (size_t i = 0; i < _priorContext.size() - 1; ++i)
    {
      _priorContext[i] = _priorContext[i + 1];
    }
    _priorContext[_priorContext.size() - 1] = last;
    Encode();
  }

  void CachedLineBreakIteratorFactory::PriorContext::Reset() noexcept
  {
    std::ranges::fill(_priorContext, 0);
    Encode();
  }

  uint32 CachedLineBreakIteratorFactory::PriorContext::GetLength() const noexcept
  {
    uint32 result = 0;
    for (auto iterator = std::rbegin(_priorContext); iterator != std::rend(_priorContext) && *iterator;
         ++iterator)
    {
      ++result;
    }
    return result;
  }

  void CachedLineBreakIteratorFactory::PriorContext::Encode() noexcept
  {
    // Encode the last code points, then move them to the end of the buffer, so that the encoding starts at a
    // different address when its length changes.
    Array<char8, Length * 4> encoded {};
    int32_t offset = 0;
    auto length = GetLength();
    for (auto i = _priorContext.size() - length; i < _priorContext.size(); ++i)
    {
      U8_APPEND_UNSAFE(encoded.data(), offset, _priorContext[i]);
    }
    _encodedLength = static_cast<uint32>(offset);
    std::ranges::fill(_encoded, 0);
    std::copy_n(encoded.begin(), offset, _encoded.end() - offset);
  }

  CachedTextBreakIterator &CachedLineBreakIteratorFactory::Get() noexcept
  {
    auto priorContext = _priorContext.Characters();
    if (!_iterator)
    {
      _iterator.emplace(_stringView, priorContext, TextBreakIterator::LineMode {_mode}, _locale,
                        _contentAnalysis);
      _cachedPriorContext = priorContext.data();
    }
    else if (priorContext.data() != _cachedPriorContext)
    {
      ResetStringAndReleaseIterator(_stringView, _locale, _mode, _contentAnalysis);
      return Get();
    }
    return *_iterator;
  }

  void CachedLineBreakIteratorFactory::ResetStringAndReleaseIterator(
    utf8_stringview stringView, const StringAtom &locale, TextBreakLineBehavior mode,
    TextBreakIterator::ContentAnalysis contentAnalysis) noexcept
  {
    _stringView = stringView;
    _locale = locale;
    _iterator = null;
    _cachedPriorContext = nullptr;
    _mode = mode;
    _contentAnalysis = contentAnalysis;
  }

  // MARK: - Non-shared iterators

  static UBreakIterator *nonSharedCharacterBreakIterator = nullptr;

  KRYS_NODISCARD static UBreakIterator *GetNonSharedCharacterBreakIterator() noexcept
  {
    if (auto *result = std::exchange(nonSharedCharacterBreakIterator, nullptr))
    {
      return result;
    }
    return InitializeIterator(UBRK_CHARACTER);
  }

  static void CacheNonSharedCharacterBreakIterator(UBreakIterator *cacheMe) noexcept
  {
    if (auto *old = std::exchange(nonSharedCharacterBreakIterator, cacheMe))
    {
      ubrk_close(old);
    }
  }

  NonSharedCharacterBreakIterator::NonSharedCharacterBreakIterator(utf8_stringview string) noexcept
  {
    if ((_iterator = GetNonSharedCharacterBreakIterator()))
    {
      _iterator = SetTextForIterator(*_iterator, string);
    }
  }

  NonSharedCharacterBreakIterator::~NonSharedCharacterBreakIterator() noexcept
  {
    if (_iterator)
    {
      CacheNonSharedCharacterBreakIterator(_iterator);
    }
  }

  NonSharedCharacterBreakIterator::NonSharedCharacterBreakIterator(
    NonSharedCharacterBreakIterator &&other) noexcept
      : _iterator(nullptr)
  {
    std::swap(_iterator, other._iterator);
  }

  static UBreakIterator *nonSharedSentenceBreakIterator = nullptr;

  KRYS_NODISCARD static UBreakIterator *GetNonSharedSentenceBreakIterator() noexcept
  {
    if (auto *result = std::exchange(nonSharedSentenceBreakIterator, nullptr))
    {
      return result;
    }
    return InitializeIterator(UBRK_SENTENCE);
  }

  static void CacheNonSharedSentenceBreakIterator(UBreakIterator *cacheMe) noexcept
  {
    if (auto *old = std::exchange(nonSharedSentenceBreakIterator, cacheMe))
    {
      ubrk_close(old);
    }
  }

  NonSharedSentenceBreakIterator::NonSharedSentenceBreakIterator(utf8_stringview string) noexcept
  {
    if ((_iterator = GetNonSharedSentenceBreakIterator()))
    {
      _iterator = SetTextForIterator(*_iterator, string);
    }
  }

  NonSharedSentenceBreakIterator::~NonSharedSentenceBreakIterator() noexcept
  {
    if (_iterator)
    {
      CacheNonSharedSentenceBreakIterator(_iterator);
    }
  }

  NonSharedSentenceBreakIterator::NonSharedSentenceBreakIterator(
    NonSharedSentenceBreakIterator &&other) noexcept
      : _iterator(nullptr)
  {
    std::swap(_iterator, other._iterator);
  }

  // MARK: - Grapheme clusters

  KRYS_NODISCARD static bool IsASCII(utf8_stringview string) noexcept
  {
    return std::ranges::all_of(string, [](char8 c) { return c < 0x80; });
  }

  uint32 NumGraphemeClusters(utf8_stringview string) noexcept
  {
    auto stringLength = static_cast<uint32>(string.size());

    if (!stringLength)
    {
      return 0;
    }

    // The only ASCII extended grapheme cluster is CRLF.
    if (IsASCII(string))
    {
      uint32 numCRLF = 0;
      for (uint32 i = 1; i < stringLength; ++i)
      {
        numCRLF += string[i - 1] == u8'\r' && string[i] == u8'\n';
      }
      return stringLength - numCRLF;
    }

    NonSharedCharacterBreakIterator iterator {string};
    if (!iterator)
    {
      krys_debug_assert_msg(false, "ICU couldn't make a character break iterator");
      return stringLength;
    }

    uint32 numGraphemeClusters = 0;
    while (ubrk_next(iterator) != UBRK_DONE)
    {
      ++numGraphemeClusters;
    }
    return numGraphemeClusters;
  }

  uint32 NumCodeUnitsInGraphemeClusters(utf8_stringview string, uint32 numGraphemeClusters) noexcept
  {
    auto stringLength = static_cast<uint32>(string.size());

    if (stringLength <= numGraphemeClusters)
    {
      return stringLength;
    }

    // The only ASCII extended grapheme cluster is CRLF.
    if (IsASCII(string))
    {
      uint32 i = 0;
      uint32 j = 0;
      for (; i < numGraphemeClusters && j + 1 < stringLength; ++i, ++j)
      {
        j += string[j] == u8'\r' && string[j + 1] == u8'\n';
      }
      return j + (i < numGraphemeClusters);
    }

    NonSharedCharacterBreakIterator iterator {string};
    if (!iterator)
    {
      krys_debug_assert_msg(false, "ICU couldn't make a character break iterator");
      return stringLength;
    }

    for (uint32 i = 0; i < numGraphemeClusters; ++i)
    {
      if (ubrk_next(iterator) == UBRK_DONE)
      {
        return stringLength;
      }
    }
    return static_cast<uint32>(ubrk_current(iterator));
  }

  const char *CurrentTextBreakLocaleID() noexcept
  {
    return uloc_getDefault();
  }
}
