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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CompactPtrTuple.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys
{
  template <typename T>
  using CompactRefPtr = RefPtr<T, CompactPtrTraits<T>>;

  template <typename T, typename TData>
  class CompactRefPtrTuple final
  {
    static_assert(krys::AllowsCompactPointers<T>());

  private:
    CompactPtrTuple<T *, TData> _data;

  public:
    constexpr CompactRefPtrTuple() noexcept = default;

    constexpr CompactRefPtrTuple(T *ptr, TData data) noexcept
    {
      SetPtr(ptr);
      SetData(data);
    }

    constexpr CompactRefPtrTuple(RefPtr<T> &&ptr, TData data) noexcept
    {
      SetPtr(krys::move(ptr));
      SetData(data);
    }

    constexpr ~CompactRefPtrTuple() noexcept
    {
      RefPtr<T>::ref_policy::SubRef(_data.Ptr());
    }

    constexpr CompactRefPtrTuple(const CompactRefPtrTuple &other) noexcept
    {
      SetPtr(other.Ptr());
      SetData(other.Data());
    }

    constexpr CompactRefPtrTuple(CompactRefPtrTuple &&other) noexcept
    {
      _data.SetPtr(other.Ptr());
      _data.SetData(other.Data());
      other._data.SetPtr(nullptr);
      other._data.SetData({});
    }

    constexpr CompactRefPtrTuple &operator=(const CompactRefPtrTuple &other) noexcept
    {
      CompactRefPtrTuple copied(other);
      swap(copied);
      return *this;
    }

    constexpr CompactRefPtrTuple &operator=(CompactRefPtrTuple &&other) noexcept
    {
      CompactRefPtrTuple moved(krys::move(other));
      swap(moved);
      return *this;
    }

    KRYS_NODISCARD constexpr T *Ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.Ptr();
    }

    constexpr void SetPtr(T *ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(RefPtr<T>::ref_policy::AddRef(ptr));
      RefPtr<T>::ref_policy::SubRef(old);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved): taken over by release().
    constexpr void SetPtr(RefPtr<T> &&ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(ptr.release());
      RefPtr<T>::ref_policy::SubRef(old);
    }

    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved): taken over by release().
    constexpr void SetPtr(Ref<T> &&ptr) noexcept
    {
      auto *old = _data.Ptr();
      _data.SetPtr(&ptr.release());
      RefPtr<T>::ref_policy::SubRef(old);
    }

    KRYS_NODISCARD constexpr TData Data() const noexcept
    {
      return _data.Data();
    }

    constexpr void SetData(TData data) noexcept
    {
      _data.SetData(data);
    }

    constexpr void swap(CompactRefPtrTuple<T, TData> &other) noexcept
    {
      _data.swap(other._data);
    }
  };
}
