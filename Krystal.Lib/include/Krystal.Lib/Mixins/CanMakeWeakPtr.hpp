#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/WeakPtrImpl.hpp"
#include "Krystal.Lib/Pointers/WeakRef.hpp"

namespace Krys::detail
{
  template <WeakPtrFactoryInitialization InitializationMode, typename TWeakPtrFactory>
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

    KRYS_NODISCARD uint32 GetRefCountWeak() const noexcept
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
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl,
            WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  using CanMakeWeakPtr =
    ::Krys::detail::CanMakeWeakPtrBase<InitializationMode, WeakPtrFactory<T, WeakPtrImpl>>;

  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl,
            WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  using CanMakeWeakPtrWithBitField =
    ::Krys::detail::CanMakeWeakPtrBase<InitializationMode, WeakPtrFactoryWithBitField<T, WeakPtrImpl>>;

  template <typename T, typename WeakPtrImpl = SingleThreadWeakPtrImpl,
            WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  using CanMakeSingleThreadWeakPtr = CanMakeWeakPtr<T, SingleThreadWeakPtrImpl>;

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  class AbstractRefCountedAndCanMakeWeakPtr : public AbstractRefCounted,
                                              public CanMakeWeakPtr<T, DefaultWeakPtrImpl, InitializationMode>
  {
  };

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  class AbstractRefCountedAndCanMakeSingleThreadWeakPtr
      : public AbstractRefCounted,
        public CanMakeSingleThreadWeakPtr<T, SingleThreadWeakPtrImpl, InitializationMode>
  {
  };
}
