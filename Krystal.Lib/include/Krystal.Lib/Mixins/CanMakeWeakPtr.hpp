#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtrFactory.hpp"

namespace Krys::detail
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

    KRYS_NODISCARD constexpr RawPtr<weak_pointer_impl> WeakImplIfExists() const noexcept
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

namespace Krys
{
  template <typename T, bool LazilyInitialize = true>
  using CanMakeWeakPtr = ::Krys::detail::CanMakeWeakPtr<WeakPtrFactory<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  using CanMakeWeakPtrWithBitField =
    ::Krys::detail::CanMakeWeakPtr<WeakPtrFactoryWithBitField<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  using CanMakeThreadSafeWeakPtr =
    ::Krys::detail::CanMakeWeakPtr<ThreadSafeWeakPtrFactory<T>, LazilyInitialize>;

  template <typename T, bool LazilyInitialize = true>
  using CanMakeThreadSafeWeakPtrWithBitField =
    ::Krys::detail::CanMakeWeakPtr<ThreadSafeWeakPtrFactoryWithBitField<T>, LazilyInitialize>;

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
