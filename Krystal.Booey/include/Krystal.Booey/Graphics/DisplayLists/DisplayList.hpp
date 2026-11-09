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

#pragma once

#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::display_list
{
  /// @brief Recorded drawing: the items a recorder appended, replayed into any graphics context.
  /// FIXME: The rendering resource observers (`addObserver`) come with the backend's resource caches.
  /// @note Not usable from multiple threads: the items' fonts aren't thread-safe.
  class DisplayList final : public ThreadSafeRefCounted<DisplayList>
  {
    KRYS_NON_COPYABLE(DisplayList);

  private:
    List<Item> _items;

  public:
    KRYS_NODISCARD static Ref<const DisplayList> Create(List<Item> &&items) noexcept
    {
      return AdoptRef(*new DisplayList(krys::move(items)));
    }

    ~DisplayList() noexcept;

    KRYS_NODISCARD Span<const Item> Items() const noexcept KRYS_LIFETIME_BOUND
    {
      return _items;
    }

    KRYS_NODISCARD utf8_string AsText(EnumSet<AsTextFlag>) const noexcept;
    void Dump(TextStream &) const noexcept;

  private:
    explicit DisplayList(List<Item> &&items) noexcept;
  };

  TextStream &operator<<(TextStream &, const DisplayList &) noexcept;
}
