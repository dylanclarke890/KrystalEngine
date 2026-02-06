#pragma once

#include "Krystal.Lib/Pointers/RefCounted/WeakPtrFactory.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys
{
  class DefaultWeakPtrImpl;

  template <typename WeakPtrFactoryType,
            WeakPtrFactoryInitialization initializationMode = WeakPtrFactoryInitialization::Lazy>
  class CanMakeWeakPtrBase
  {
  public:
    using WeakValueType = typename WeakPtrFactoryType::ObjectType;
    using WeakPtrImplType = typename WeakPtrFactoryType::WeakPtrImplType;

    WeakPtrImplType *weakImplIfExists() const
    {
      return m_weakPtrFactory.impl();
    }
    WeakPtrImplType &weakImpl() const
    {
      initializeWeakPtrFactory();
      return *m_weakPtrFactory.impl();
    }
    unsigned weakCount() const
    {
      return m_weakPtrFactory.weakPtrCount();
    }

  protected:
    CanMakeWeakPtrBase()
    {
      if (initializationMode == WeakPtrFactoryInitialization::Eager)
        initializeWeakPtrFactory();
    }

    CanMakeWeakPtrBase(const CanMakeWeakPtrBase &)
    {
      if (initializationMode == WeakPtrFactoryInitialization::Eager)
        initializeWeakPtrFactory();
    }

    CanMakeWeakPtrBase &operator=(const CanMakeWeakPtrBase &)
    {
      return *this;
    }

    void initializeWeakPtrFactory() const
    {
      m_weakPtrFactory.initializeIfNeeded(static_cast<const WeakValueType &>(*this));
    }

    const WeakPtrFactoryType &weakPtrFactory() const
    {
      return m_weakPtrFactory;
    }
    WeakPtrFactoryType &weakPtrFactory()
    {
      return m_weakPtrFactory;
    }

  private:
    WeakPtrFactoryType m_weakPtrFactory;
  };

  template <typename T, WeakPtrFactoryInitialization initializationMode = WeakPtrFactoryInitialization::Lazy,
            typename WeakPtrImpl = DefaultWeakPtrImpl>
  using CanMakeWeakPtr = CanMakeWeakPtrBase<WeakPtrFactory<T, WeakPtrImpl>, initializationMode>;

  template <typename T, WeakPtrFactoryInitialization initializationMode = WeakPtrFactoryInitialization::Lazy,
            typename WeakPtrImpl = DefaultWeakPtrImpl>
  using CanMakeWeakPtrWithBitField =
    CanMakeWeakPtrBase<WeakPtrFactoryWithBitField<T, WeakPtrImpl>, initializationMode>;

  template <typename T, WeakPtrFactoryInitialization initializationMode = WeakPtrFactoryInitialization::Lazy>
  using CanMakeSingleThreadWeakPtr = CanMakeWeakPtr<T, initializationMode, SingleThreadWeakPtrImpl>;
}
