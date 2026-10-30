#include "Krystal.Booey/HTML/TypeAhead.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace krys::boo::html
{
  namespace
  {
    constexpr auto TypeAheadTimeout = static_cast<dom::DOMHighResTimeStamp>(
      std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::seconds(1)).count());

    KRYS_NODISCARD bool IsSpaceOrNewline(char32 c) noexcept
    {
      if (c <= 0x7F)
      {
        return c == ' ' || (c >= '\t' && c <= '\r');
      }
      return u_charDirection(static_cast<UChar32>(c)) == U_WHITE_SPACE_NEUTRAL;
    }

    /// @brief @p text's code points, case folded, without its leading white space.
    KRYS_NODISCARD utf32_string FoldCaseStrippingLeadingWhiteSpace(dom::DOMStringView text) noexcept
    {
      utf32_string folded;
      auto *bytes = reinterpret_cast<const uint8_t *>(text.data());
      auto length = static_cast<int32>(text.size());
      for (int32 i = 0; i < length;)
      {
        UChar32 c;
        U8_NEXT(bytes, i, length, c);
        if (c < 0)
        {
          continue;
        }
        if (folded.empty() && (c == 0xA0 || IsSpaceOrNewline(static_cast<char32>(c))))
        {
          continue;
        }
        folded.push_back(static_cast<char32>(u_foldCase(c, U_FOLD_CASE_DEFAULT)));
      }
      return folded;
    }

    KRYS_NODISCARD utf32_string FoldCase(utf32_stringview text) noexcept
    {
      utf32_string folded;
      folded.reserve(text.size());
      for (auto c : text)
      {
        folded.push_back(static_cast<char32>(u_foldCase(static_cast<UChar32>(c), U_FOLD_CASE_DEFAULT)));
      }
      return folded;
    }
  }

  TypeAhead::TypeAhead(TypeAheadDataSource *dataSource) noexcept : _dataSource(dataSource)
  {
  }

  int32 TypeAhead::HandleEvent(dom::KeyboardEvent &event, EnumSet<MatchMode> matchMode) noexcept
  {
    if (event.TimeStamp() < _lastTypeTime)
    {
      return -1;
    }

    int32 optionCount = _dataSource->OptionCount();
    auto delta = event.TimeStamp() - _lastTypeTime;
    _lastTypeTime = event.TimeStamp();

    auto c = static_cast<char32>(event.CharCode());

    if (delta > TypeAheadTimeout)
    {
      _buffer.clear();
    }

    _buffer.push_back(c);

    if (optionCount < 1)
    {
      return -1;
    }

    int32 searchStartOffset = 1;
    utf32_string prefix;
    if (matchMode.Contains(MatchMode::CycleFirstChar) && c == _repeatingChar)
    {
      // The user is likely trying to cycle through all the items starting with this character, so just
      // search on the character.
      prefix = utf32_string(1uz, c);
      _repeatingChar = c;
    }
    else if (matchMode.Contains(MatchMode::MatchPrefix))
    {
      prefix = _buffer;
      if (_buffer.size() > 1uz)
      {
        _repeatingChar = 0;
        searchStartOffset = 0;
      }
      else
      {
        _repeatingChar = c;
      }
    }

    if (!prefix.empty())
    {
      int32 selected = _dataSource->IndexOfSelectedOption();
      int32 index = (selected < 0 ? 0 : selected) + searchStartOffset;
      index %= optionCount;

      auto prefixWithCaseFolded = FoldCase(prefix);
      for (int32 i = 0; i < optionCount; ++i, index = (index + 1) % optionCount)
      {
        // Fold the option string and check if its prefix is equal to the folded prefix.
        auto text = _dataSource->OptionAtIndex(index);
        if (FoldCaseStrippingLeadingWhiteSpace(text).starts_with(prefixWithCaseFolded))
        {
          return index;
        }
      }
    }

    if (matchMode.Contains(MatchMode::MatchIndex))
    {
      dom::DOMString digits;
      for (auto bufferChar : _buffer)
      {
        if (bufferChar > 0x7F)
        {
          break;
        }
        digits.push_back(static_cast<char8_t>(bufferChar));
      }
      auto result = Numbers::ParseInteger(digits);
      auto index = result.Success() ? result.Value : 0;
      if (index > 0 && index <= optionCount)
      {
        return static_cast<int32>(index - 1);
      }
    }

    return -1;
  }
}
