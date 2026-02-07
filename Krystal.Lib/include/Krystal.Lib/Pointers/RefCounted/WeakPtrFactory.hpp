#pragma once

#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactRefPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys
{
#define USING_CAN_MAKE_WEAKPTR(BASE)                                                                         \
  using BASE::weakImpl;                                                                                      \
  using BASE::weakImplIfExists;                                                                              \
  using BASE::weakCount;                                                                                     \
  using BASE::WeakValueType;                                                                                 \
  using BASE::WeakPtrImplType;

  // Note: you probably want to inherit from CanMakeWeakPtr rather than use this directly.
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl>
  class WeakPtrFactory : public NonCopyable<WeakPtrFactory<T, WeakPtrImpl>>
  {
  public:
    using ObjectType = T;
    using WeakPtrImplType = WeakPtrImpl;

    WeakPtrFactory()
    {
    }

    ~WeakPtrFactory()
    {
      if (m_impl)
        m_impl->clear();
    }

    WeakPtrImpl *impl() const
    {
      return m_impl.get();
    }

    void initializeIfNeeded(const T &object) const
    {
      if (m_impl)
        return;

      static_assert(std::is_final_v<WeakPtrImpl>);
      m_impl = adoptRef(*new WeakPtrImpl(const_cast<T *>(&object)));
    }

    template <typename U>
    WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>
      createWeakPtr(U &object, EnableWeakPtrThreadingAssertions enableWeakPtrThreadingAssertions =
                                 EnableWeakPtrThreadingAssertions::Yes) const
    {
      initializeIfNeeded(object);

      assert(&object == m_impl->template get<T>());
      return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*m_impl, enableWeakPtrThreadingAssertions);
    }

    void revokeAll()
    {
      if (RefPtr impl = std::exchange(m_impl, nullptr))
        impl->clear();
    }

    unsigned weakPtrCount() const
    {
      return m_impl ? m_impl->refCount() - 1 : 0u;
    }

#if KRYS_ENV(DEV)
    bool isInitialized() const
    {
      return m_impl;
    }
#endif

  private:
    template <typename, typename, typename>
    friend class WeakPtr;
    template <typename, typename>
    friend class WeakRef;

    mutable RefPtr<WeakPtrImpl> m_impl;
  };

  // Note: you probably want to inherit from CanMakeWeakPtrWithBitField rather than use this directly.
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl>
  class WeakPtrFactoryWithBitField : public NonCopyable<WeakPtrFactoryWithBitField<T, WeakPtrImpl>>
  {
  public:
    using ObjectType = T;
    using WeakPtrImplType = WeakPtrImpl;

    WeakPtrFactoryWithBitField()
    {
    }

    ~WeakPtrFactoryWithBitField()
    {
      if (auto *pointer = m_impl.pointer())
        pointer->clear();
    }

    WeakPtrImpl *impl() const
    {
      return m_impl.pointer();
    }

    void initializeIfNeeded(const T &object) const
    {
      if (m_impl.pointer())
        return;

      static_assert(std::is_final_v<WeakPtrImpl>);
      m_impl.setPointer(adoptRef(*new WeakPtrImpl(const_cast<T *>(&object))));
    }

    template <typename U>
    WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>
      createWeakPtr(U &object, EnableWeakPtrThreadingAssertions enableWeakPtrThreadingAssertions =
                                 EnableWeakPtrThreadingAssertions::Yes) const
    {
      initializeIfNeeded(object);

      assert(&object == m_impl.pointer()->template get<T>());
      return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*m_impl.pointer(), enableWeakPtrThreadingAssertions);
    }

    void revokeAll()
    {
      if (auto *pointer = m_impl.pointer())
      {
        pointer->clear();
        m_impl.setPointer(nullptr);
      }
    }

    unsigned weakPtrCount() const
    {
      if (auto *pointer = m_impl.pointer())
        return pointer->refCount() - 1;
      return 0;
    }

#if KRYS_ENV(DEV)
    bool isInitialized() const
    {
      return m_impl.pointer();
    }
#endif

    uint16_t bitfield() const
    {
      return m_impl.type();
    }
    void setBitfield(uint16_t value) const
    {
      return m_impl.setType(value);
    }

  private:
    template <typename, typename, typename>
    friend class WeakPtr;

    template <typename, typename>
    friend class WeakRef;

    mutable CompactRefPtrTuple<WeakPtrImpl, uint16_t> m_impl;
  };

  // We use lazy initialization of the WeakPtrFactory by default to avoid unnecessary initialization. Eager
  // initialization is however useful if you plan to call construct WeakPtrs from other threads.
  enum class WeakPtrFactoryInitialization
  {
    Lazy,
    Eager
  };

}
