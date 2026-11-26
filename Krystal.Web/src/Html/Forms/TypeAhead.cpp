#include "Krystal.Web/Html/Forms/TypeAhead.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Text/Bidi.hpp"
#include "Krystal.Text/CaseMapping.hpp"
#include "Krystal.Web/Html/MicroParsers/Numbers.hpp"
#include "Krystal.Web/Input/UIEvents/KeyboardEvent.hpp"

namespace krys::web::html
{
  namespace
  {
    constexpr MonotonicTime::duration TypeAheadTimeout = std::chrono::seconds(1);

    KRYS_NODISCARD bool IsSpaceOrNewline(char32 c) noexcept
    {
      if (c <= 0x7F)
      {
        return c == ' ' || (c >= '\t' && c <= '\r');
      }
      return text::GetBidiClass(c) == text::BidiClass::WhiteSpaceNeutral;
    }

    /// @brief @p text's code points, case folded, without its leading white space.
    KRYS_NODISCARD utf32_string FoldCaseStrippingLeadingWhiteSpace(DOMStringView text) noexcept
    {
      utf32_string folded;
      for (size_t i = 0; i < text.size();)
      {
        auto decoded = text::DecodeUTF8(Span<const char8>(text.data() + i, text.size() - i));
        i += decoded.Length;
        if (!decoded.IsValid)
        {
          continue;
        }
        auto c = decoded.CodePoint;
        if (folded.empty() && (c == 0xA0 || IsSpaceOrNewline(static_cast<char32>(c))))
        {
          continue;
        }
        folded.push_back(text::SimpleCaseFold(c));
      }
      return folded;
    }

    KRYS_NODISCARD utf32_string FoldCase(utf32_stringview text) noexcept
    {
      utf32_string folded;
      folded.reserve(text.size());
      for (auto c : text)
      {
        folded.push_back(text::SimpleCaseFold(c));
      }
      return folded;
    }
  }

  TypeAhead::TypeAhead(TypeAheadDataSource *dataSource) noexcept : _dataSource(dataSource)
  {
  }

  int32 TypeAhead::HandleEvent(KeyboardEvent &event, EnumSet<MatchMode> matchMode) noexcept
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
    if (matchMode.contains(MatchMode::CycleFirstChar) && c == _repeatingChar)
    {
      // The user is likely trying to cycle through all the items starting with this character, so just
      // search on the character.
      prefix = utf32_string(1uz, c);
      _repeatingChar = c;
    }
    else if (matchMode.contains(MatchMode::MatchPrefix))
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
        // Compared case-folded, ignoring the option's leading white space.
        auto text = _dataSource->OptionAtIndex(index);
        if (FoldCaseStrippingLeadingWhiteSpace(text).starts_with(prefixWithCaseFolded))
        {
          return index;
        }
      }
    }

    if (matchMode.contains(MatchMode::MatchIndex))
    {
      DOMString digits;
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
