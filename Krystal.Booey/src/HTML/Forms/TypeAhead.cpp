/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2009, 2010, 2011 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
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

#include "Krystal.Booey/HTML/Forms/TypeAhead.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <unicode/uchar.h>
#include <unicode/utf8.h>

namespace krys::boo::html
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
