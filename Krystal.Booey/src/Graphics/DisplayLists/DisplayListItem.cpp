/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItem.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::display_list
{
  void ApplyItem(GraphicsContext &context, const Item &item) noexcept
  {
    std::visit([&](const auto &item) { item.Apply(context); }, item);
  }

  bool ShouldDumpItem(const Item &item, EnumSet<AsTextFlag> flags) noexcept
  {
    if (auto *setState = std::get_if<SetState>(&item))
    {
      if (!flags.Contains(AsTextFlag::IncludePlatformOperations))
      {
        return true;
      }
      // FIXME: for now, only drop the item if the only state-change flags are platform-specific.
      return setState->State().Changes()
             != GraphicsContextState::ChangeFlags {GraphicsContextState::Change::ShouldSubpixelQuantizeFonts};
    }
    return true;
  }

  void DumpItem(TextStream &ts, const Item &item, EnumSet<AsTextFlag> flags) noexcept
  {
    std::visit(
      [&]<typename TItem>(const TItem &item)
      {
        ts << TItem::Name;
        item.Dump(ts, flags);
      },
      item);
  }

  TextStream &operator<<(TextStream &ts, const Item &item) noexcept
  {
    DumpItem(ts, item, {AsTextFlag::IncludePlatformOperations, AsTextFlag::IncludeResourceIdentifiers});
    return ts;
  }
}
