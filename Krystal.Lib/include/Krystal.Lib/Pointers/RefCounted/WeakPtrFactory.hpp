#pragma once

#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactRefPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"

namespace Krys
{
#define USING_CAN_MAKE_WEAKPTR(BASE)                                                                         \
  using BASE::WeakImpl;                                                                                      \
  using BASE::WeakImplIfExists;                                                                              \
  using BASE::weakCount;                                                                                     \
  using BASE::TWeakValue;                                                                                    \
  using BASE::TWeakPtrImpl;

  // Note: you probably want to inherit from CanMakeWeakPtr rather than use this directly.
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl>
  class WeakPtrFactory : public NonCopyable<WeakPtrFactory<T, WeakPtrImpl>>
  {
  public:
    using ObjectType = T;
    using TWeakPtrImpl = WeakPtrImpl;

    WeakPtrFactory() noexcept = default;

    ~WeakPtrFactory() noexcept
    {
      if (_impl)
      {
        _impl->clear();
      }
    }

    WeakPtrImpl *Impl() const
    {
      return _impl.get();
    }

    void InitializeIfNeeded(const T &object) const
    {
      if (_impl)
      {
        return;
      }

      static_assert(std::is_final_v<WeakPtrImpl>);
      _impl = adoptRef(*new WeakPtrImpl(const_cast<T *>(&object)));
    }

    template <typename U>
    WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>
      createWeakPtr(U &object, EnabledWeakPtrThreadAsserts enableWeakPtrThreadingAssertions =
                                 EnabledWeakPtrThreadAsserts::Yes) const
    {
      InitializeIfNeeded(object);

      assert(&object == _impl->template get<T>());
      return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*_impl, enableWeakPtrThreadingAssertions);
    }

    void revokeAll()
    {
      if (RefPtr impl = std::exchange(_impl, nullptr))
        impl->clear();
    }

    unsigned weakPtrCount() const
    {
      return _impl ? _impl->refCount() - 1 : 0u;
    }

#if KRYS_ENV(DEV)
    bool isInitialized() const
    {
      return _impl;
    }
#endif

  private:
    template <typename, typename, typename>
    friend class WeakPtr;
    template <typename, typename>
    friend class WeakRef;

    mutable RefPtr<WeakPtrImpl> _impl;
  };

  // Note: you probably want to inherit from CanMakeWeakPtrWithBitField rather than use this directly.
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl>
  class WeakPtrFactoryWithBitField : public NonCopyable<WeakPtrFactoryWithBitField<T, WeakPtrImpl>>
  {
  public:
    using ObjectType = T;
    using TWeakPtrImpl = WeakPtrImpl;

    WeakPtrFactoryWithBitField()
    {
    }

    ~WeakPtrFactoryWithBitField()
    {
      if (auto *pointer = _impl.pointer())
        pointer->clear();
    }

    WeakPtrImpl *Impl() const
    {
      return _impl.pointer();
    }

    void InitializeIfNeeded(const T &object) const
    {
      if (_impl.pointer())
        return;

      static_assert(std::is_final_v<WeakPtrImpl>);
      _impl.setPointer(adoptRef(*new WeakPtrImpl(const_cast<T *>(&object))));
    }

    template <typename U>
    WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>
      createWeakPtr(U &object, EnabledWeakPtrThreadAsserts enableWeakPtrThreadingAssertions =
                                 EnabledWeakPtrThreadAsserts::Yes) const
    {
      InitializeIfNeeded(object);

      assert(&object == _impl.pointer()->template get<T>());
      return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*_impl.pointer(), enableWeakPtrThreadingAssertions);
    }

    void revokeAll()
    {
      if (auto *pointer = _impl.pointer())
      {
        pointer->clear();
        _impl.setPointer(nullptr);
      }
    }

    unsigned weakPtrCount() const
    {
      if (auto *pointer = _impl.pointer())
        return pointer->refCount() - 1;
      return 0;
    }

#if KRYS_ENV(DEV)
    bool isInitialized() const
    {
      return _impl.pointer();
    }
#endif

    uint16_t bitfield() const
    {
      return _impl.type();
    }
    void setBitfield(uint16_t value) const
    {
      return _impl.setType(value);
    }

  private:
    template <typename, typename, typename>
    friend class WeakPtr;

    template <typename, typename>
    friend class WeakRef;

    mutable CompactRefPtrTuple<WeakPtrImpl, uint16_t> _impl;
  };

  // We use lazy initialization of the WeakPtrFactory by default to avoid unnecessary initialization. Eager
  // initialization is however useful if you plan to call construct WeakPtrs from other threads.
  enum class WeakPtrFactoryInitialization
  {
    Lazy,
    Eager
  };

}
