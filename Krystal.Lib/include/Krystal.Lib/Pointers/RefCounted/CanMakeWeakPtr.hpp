#pragma once

#include "Krystal.Lib/Pointers/RefCounted/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys
{
  class DefaultWeakPtrImpl;

  template <typename TWeakPtrFactory,
            WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  class CanMakeWeakPtrBase
  {
  public:
    using TWeakValue = typename TWeakPtrFactory::ObjectType;
    using TWeakPtrImpl = typename TWeakPtrFactory::TWeakPtrImpl;

    TWeakPtrImpl *WeakImplIfExists() const noexcept
    {
      return _weakPtrFactory.Impl();
    }

    TWeakPtrImpl &WeakImpl() const noexcept
    {
      InitializeWeakPtrFactory();
      return *_weakPtrFactory.Impl();
    }

    uint32 weakCount() const noexcept
    {
      return _weakPtrFactory.weakPtrCount();
    }

  protected:
    CanMakeWeakPtrBase() noexcept
    {
      if constexpr (InitializationMode == WeakPtrFactoryInitialization::Eager)
      {
        InitializeWeakPtrFactory();
      }
    }

    CanMakeWeakPtrBase(const CanMakeWeakPtrBase &) noexcept
    {
      if constexpr (InitializationMode == WeakPtrFactoryInitialization::Eager)
      {
        InitializeWeakPtrFactory();
      }
    }

    CanMakeWeakPtrBase &operator=(const CanMakeWeakPtrBase &)
    {
      return *this;
    }

    void InitializeWeakPtrFactory() const
    {
      _weakPtrFactory.InitializeIfNeeded(static_cast<const TWeakValue &>(*this));
    }

    const TWeakPtrFactory &weakPtrFactory() const
    {
      return _weakPtrFactory;
    }
    TWeakPtrFactory &weakPtrFactory()
    {
      return _weakPtrFactory;
    }

  private:
    TWeakPtrFactory _weakPtrFactory;
  };

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy,
            typename WeakPtrImpl = DefaultWeakPtrImpl>
  using CanMakeWeakPtr = CanMakeWeakPtrBase<WeakPtrFactory<T, WeakPtrImpl>, InitializationMode>;

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy,
            typename WeakPtrImpl = DefaultWeakPtrImpl>
  using CanMakeWeakPtrWithBitField =
    CanMakeWeakPtrBase<WeakPtrFactoryWithBitField<T, WeakPtrImpl>, InitializationMode>;

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  using CanMakeSingleThreadWeakPtr = CanMakeWeakPtr<T, InitializationMode, SingleThreadWeakPtrImpl>;
}
