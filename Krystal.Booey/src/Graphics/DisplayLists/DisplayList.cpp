/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::display_list
{
  DisplayList::DisplayList(List<Item> &&items) noexcept : _items(krys::move(items))
  {
  }

  DisplayList::~DisplayList() noexcept = default;

  utf8_string DisplayList::AsText(EnumSet<AsTextFlag> flags) const noexcept
  {
    TextStream stream(TextStream::LineMode::MultipleLine, TextStream::Formatting::SVGStyleRect);
    if (flags.Contains(AsTextFlag::IncludeResourceIdentifiers))
    {
      stream.DumpProperty(u8"display-list-identifier", static_cast<const void *>(this));
    }
    for (const auto &item : _items)
    {
      if (!ShouldDumpItem(item, flags))
      {
        continue;
      }
      TextStream::GroupScope group(stream);
      DumpItem(stream, item, flags);
    }
    return stream.Release();
  }

  void DisplayList::Dump(TextStream &ts) const noexcept
  {
    TextStream::GroupScope group(ts);
    ts << "display list";
    for (const auto &item : _items)
    {
      TextStream::GroupScope itemGroup(ts);
      DumpItem(ts, item, {AsTextFlag::IncludePlatformOperations, AsTextFlag::IncludeResourceIdentifiers});
    }
  }

  TextStream &operator<<(TextStream &ts, const DisplayList &displayList) noexcept
  {
    displayList.Dump(ts);
    return ts;
  }
}
