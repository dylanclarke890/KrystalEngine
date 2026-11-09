/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 * Copyright (C) 2003-2024 Apple Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style::calc
{
  /// @brief Maps integer handles to calculation values, so that a calculated length fits in the 32 bits of a
  /// float. Each handle carries its own reference count.
  class ValueMap
  {
    friend class NeverDestroyed<ValueMap>;

  private:
    struct Entry
    {
      uint64 ReferenceCountMinusOne {0};
      RefPtr<Value> Value;

      Entry() noexcept = default;
      explicit Entry(Ref<calc::Value> &&value) noexcept;
    };

    uint32 _nextAvailableHandle {1};
    HashMap<uint32, Entry> _map;

  public:
    KRYS_NODISCARD static ValueMap &CalculationValues() noexcept;

    KRYS_NODISCARD uint32 Insert(Ref<Value> &&) noexcept;
    void AddRef(uint32 handle) noexcept;
    void SubRef(uint32 handle) noexcept;

    KRYS_NODISCARD Value &Get(uint32 handle) const noexcept;

  private:
    ValueMap() noexcept;

    // Handles 0 and the maximum value are reserved, matching a hash table's empty and deleted keys.
    KRYS_NODISCARD constexpr static bool IsValidHandle(uint32 handle) noexcept
    {
      return handle != 0 && handle != Limits<uint32>::max();
    }
  };

  inline ValueMap::Entry::Entry(Ref<calc::Value> &&value) noexcept : Value(krys::move(value))
  {
  }

  inline uint32 ValueMap::Insert(Ref<Value> &&value) noexcept
  {
    krys_debug_assert(_nextAvailableHandle);

    Entry entry(krys::move(value));

    // FIXME: This monotonically increasing handle generation scheme is potentially wasteful
    // of the handle space. Consider reusing empty handles. https://bugs.webkit.org/show_bug.cgi?id=80489
    while (!IsValidHandle(_nextAvailableHandle) || !_map.try_emplace(_nextAvailableHandle, entry).second)
    {
      ++_nextAvailableHandle;
    }

    return _nextAvailableHandle++;
  }

  inline Value &ValueMap::Get(uint32 handle) const noexcept
  {
    krys_debug_assert(_map.contains(handle));

    return *_map.find(handle)->second.Value;
  }

  inline void ValueMap::AddRef(uint32 handle) noexcept
  {
    krys_debug_assert(_map.contains(handle));

    ++_map.find(handle)->second.ReferenceCountMinusOne;
  }

  inline void ValueMap::SubRef(uint32 handle) noexcept
  {
    krys_debug_assert(_map.contains(handle));

    auto it = _map.find(handle);
    if (it->second.ReferenceCountMinusOne)
    {
      --it->second.ReferenceCountMinusOne;
      return;
    }

    _map.erase(it);
  }
}
