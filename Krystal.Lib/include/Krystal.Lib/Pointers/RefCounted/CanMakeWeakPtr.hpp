#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakPtrImpl.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys::detail
{
  template <typename TWeakPtrFactory, WeakPtrFactoryInitialization InitializationMode>
  class CanMakeWeakPtrBase
  {
  public:
    using TWeakValue = typename TWeakPtrFactory::TObject;
    using TWeakPtrImpl = typename TWeakPtrFactory::TWeakPtrImpl;

  private:
    TWeakPtrFactory _weakPtrFactory;

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

  public:
    KRYS_NODISCARD RawPtr<TWeakPtrImpl> WeakImplIfExists() const noexcept
    {
      return _weakPtrFactory.Impl();
    }

    KRYS_NODISCARD TWeakPtrImpl &WeakImpl() const noexcept
    {
      InitializeWeakPtrFactory();
      return *_weakPtrFactory.Impl();
    }

    KRYS_NODISCARD uint32 WeakCount() const noexcept
    {
      return _weakPtrFactory.WeakPtrCount();
    }

  protected:
    void InitializeWeakPtrFactory() const noexcept
    {
      _weakPtrFactory.InitializeIfNeeded(static_cast<const TWeakValue &>(*this));
    }

    CanMakeWeakPtrBase &operator=(const CanMakeWeakPtrBase &) noexcept
    {
      return *this;
    }

    KRYS_NODISCARD const TWeakPtrFactory &GetWeakPtrFactory() const noexcept
    {
      return _weakPtrFactory;
    }

    KRYS_NODISCARD TWeakPtrFactory &GetWeakPtrFactory() noexcept
    {
      return _weakPtrFactory;
    }
  };
}

namespace Krys
{
  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy,
            typename WeakPtrImpl = DefaultWeakPtrImpl>
  using CanMakeWeakPtr =
    ::Krys::detail::CanMakeWeakPtrBase<WeakPtrFactory<T, WeakPtrImpl>, InitializationMode>;

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy,
            typename WeakPtrImpl = DefaultWeakPtrImpl>
  using CanMakeWeakPtrWithBitField =
    ::Krys::detail::CanMakeWeakPtrBase<WeakPtrFactoryWithBitField<T, WeakPtrImpl>, InitializationMode>;

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  using CanMakeSingleThreadWeakPtr = CanMakeWeakPtr<T, InitializationMode, SingleThreadWeakPtrImpl>;
}
