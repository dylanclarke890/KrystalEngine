/*
 * Copyright (C) 2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2004-2012, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Nicholas Shanks <webkit@nickshanks.com>
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
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
 */

#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/HexNumber.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo::css
{
  namespace
  {
    void AppendCodePoint(char32 c, StringBuilder &appendTo) noexcept
    {
      appendTo.Append(c);
    }

    /// @brief "ident" from the CSS tokenizer, minus backslash-escape sequences.
    KRYS_NODISCARD bool IsCSSTokenizerIdentifier(CSSOMStringView string) noexcept
    {
      if (string.empty())
      {
        return false;
      }

      // -?
      while (!string.empty() && string.front() == u8'-')
      {
        string.remove_prefix(1);
      }

      // {nmstart}
      // Any non-ASCII code point is a name start code point, so checking the lead byte is enough.
      if (string.empty() || !IsNameStartCodePoint(string.front()))
      {
        return false;
      }
      string.remove_prefix(1);

      // {nmchar}*
      // Continuation bytes are non-ASCII, so they pass IsNameCodePoint too.
      while (!string.empty() && IsNameCodePoint(string.front()))
      {
        string.remove_prefix(1);
      }

      return string.empty();
    }

    void SerialiseCharacter(char32 c, StringBuilder &appendTo) noexcept
    {
      appendTo.Append(u8'\\');
      AppendCodePoint(c, appendTo);
    }

    void SerialiseCharacterAsCodePoint(char32 c, StringBuilder &appendTo) noexcept
    {
      appendTo.Append(u8'\\', Hex(static_cast<uint32>(c), HexConversionMode::Lowercase), u8' ');
    }
  }

  void SerialiseIdentifier(const CSSOMString &identifier, StringBuilder &appendTo,
                           bool skipStartChecks) noexcept
  {
    bool isFirst = !skipStartChecks;
    bool isSecond = false;
    bool isFirstCharHyphen = false;
    for (krys::text::CodePointIterator it {utf8_stringview {identifier}}; !it.AtEnd();)
    {
      auto c = *it;
      bool isLast = (++it).AtEnd();
      if (!c)
      {
        AppendCodePoint(krys::text::UnicodeReplacementChar, appendTo);
      }
      else if (c <= 0x1F || c == 0x7F
               || (0x30 <= c && c <= 0x39 && (isFirst || (isSecond && isFirstCharHyphen))))
      {
        SerialiseCharacterAsCodePoint(c, appendTo);
      }
      else if (c == U'-' && isFirst && isLast)
      {
        SerialiseCharacter(c, appendTo);
      }
      else if (0x80 <= c || c == U'-' || c == U'_' || (0x30 <= c && c <= 0x39) || (0x41 <= c && c <= 0x5A)
               || (0x61 <= c && c <= 0x7A))
      {
        AppendCodePoint(c, appendTo);
      }
      else
      {
        SerialiseCharacter(c, appendTo);
      }

      if (isFirst)
      {
        isFirst = false;
        isSecond = true;
        isFirstCharHyphen = (c == U'-');
      }
      else if (isSecond)
      {
        isSecond = false;
      }
    }
  }

  void SerialiseString(const CSSOMString &string, StringBuilder &appendTo) noexcept
  {
    appendTo.Append(u8'"');

    for (krys::text::CodePointIterator it {utf8_stringview {string}}; !it.AtEnd(); ++it)
    {
      auto c = *it;
      if (c <= 0x1F || c == 0x7F)
      {
        SerialiseCharacterAsCodePoint(c, appendTo);
      }
      else if (c == U'"' || c == U'\\')
      {
        SerialiseCharacter(c, appendTo);
      }
      else
      {
        AppendCodePoint(c, appendTo);
      }
    }

    appendTo.Append(u8'"');
  }

  CSSOMString SerialiseString(const CSSOMString &value) noexcept
  {
    StringBuilder builder;
    SerialiseString(value, builder);
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseURL(const CSSOMString &value) noexcept
  {
    StringBuilder builder;
    builder.Append(u8"url(");
    SerialiseString(value, builder);
    builder.Append(u8')');
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseFontFamily(const CSSOMString &value) noexcept
  {
    return IsCSSTokenizerIdentifier(value) ? value : SerialiseString(value);
  }
}
