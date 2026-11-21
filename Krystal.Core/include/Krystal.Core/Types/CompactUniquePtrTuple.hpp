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
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CompactPtrTuple.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <memory>

namespace krys
{
  template <typename T, typename TData, typename Deleter = std::default_delete<T>>
  class CompactUniquePtrTuple;

  template <typename T, typename TData, typename... Args>
  KRYS_NODISCARD constexpr CompactUniquePtrTuple<T, TData> CreateCompactUniquePtr(Args &&...args) noexcept
  {
    return CompactUniquePtrTuple<T, TData>(CreateUnique<T>(std::forward<Args>(args)...));
  }

  template <typename T, typename TData, typename Deleter, typename... Args>
  KRYS_NODISCARD constexpr CompactUniquePtrTuple<T, TData, Deleter>
    CreateCompactUniquePtr(Args &&...args) noexcept
  {
    return CompactUniquePtrTuple<T, TData, Deleter>(CreateUnique<T>(std::forward<Args>(args)...));
  }

  template <typename T, typename TData, typename Deleter>
  class CompactUniquePtrTuple
  {
    static_assert(krys::AllowsCompactPointers<T>());

    template <typename U, typename E, typename... Args>
    friend constexpr CompactUniquePtrTuple<U, E, Deleter> CreateCompactUniquePtr(Args &&...args) noexcept;

    template <typename U, typename E, typename D, typename... Args>
    friend constexpr CompactUniquePtrTuple<U, E, D> CreateCompactUniquePtr(Args &&...args) noexcept;

    template <typename, typename, typename>
    friend class CompactUniquePtrTuple;

    KRYS_NON_COPYABLE(CompactUniquePtrTuple);

  private:
    CompactPtrTuple<T *, TData> _data;

  public:
    constexpr CompactUniquePtrTuple() noexcept = default;

    template <typename U, typename UDeleter>
    requires(SameType<UDeleter, Deleter> || SameType<UDeleter, std::default_delete<U>>)
    constexpr CompactUniquePtrTuple(CompactUniquePtrTuple<U, TData, UDeleter> &&other) noexcept
        : _data {std::exchange(other._data, {})}
    {
    }

    constexpr ~CompactUniquePtrTuple() noexcept
    {
      SetPtr(nullptr);
    }

    template <typename U, typename UDeleter>
    requires(SameType<UDeleter, Deleter> || SameType<UDeleter, std::default_delete<U>>)
    CompactUniquePtrTuple<T, TData, Deleter> &
      operator=(CompactUniquePtrTuple<U, TData, UDeleter> &&other) noexcept
    {
      CompactUniquePtrTuple moved {krys::move(other)};
      std::swap(_data, moved._data);
      return *this;
    }

    KRYS_NODISCARD T *Ptr() const noexcept KRYS_LIFETIME_BOUND
    {
      return _data.Ptr();
    }

    KRYS_NODISCARD UniquePtr<T, Deleter> MoveToUniquePtr() noexcept
    {
      T *ptr = _data.Ptr();
      _data.SetPtr(nullptr);

      return UniquePtr<T, Deleter>(ptr);
    }

    void SetPtr(std::nullptr_t) noexcept
    {
      DeletePtr();
      _data.SetPtr(nullptr);
    }

    template <typename U, typename UDeleter>
    requires(SameType<UDeleter, Deleter> || SameType<UDeleter, std::default_delete<U>>)
    void SetPtr(UniquePtr<U, UDeleter> &&ptr) noexcept
    {
      DeletePtr();
      _data.SetPtr(ptr.release());
    }

    KRYS_NODISCARD TData Data() const noexcept
    {
      return _data.Data();
    }

    void SetData(TData data) noexcept
    {
      _data.SetData(data);
    }

  private:
    constexpr CompactUniquePtrTuple(std::unique_ptr<T> &&ptr) noexcept
    {
      _data.SetPtr(ptr.release());
    }

    constexpr void DeletePtr() noexcept
    {
      if (T *ptr = _data.Ptr())
      {
        Deleter()(ptr);
      }
    }
  };
}
