/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Platform/Network/HTTPHeaderMap.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>

namespace krys::boo
{
  Maybe<utf8_stringview> HTTPHeaderMap::Get(utf8_stringview name) const noexcept
  {
    auto found = std::ranges::find_if(_headers, [&](const auto &header)
                                      { return text::IsASCIICaselessEqual(header.first, name); });
    if (found == _headers.end())
    {
      return null;
    }
    return utf8_stringview(found->second);
  }

  Pair<utf8_string, utf8_string> *HTTPHeaderMap::Find(utf8_stringview name) noexcept
  {
    auto found = std::ranges::find_if(_headers, [&](const auto &header)
                                      { return text::IsASCIICaselessEqual(header.first, name); });
    return found == _headers.end() ? nullptr : &*found;
  }

  void HTTPHeaderMap::Set(utf8_stringview name, utf8_stringview value) noexcept
  {
    if (auto *header = Find(name))
    {
      header->second = utf8_string(value);
      return;
    }
    _headers.emplace_back(utf8_string(name), utf8_string(value));
  }

  void HTTPHeaderMap::Add(utf8_stringview name, utf8_stringview value) noexcept
  {
    if (auto *header = Find(name))
    {
      header->second += u8", ";
      header->second += value;
      return;
    }
    _headers.emplace_back(utf8_string(name), utf8_string(value));
  }

  void HTTPHeaderMap::Remove(utf8_stringview name) noexcept
  {
    std::erase_if(_headers,
                  [&](const auto &header) { return text::IsASCIICaselessEqual(header.first, name); });
  }
}
