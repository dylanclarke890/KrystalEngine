#include "Krystal.Text/TextBreakIterator.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/Unicode.hpp"
#include <algorithm>
#include <unicode/ubrk.h>
#include <unicode/uloc.h>
#include <unicode/utext.h>
#include <utility>

namespace krys::text
{
  namespace
  {
    KRYS_NODISCARD UBreakIteratorType ToICU(TextBreakUnit unit) noexcept
    {
      switch (unit)
      {
        case TextBreakUnit::GraphemeCluster: return UBRK_CHARACTER;
        case TextBreakUnit::Word:            return UBRK_WORD;
        case TextBreakUnit::Sentence:        return UBRK_SENTENCE;
        case TextBreakUnit::Line:            return UBRK_LINE;
      }
      krys_unreachable();
    }

    // The ICU locale for line breaking with @p behavior: the locale with its `lb` keyword set. A locale is
    // needed for the keyword to apply.
    KRYS_NODISCARD utf8_string LocaleId(const StringAtom &locale, TextBreakUnit unit,
                                        TextBreakLineBehavior behavior) noexcept
    {
      auto id = utf8_string {locale.View()};
      if (unit != TextBreakUnit::Line || behavior == TextBreakLineBehavior::Default || id.empty())
      {
        return id;
      }

      const char *value = behavior == TextBreakLineBehavior::Loose    ? "loose"
                          : behavior == TextBreakLineBehavior::Normal ? "normal"
                                                                      : "strict";
      Array<char, ULOC_FULLNAME_CAPACITY> buffer {};
      if (id.size() >= buffer.size())
      {
        return id;
      }
      std::ranges::copy(id, buffer.begin());
      UErrorCode status = U_ZERO_ERROR;
      auto length =
        uloc_setKeywordValue("lb", value, buffer.data(), static_cast<int32>(buffer.size()), &status);
      if (U_FAILURE(status) || status == U_STRING_NOT_TERMINATED_WARNING)
      {
        return id;
      }
      return utf8_string {reinterpret_cast<const char8 *>(buffer.data()), static_cast<size_t>(length)};
    }

    // Released ICU iterators, for reuse: opening one loads its rules, which is slow.
    class IteratorPool
    {
      KRYS_NON_COPY_MOVABLE(IteratorPool);

    private:
      struct Entry
      {
        UBreakIterator *Iterator;
        TextBreakUnit Unit;
        utf8_string LocaleId;
      };

      constexpr static size_t Capacity = 4;

      List<Entry> _entries;

    public:
      IteratorPool() noexcept = default;

      ~IteratorPool() noexcept
      {
        for (auto &entry : _entries)
        {
          ubrk_close(entry.Iterator);
        }
      }

      KRYS_NODISCARD static IteratorPool &Current() noexcept
      {
        thread_local IteratorPool pool;
        return pool;
      }

      KRYS_NODISCARD UBreakIterator *Take(TextBreakUnit unit, const utf8_string &localeId) noexcept
      {
        auto entry =
          std::ranges::find_if(_entries, [&](const Entry &candidate)
                               { return candidate.Unit == unit && candidate.LocaleId == localeId; });
        if (entry != _entries.end())
        {
          auto *iterator = entry->Iterator;
          _entries.erase(entry);
          return iterator;
        }

        UErrorCode status = U_ZERO_ERROR;
        auto *iterator =
          ubrk_open(ToICU(unit), reinterpret_cast<const char *>(localeId.c_str()), nullptr, 0, &status);
        if (U_FAILURE(status) && !localeId.empty())
        {
          // An unknown locale falls back to the root rules.
          status = U_ZERO_ERROR;
          iterator = ubrk_open(ToICU(unit), "", nullptr, 0, &status);
        }
        krys_debug_assert_msg(U_SUCCESS(status), "ICU couldn't open a break iterator");
        return U_SUCCESS(status) ? iterator : nullptr;
      }

      void Put(UBreakIterator *iterator, TextBreakUnit unit, utf8_string localeId) noexcept
      {
        if (_entries.size() == Capacity)
        {
          ubrk_close(_entries.front().Iterator);
          _entries.erase(_entries.begin());
        }
        _entries.push_back({iterator, unit, krys::move(localeId)});
      }
    };
  }

#pragma region TextBreakIterator

  TextBreakIterator::TextBreakIterator(utf8_stringview text, TextBreakUnit unit, const StringAtom &locale,
                                       TextBreakLineBehavior lineBehavior,
                                       utf8_stringview priorContext) noexcept
      : _unit(unit), _localeId(LocaleId(locale, unit, lineBehavior))
  {
    _iterator = IteratorPool::Current().Take(_unit, _localeId);
    if (_iterator)
    {
      SetText(text, priorContext);
    }
  }

  TextBreakIterator::TextBreakIterator(TextBreakIterator &&other) noexcept
      : _iterator(std::exchange(other._iterator, nullptr)), _unit(other._unit),
        _localeId(krys::move(other._localeId)),
        _textWithPriorContext(krys::move(other._textWithPriorContext)),
        _priorContextLength(other._priorContextLength)
  {
  }

  TextBreakIterator &TextBreakIterator::operator=(TextBreakIterator &&other) noexcept
  {
    if (this != &other)
    {
      Release();
      _iterator = std::exchange(other._iterator, nullptr);
      _unit = other._unit;
      _localeId = krys::move(other._localeId);
      // A list keeps its characters in place when it moves, so ICU's pointer into them stays valid.
      _textWithPriorContext = krys::move(other._textWithPriorContext);
      _priorContextLength = other._priorContextLength;
    }
    return *this;
  }

  TextBreakIterator::~TextBreakIterator() noexcept
  {
    Release();
  }

  void TextBreakIterator::Release() noexcept
  {
    if (auto *iterator = std::exchange(_iterator, nullptr))
    {
      IteratorPool::Current().Put(iterator, _unit, krys::move(_localeId));
    }
  }

  void TextBreakIterator::SetText(utf8_stringview text, utf8_stringview priorContext) noexcept
  {
    // ICU reads UTF-8 in place, so a prior context needs a copy of the text after it.
    auto iterated = text;
    _textWithPriorContext.clear();
    if (!priorContext.empty())
    {
      _textWithPriorContext.reserve(priorContext.size() + text.size());
      _textWithPriorContext.insert(_textWithPriorContext.end(), priorContext.begin(), priorContext.end());
      _textWithPriorContext.insert(_textWithPriorContext.end(), text.begin(), text.end());
      iterated = {_textWithPriorContext.data(), _textWithPriorContext.size()};
    }
    _priorContextLength = static_cast<uint32>(priorContext.size());

    UErrorCode status = U_ZERO_ERROR;
    UText utext = UTEXT_INITIALIZER;
    utext_openUTF8(&utext, reinterpret_cast<const char *>(iterated.data()),
                   static_cast<int64_t>(iterated.size()), &status);
    // The iterator keeps a shallow clone of the UText, which refers to the characters.
    ubrk_setUText(_iterator, &utext, &status);
    utext_close(&utext);
    krys_debug_assert_msg(U_SUCCESS(status), "ICU couldn't take the text");
  }

  Maybe<uint32> TextBreakIterator::FromICU(int32 index) const noexcept
  {
    if (index == UBRK_DONE)
    {
      return null;
    }
    return static_cast<uint32>(std::max(index, static_cast<int32>(_priorContextLength)))
           - _priorContextLength;
  }

  uint32 TextBreakIterator::First() const noexcept
  {
    if (!_priorContextLength)
    {
      return static_cast<uint32>(ubrk_first(_iterator));
    }
    std::ignore = ubrk_isBoundary(_iterator, static_cast<int32>(_priorContextLength));
    return Current();
  }

  uint32 TextBreakIterator::Last() const noexcept
  {
    return FromICU(ubrk_last(_iterator)).value_or(0);
  }

  Maybe<uint32> TextBreakIterator::Next() const noexcept
  {
    return FromICU(ubrk_next(_iterator));
  }

  Maybe<uint32> TextBreakIterator::Previous() const noexcept
  {
    auto index = ubrk_previous(_iterator);
    if (index != UBRK_DONE && index < static_cast<int32>(_priorContextLength))
    {
      return null;
    }
    return FromICU(index);
  }

  Maybe<uint32> TextBreakIterator::Following(uint32 offset) const noexcept
  {
    // Unsigned arithmetic, so an offset of -1 lands in the prior context.
    return FromICU(ubrk_following(_iterator, static_cast<int32>(offset + _priorContextLength)));
  }

  Maybe<uint32> TextBreakIterator::Preceding(uint32 offset) const noexcept
  {
    if (!offset)
    {
      return null;
    }
    return FromICU(ubrk_preceding(_iterator, static_cast<int32>(offset + _priorContextLength)));
  }

  bool TextBreakIterator::IsBoundary(uint32 offset) const noexcept
  {
    return ubrk_isBoundary(_iterator, static_cast<int32>(offset + _priorContextLength));
  }

  uint32 TextBreakIterator::Current() const noexcept
  {
    return FromICU(ubrk_current(_iterator)).value_or(0);
  }

  bool TextBreakIterator::IsWordBreak() const noexcept
  {
    return ubrk_getRuleStatus(_iterator) != UBRK_WORD_NONE;
  }

  TextBreakIterator WordBreakIterator(utf8_stringview text) noexcept
  {
    return TextBreakIterator {text, TextBreakUnit::Word};
  }

  TextBreakIterator SentenceBreakIterator(utf8_stringview text) noexcept
  {
    return TextBreakIterator {text, TextBreakUnit::Sentence};
  }

#pragma endregion

#pragma region CachedLineBreakIteratorFactory

  void CachedLineBreakIteratorFactory::PriorContext::Set(Array<char32, Length> codePoints) noexcept
  {
    _codePoints = codePoints;
    Encode();
  }

  void CachedLineBreakIteratorFactory::PriorContext::Update(char32 last) noexcept
  {
    std::shift_left(_codePoints.begin(), _codePoints.end(), 1);
    _codePoints.back() = last;
    Encode();
  }

  void CachedLineBreakIteratorFactory::PriorContext::Reset() noexcept
  {
    _codePoints = {};
    Encode();
  }

  uint32 CachedLineBreakIteratorFactory::PriorContext::GetLength() const noexcept
  {
    // The code points are right-aligned, after any zeros.
    auto firstUsed = std::ranges::find_if(_codePoints, [](char32 codePoint) { return codePoint != 0; });
    return static_cast<uint32>(_codePoints.end() - firstUsed);
  }

  void CachedLineBreakIteratorFactory::PriorContext::Encode() noexcept
  {
    _encodedLength = 0;
    for (auto index = Length - GetLength(); index < Length; ++index)
    {
      _encodedLength += static_cast<uint32>(
        EncodeUTF8(_codePoints[index], Span<char8, MaxUTF8SequenceLength>(_encoded.data() + _encodedLength,
                                                                          MaxUTF8SequenceLength)));
    }
  }

  TextBreakIterator &CachedLineBreakIteratorFactory::operator*() noexcept
  {
    if (!_iterator || _iteratorPriorContext != _priorContext)
    {
      _iterator.emplace(_string, TextBreakUnit::Line, _locale, _mode, _priorContext.Characters());
      _iteratorPriorContext = _priorContext;
    }
    return *_iterator;
  }

#pragma endregion

#pragma region Grapheme clusters

  namespace
  {
    KRYS_NODISCARD bool IsASCII(utf8_stringview text) noexcept
    {
      return std::ranges::all_of(text, [](char8 c) { return c < 0x80; });
    }

    KRYS_NODISCARD bool IsCRLF(utf8_stringview text, size_t index) noexcept
    {
      return text[index] == u8'\r' && index + 1 < text.size() && text[index + 1] == u8'\n';
    }
  }

  uint32 NumGraphemeClusters(utf8_stringview text) noexcept
  {
    // In ASCII, every character is a cluster except for CRLF, which is one cluster of two.
    if (IsASCII(text))
    {
      uint32 count = 0;
      for (size_t index = 0; index < text.size(); index += IsCRLF(text, index) ? 2 : 1)
      {
        ++count;
      }
      return count;
    }

    TextBreakIterator clusters {text, TextBreakUnit::GraphemeCluster};
    if (!clusters)
    {
      return static_cast<uint32>(text.size());
    }
    uint32 count = 0;
    while (clusters.Next())
    {
      ++count;
    }
    return count;
  }

  uint32 NumCodeUnitsInGraphemeClusters(utf8_stringview text, uint32 count) noexcept
  {
    if (IsASCII(text))
    {
      size_t index = 0;
      for (uint32 cluster = 0; cluster < count && index < text.size(); ++cluster)
      {
        index += IsCRLF(text, index) ? 2 : 1;
      }
      return static_cast<uint32>(index);
    }

    TextBreakIterator clusters {text, TextBreakUnit::GraphemeCluster};
    if (!clusters)
    {
      return static_cast<uint32>(text.size());
    }
    for (uint32 cluster = 0; cluster < count; ++cluster)
    {
      if (!clusters.Next())
      {
        return static_cast<uint32>(text.size());
      }
    }
    return clusters.Current();
  }

#pragma endregion
}
