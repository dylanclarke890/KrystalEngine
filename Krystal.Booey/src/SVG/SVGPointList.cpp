/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

namespace krys::boo
{
  bool SVGPointList::Parse(DOMStringView value) noexcept
  {
    ClearItems();

    auto buffer = value;
    svg::SkipOptionalSVGSpaces(buffer);

    bool delimParsed = false;
    while (!buffer.empty())
    {
      delimParsed = false;
      auto xPos = svg::ParseNumber(buffer);
      if (!xPos)
      {
        return false;
      }

      auto yPos = svg::ParseNumber(buffer, svg::SuffixSkippingPolicy::DontSkip);
      if (!yPos)
      {
        return false;
      }

      svg::SkipOptionalSVGSpaces(buffer);

      if (!buffer.empty() && buffer.front() == u8',')
      {
        buffer.remove_prefix(1);
        delimParsed = true;
      }

      svg::SkipOptionalSVGSpaces(buffer);

      Append(SVGPoint::Create({*xPos, *yPos}));
    }

    // FIXME: Should this ClearItems() on failure like SVGTransformList does?
    return !delimParsed;
  }

  DOMString SVGPointList::ValueAsString() const noexcept
  {
    StringBuilder builder;
    for (const auto &point : _items)
    {
      if (builder.Length())
      {
        builder.Append(u8' ');
      }

      builder.Append(point->X(), u8' ', point->Y());
    }
    return krys::move(builder).ToString();
  }
}
