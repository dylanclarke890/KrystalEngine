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
#include "Krystal.Core/Types/WeakPtrFactory.hpp"

namespace krys::detail
{
#define KRYS_USING_CAN_MAKE_WEAKPTR(Base)                                                                    \
  using Base::GetRefCountWeak;                                                                               \
  using Base::WeakImpl;                                                                                      \
  using Base::WeakImplIfExists;                                                                              \
  using Base::weak_value;                                                                                    \
  using Base::weak_pointer_impl;

  // Uses lazy initialization by default to avoid unnecessary initialization. Eager initialization is
  // useful if you plan to call construct WeakPtrs from other threads.
  template <typename TFactory, bool LazilyInitialize>
  class CanMakeWeakPtr
  {
  public:
    using weak_value = typename TFactory::weak_value;
    using weak_pointer_impl = typename TFactory::weak_value_impl;
    using weak_pointer_factory = TFactory;

  private:
    weak_pointer_factory _weakPtrFactory;

  protected:
    constexpr CanMakeWeakPtr() noexcept
    {
      if constexpr (!LazilyInitialize)
      {
        InitializeWeakPtrFactory();
      }
    }

    constexpr CanMakeWeakPtr(const CanMakeWeakPtr &) noexcept
    {
      if constexpr (!LazilyInitialize)
      {
        InitializeWeakPtrFactory();
      }
    }

  public:
    KRYS_NODISCARD constexpr uint32 GetRefCountWeak() const noexcept
    {
      return _weakPtrFactory.GetRefCountWeak();
    }

    KRYS_NODISCARD constexpr weak_pointer_impl *WeakImplIfExists() const noexcept
    {
      return _weakPtrFactory.Impl();
    }

    KRYS_NODISCARD constexpr weak_pointer_impl &WeakImpl() const noexcept
    {
      InitializeWeakPtrFactory();
      return *_weakPtrFactory.Impl();
    }

  protected:
    constexpr void InitializeWeakPtrFactory() const noexcept
    {
      _weakPtrFactory.InitializeIfNeeded(static_cast<const weak_value &>(*this));
    }

    KRYS_NODISCARD constexpr weak_pointer_factory &GetWeakPtrFactory() noexcept
    {
      return _weakPtrFactory;
    }

    KRYS_NODISCARD constexpr const weak_pointer_factory &GetWeakPtrFactory() const noexcept
    {
      return _weakPtrFactory;
    }

    constexpr CanMakeWeakPtr &operator=(const CanMakeWeakPtr &) noexcept
    {
      return *this;
    }
  };
}

namespace krys
{
  template <typename T, bool LazilyInitialize = true>
  using CanMakeWeakPtr = ::krys::detail::CanMakeWeakPtr<WeakPtrFactory<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  using CanMakeWeakPtrWithBitField =
    ::krys::detail::CanMakeWeakPtr<WeakPtrFactoryWithBitField<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  using CanMakeThreadSafeWeakPtr =
    ::krys::detail::CanMakeWeakPtr<ThreadSafeWeakPtrFactory<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  using CanMakeThreadSafeWeakPtrWithBitField =
    ::krys::detail::CanMakeWeakPtr<ThreadSafeWeakPtrFactoryWithBitField<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  class AbstractRefCountedAndCanMakeWeakPtr : public AbstractRefCounted,
                                              public CanMakeWeakPtr<T, LazilyInitialize>
  {
  };

  template <typename T, bool LazilyInitialize = true>
  class AbstractRefCountedAndCanMakeThreadSafeWeakPtr : public AbstractRefCounted,
                                                        public CanMakeThreadSafeWeakPtr<T, LazilyInitialize>
  {
  };
}
