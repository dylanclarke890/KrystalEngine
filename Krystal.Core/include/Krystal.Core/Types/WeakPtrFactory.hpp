/*
 * Copyright (C) 2013 Google, Inc. All rights reserved.
 * Copyright (C) 2015-2020 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/CompactRefPtrTuple.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Core/Types/WeakPtrImpl.hpp"

namespace krys::detail
{
  template <typename T, typename WeakPtrImpl, bool UseCompactPointer>
  class WeakPtrFactory
  {
    KRYS_NON_COPYABLE(WeakPtrFactory);

  public:
    using weak_value = T;
    using weak_value_impl = WeakPtrImpl;
    using weak_value_impl_ref_ptr =
      conditional_t<UseCompactPointer, CompactRefPtrTuple<WeakPtrImpl, uint16>, RefPtr<weak_value_impl>>;

  private:
    mutable weak_value_impl_ref_ptr _impl;

  public:
    constexpr WeakPtrFactory() noexcept = default;

    constexpr ~WeakPtrFactory() noexcept
    {
      if constexpr (UseCompactPointer)
      {
        if (auto *pointer = _impl.Ptr())
        {
          pointer->reset();
        }
      }
      else
      {
        if (_impl)
        {
          _impl->reset();
        }
      }
    }

    constexpr void InitializeIfNeeded(const T &object) const noexcept
    {
      static_assert(IsFinal<WeakPtrImpl>);

      if constexpr (UseCompactPointer)
      {
        if (_impl.Ptr())
        {
          return;
        }

        _impl.SetPtr(CreateRef<WeakPtrImpl>(const_cast<T *>(&object)));
      }
      else
      {
        if (_impl)
        {
          return;
        }

        _impl = CreateRef<WeakPtrImpl>(const_cast<T *>(&object));
      }
    }

    KRYS_NODISCARD constexpr WeakPtrImpl *Impl() const noexcept
    {
      if constexpr (UseCompactPointer)
      {
        return _impl.Ptr();
      }
      else
      {
        return _impl.get();
      }
    }

    template <typename U>
    KRYS_NODISCARD constexpr WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>> CreateWeakPtr(U &object) const noexcept
    {
      InitializeIfNeeded(object);

      if constexpr (UseCompactPointer)
      {
        krys_assert(&object == _impl.Ptr()->template get<T>());
        return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*_impl.Ptr());
      }
      else
      {
        krys_assert(&object == _impl->template get<T>());
        return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*_impl);
      }
    }

    constexpr void RevokeAll() noexcept
    {
      if constexpr (UseCompactPointer)
      {
        if (auto *pointer = _impl.Ptr())
        {
          pointer->reset();
          _impl.SetPtr(nullptr);
        }
      }
      else
      {
        if (RefPtr impl = std::exchange(_impl, nullptr))
        {
          impl->reset();
        }
      }
    }

    KRYS_NODISCARD constexpr uint32 GetRefCountWeak() const noexcept
    {
      if constexpr (UseCompactPointer)
      {
        if (auto *pointer = _impl.Ptr())
        {
          return pointer->GetRefCount() - 1;
        }

        return 0;
      }
      else
      {
        return _impl ? _impl->GetRefCount() - 1 : 0u;
      }
    }

    KRYS_NODISCARD constexpr uint16 Bitfield() const noexcept
    requires(UseCompactPointer)
    {
      return _impl.Data();
    }

    constexpr void SetBitfield(uint16 value) const noexcept
    requires(UseCompactPointer)
    {
      return _impl.SetData(value);
    }
  };
}

namespace krys
{
  template <typename T>
  using WeakPtrFactory = ::krys::detail::WeakPtrFactory<T, WeakPtrImpl, false>;

  template <typename T>
  using WeakPtrFactoryWithBitField = ::krys::detail::WeakPtrFactory<T, WeakPtrImpl, true>;

  template <typename T>
  using ThreadSafeWeakPtrFactory = ::krys::detail::WeakPtrFactory<T, ThreadSafeWeakPtrImpl, false>;

  template <typename T>
  using ThreadSafeWeakPtrFactoryWithBitField = ::krys::detail::WeakPtrFactory<T, ThreadSafeWeakPtrImpl, true>;
}
