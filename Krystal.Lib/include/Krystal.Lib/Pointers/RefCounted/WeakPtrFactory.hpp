#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactRefPtrTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/WeakRef.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys
{
#define KRYS_USING_CAN_MAKE_WEAKPTR(Base)                                                                    \
  using Base::WeakImpl;                                                                                      \
  using Base::WeakImplIfExists;                                                                              \
  using Base::WeakCount;                                                                                     \
  using Base::TWeakValue;                                                                                    \
  using Base::TWeakPtrImpl;

  // Note: you probably want to inherit from CanMakeWeakPtr rather than use this directly.
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl>
  class WeakPtrFactory : public NonCopyable<WeakPtrFactory<T, WeakPtrImpl>>
  {
  public:
    using TObject = T;
    using TWeakPtrImpl = WeakPtrImpl;

  private:
    template <typename, typename, typename>
    friend class WeakPtr;

    template <typename, typename>
    friend class WeakRef;

    mutable RefPtr<WeakPtrImpl> _impl;

  public:
    WeakPtrFactory() noexcept = default;

    ~WeakPtrFactory() noexcept
    {
      if (_impl)
      {
        _impl->clear();
      }
    }

#if KRYS_ENV(DEV)
    KRYS_NODISCARD bool IsInitialized() const noexcept
    {
      return _impl;
    }
#endif

    KRYS_NODISCARD RawPtr<WeakPtrImpl> Impl() const noexcept
    {
      return _impl.get();
    }

    void InitializeIfNeeded(const T &object) const noexcept
    {
      if (_impl)
      {
        return;
      }

      static_assert(Final<WeakPtrImpl>);
      _impl = adoptRef(*new WeakPtrImpl(const_cast<RawPtr<T>>(&object)));
    }

    template <typename U>
    KRYS_NODISCARD WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>> CreateWeakPtr(
      U &object, EnabledWeakPtrThreadAsserts enableAsserts = EnabledWeakPtrThreadAsserts::Yes) const noexcept
    {
      InitializeIfNeeded(object);

      assert(&object == _impl->template get<T>());
      return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*_impl, enableAsserts);
    }

    void RevokeAll() noexcept
    {
      if (RefPtr impl = std::exchange(_impl, nullptr))
      {
        impl->clear();
      }
    }

    KRYS_NODISCARD uint32 WeakPtrCount() const noexcept
    {
      return _impl ? _impl->refCount() - 1 : 0u;
    }
  };

  // Note: you probably want to inherit from CanMakeWeakPtrWithBitField rather than use this directly.
  template <typename T, typename WeakPtrImpl = DefaultWeakPtrImpl>
  class WeakPtrFactoryWithBitField : public NonCopyable<WeakPtrFactoryWithBitField<T, WeakPtrImpl>>
  {
  public:
    using TObject = T;
    using TWeakPtrImpl = WeakPtrImpl;

  private:
    template <typename, typename, typename>
    friend class WeakPtr;

    template <typename, typename>
    friend class WeakRef;

    mutable CompactRefPtrTuple<WeakPtrImpl, uint16> _impl;

  public:
    WeakPtrFactoryWithBitField() noexcept
    {
    }

    ~WeakPtrFactoryWithBitField() noexcept
    {
      if (auto *pointer = _impl.pointer())
      {
        pointer->clear();
      }
    }

#if KRYS_ENV(DEV)
    KRYS_NODISCARD bool IsInitialized() const noexcept
    {
      return _impl.pointer();
    }
#endif

    KRYS_NODISCARD RawPtr<WeakPtrImpl> Impl() const noexcept
    {
      return _impl.pointer();
    }

    void InitializeIfNeeded(const T &object) const noexcept
    {
      if (_impl.pointer())
      {
        return;
      }

      static_assert(Final<WeakPtrImpl>);
      _impl.setPointer(adoptRef(*new WeakPtrImpl(const_cast<RawPtr<T>>(&object))));
    }

    template <typename U>
    KRYS_NODISCARD WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>> CreateWeakPtr(
      U &object, EnabledWeakPtrThreadAsserts enableAsserts = EnabledWeakPtrThreadAsserts::Yes) const noexcept
    {
      InitializeIfNeeded(object);

      assert(&object == _impl.pointer()->template get<T>());
      return WeakPtr<U, WeakPtrImpl, RawPtrTraits<U>>(*_impl.pointer(), enableAsserts);
    }

    void RevokeAll() noexcept
    {
      if (auto *pointer = _impl.pointer())
      {
        pointer->clear();
        _impl.setPointer(nullptr);
      }
    }

    KRYS_NODISCARD uint32 WeakPtrCount() const noexcept
    {
      if (auto *pointer = _impl.pointer())
      {
        return pointer->refCount() - 1;
      }

      return 0;
    }

    KRYS_NODISCARD uint16 Bitfield() const noexcept
    {
      return _impl.type();
    }

    void SetBitfield(uint16 value) const noexcept
    {
      return _impl.setType(value);
    }
  };

  // We use lazy initialization of the WeakPtrFactory by default to avoid unnecessary initialization. Eager
  // initialization is however useful if you plan to call construct WeakPtrs from other threads.
  enum class WeakPtrFactoryInitialization
  {
    Lazy,
    Eager
  };
}
