/*
 * Copyright (C) 2004, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Editing/Markup/HTMLInterchange.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo
{
  // Offsets into the text are UTF-8 bytes; collapsible whitespace is ASCII, so each is one byte.
  utf8_string ConvertHTMLTextToInterchangeFormat(utf8_stringview in, const dom::Text *node) noexcept
  {
    // Assume all the text comes from node.
    if (node->Renderer() && node->Renderer()->Style().PreserveNewline())
    {
      return utf8_string(in);
    }

    const utf8_string convertedSpaceString =
      utf8_string(u8"<span class=\"") + utf8_string(AppleConvertedSpace) + u8"\"> </span>";

    utf8_string s;
    size_t i = 0;
    size_t consumed = 0;
    while (i < in.size())
    {
      consumed = 1;
      if (DeprecatedIsCollapsibleWhitespace(in[i]))
      {
        // count number of adjoining spaces
        size_t j = i + 1;
        while (j < in.size() && DeprecatedIsCollapsibleWhitespace(in[j]))
        {
          j++;
        }
        size_t count = j - i;
        consumed = count;
        while (count)
        {
          size_t add = count % 3;
          switch (add)
          {
            case 0:
              s += convertedSpaceString;
              s += u8' ';
              s += convertedSpaceString;
              add = 3;
              break;
            case 1:
              if (i == 0 || i + 1 == in.size()) // at start or end of string
              {
                s += convertedSpaceString;
              }
              else
              {
                s += u8' ';
              }
              break;
            case 2:
              if (i == 0)
              {
                // at start of string
                s += convertedSpaceString;
                s += u8' ';
              }
              else if (i + 2 == in.size())
              {
                // at end of string
                s += convertedSpaceString;
                s += convertedSpaceString;
              }
              else
              {
                s += convertedSpaceString;
                s += u8' ';
              }
              break;
          }
          count -= add;
        }
      }
      else
      {
        s += in[i];
      }
      i += consumed;
    }
    return s;
  }
}
