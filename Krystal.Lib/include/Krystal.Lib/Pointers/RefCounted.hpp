#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <atomic>
#include <cassert>
#include <limits>

namespace Krys
{
  enum class RefCountedFlags : uint8
  {
    None = 0,
    ProvideWeakReferences = 1,
    SingleThreaded = 2
  };

  template <>
  constexpr inline bool EnableEnumFlags<RefCountedFlags> = true;

  template <RefCountedFlags Flags>
  using DefaultRefCountType =
    conditional_t<HasFlag(Flags, RefCountedFlags::ProvideWeakReferences), intptr_t, int32>;

  template <typename Derived, RefCountedFlags Flags = RefCountedFlags::None,
            typename TCount = DefaultRefCountType<Flags>>
  class RefCounted;

  template <typename T, RefCountedFlags Flags = RefCountedFlags::None,
            typename TCount = DefaultRefCountType<Flags>>
  class RefCountedAdapter;

  template <typename T, RefCountedFlags Flags = RefCountedFlags::None,
            typename TCount = DefaultRefCountType<Flags>>
  class RefCountedWrapper;

  template <typename Derived>
  using WeakRefCounted = RefCounted<Derived, RefCountedFlags::ProvideWeakReferences>;

  template <typename Derived>
  using WeakRefCountedAdapter = RefCountedAdapter<Derived, RefCountedFlags::ProvideWeakReferences>;

  template <typename Derived>
  using WeakRefCountedWrapper = RefCountedWrapper<Derived, RefCountedFlags::ProvideWeakReferences>;

  template <typename Derived, typename TCount = DefaultRefCountType<RefCountedFlags::SingleThreaded>>
  using SingleThreadRefCounted = RefCounted<Derived, RefCountedFlags::SingleThreaded, TCount>;

  template <typename Derived, typename TCount = DefaultRefCountType<RefCountedFlags::SingleThreaded>>
  using SingleThreadRefCountedAdapter = RefCountedAdapter<Derived, RefCountedFlags::SingleThreaded, TCount>;

  template <typename Derived, typename TCount = DefaultRefCountType<RefCountedFlags::SingleThreaded>>
  using SingleThreadRefCountedWrapper = RefCountedWrapper<Derived, RefCountedFlags::SingleThreaded, TCount>;

  template <typename Derived>
  using SingleThreadWeakRefCounted =
    RefCounted<Derived, RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>;

  template <typename Derived>
  using SingleThreadWeakRefCountedAdapter =
    RefCountedAdapter<Derived, RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>;

  template <typename Derived>
  using SingleThreadWeakRefCountedWrapper =
    RefCountedWrapper<Derived, RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>;

  template <typename Owner>
  class WeakReference;

  struct RefCountedTraits
  {
    template <typename T>
    static void AddRef(const T *obj) noexcept
    {
      obj->CallAddRef();
    }

    template <typename T>
    static void SubRef(const T *obj) noexcept
    {
      obj->CallSubRef();
    }
  };

  template <typename Derived, RefCountedFlags Flags, typename TCount>
  class RefCounted
  {
  public:
    static constexpr bool ProvidesWeakReferences = HasFlag(Flags, RefCountedFlags::ProvideWeakReferences);
    static constexpr bool SingleThreaded = HasFlag(Flags, RefCountedFlags::SingleThreaded);

  private:
    static_assert(!RefCounted::ProvidesWeakReferences || SameType<TCount, intptr_t>,
                  "TCount must be intptr_t (the default) when providing weak references");

    static_assert(Integral<TCount>, "TCount must be an integral type");

    static_assert(RefCounted::SingleThreaded || std::atomic<TCount>::is_always_lock_free,
                  "TCount must be such that std::atomic<TCount> is always lock free");

    template <typename Owner>
    friend class WeakReference;

    friend RefCountedTraits;

  public:
    using RefPtrTraits = RefCountedTraits;
    using RefCountedBase = RefCounted;

    using weak_value_type = conditional_t<RefCounted::ProvidesWeakReferences, WeakReference<Derived>, void>;

    using weak_ptr = conditional_t<RefCounted::ProvidesWeakReferences,
                                   IntrusivePtr<weak_value_type, RefCountedTraits>, void>;

    using const_weak_ptr = conditional_t<RefCounted::ProvidesWeakReferences,
                                         IntrusivePtr<const weak_value_type, RefCountedTraits>, void>;

    using count_type = conditional_t<SingleThreaded, TCount, std::atomic<TCount>>;

    RefCounted(const RefCounted &) noexcept = delete;
    RefCounted &operator=(const RefCounted &) noexcept = delete;
    RefCounted(RefCounted &&) noexcept = delete;
    RefCounted &operator=(RefCounted &&) noexcept = delete;

    void AddRef() const noexcept
    {
      if constexpr (!RefCounted::ProvidesWeakReferences)
      {
        if constexpr (!RefCounted::SingleThreaded)
        {
          KRYS_MAYBE_UNUSED auto oldCount = this->_count.fetch_add(1, std::memory_order_relaxed);
          assert(oldCount > 0);
          assert(oldCount < std::numeric_limits<decltype(oldCount)>::max());
        }
        else
        {
          assert(this->_count > 0);
          assert(this->_count < std::numeric_limits<decltype(this->_count)>::max());
          ++this->_count;
        }
      }
      else if constexpr (!RefCounted::SingleThreaded)
      {
        for (intptr_t value = this->_count.load(std::memory_order_relaxed);;)
        {
          assert(value != 0);
          if (!RefCounted::IsEncodedPointer(value))
          {
            assert(value < std::numeric_limits<decltype(value)>::max());
            if (this->_count.compare_exchange_strong(value, value + 1, std::memory_order_release,
                                                     std::memory_order_relaxed))
              return;
          }
          else
          {
            auto ptr = RefCounted::DecodePtr<weak_value_type>(value);
            ptr->CallAddOwnerRef();
            return;
          }
        }
      }
      else
      {
        assert(this->_count != 0);
        if (!RefCounted::IsEncodedPointer(this->_count))
        {
          assert(this->_count < std::numeric_limits<decltype(this->_count)>::max());
          ++this->_count;
        }
        else
        {
          auto ptr = RefCounted::DecodePtr<weak_value_type>(this->_count);
          ptr->CallAddOwnerRef();
        }
      }
    }

    void SubRef() const noexcept
    {
      if constexpr (!RefCounted::ProvidesWeakReferences)
      {
        if constexpr (!RefCounted::SingleThreaded)
        {
          auto oldcount = this->_count.fetch_sub(1, std::memory_order_release);
          assert(oldcount > 0);
          if (oldcount == 1)
          {
            std::atomic_thread_fence(std::memory_order_acquire);
            this->CallDestroy();
          }
        }
        else
        {
          assert(this->_count > 0);
          if (--this->_count == 0)
            this->CallDestroy();
        }
      }
      else if constexpr (!RefCounted::SingleThreaded)
      {
        for (intptr_t value = this->_count.load(std::memory_order_relaxed);;)
        {
          assert(value != 0);
          if (!RefCounted::IsEncodedPointer(value))
          {
            if (this->_count.compare_exchange_strong(value, value - 1, std::memory_order_release,
                                                     std::memory_order_relaxed))
            {
              if (value == 1)
              {
                std::atomic_thread_fence(std::memory_order_acquire);
                this->CallDestroy();
              }
              return;
            }
          }
          else
          {
            auto ptr = RefCounted::DecodePtr<weak_value_type>(value);
            ptr->CallSubOwnerRef();
            return;
          }
        }
      }
      else
      {
        assert(this->_count != 0);
        if (!RefCounted::IsEncodedPointer(this->_count))
        {
          if (--this->_count == 0)
            this->CallDestroy();
        }
        else
        {
          auto ptr = RefCounted::DecodePtr<weak_value_type>(this->_count);
          ptr->CallSubOwnerRef();
        }
      }
    }

    template <typename X = Derived>
    requires(RefCounted::ProvidesWeakReferences)
    weak_ptr GetWeakPtr()
    {
      return weak_ptr::NoRef(
        const_cast<WeakReference<X> *>(const_cast<const RefCounted *>(this)->CallGetWeakValue()));
    }

    template <typename X = Derived>
    requires(RefCounted::ProvidesWeakReferences)
    const_weak_ptr GetWeakPtr() const
    {
      return const_weak_ptr::NoRef(this->CallGetWeakValue());
    }

  protected:
    RefCounted() noexcept = default;

    ~RefCounted() noexcept
    {
      KRYS_MAYBE_UNUSED auto validCount = [](auto val)
      {
        return val == 0 || val == 1;
      };

      if constexpr (RefCounted::ProvidesWeakReferences)
      {
        if constexpr (!RefCounted::SingleThreaded)
        {
          intptr_t value = this->_count.load(std::memory_order_relaxed);
          if (RefCounted::IsEncodedPointer(value))
          {
            auto ptr = RefCounted::DecodePtr<const weak_value_type>(value);
            assert(validCount(ptr->_strong.load(std::memory_order_relaxed)));
            ptr->CallOnOwnerDestruction();
            ptr->CallSubRef();
          }
          else
          {
            assert(validCount(value));
          }
        }
        else if (RefCounted::IsEncodedPointer(this->_count))
        {
          auto ptr = RefCounted::DecodePtr<const weak_value_type>(this->_count);
          assert(validCount(ptr->_strong));
          ptr->CallOnOwnerDestruction();
          ptr->CallSubRef();
        }
        else
        {
          assert(validCount(this->_count));
        }
      }
      else if constexpr (!RefCounted::SingleThreaded)
      {
        assert(validCount(this->_count.load(std::memory_order_relaxed)));
      }
      else
      {
        assert(validCount(this->_count));
      }
    }

    void Destroy() const noexcept
    {
      delete static_cast<const Derived *>(this);
    }

    const weak_value_type *GetWeakValue() const
    requires(RefCounted::ProvidesWeakReferences)
    {
      if constexpr (!RefCounted::SingleThreaded)
      {
        for (intptr_t value = this->_count.load(std::memory_order_acquire);;)
        {
          if (!RefCounted::IsEncodedPointer(value))
          {
            WeakReference<Derived> *ret = this->CallMakeWeakReference(value);
            uintptr_t desired = RefCounted::EncodePtr(ret);
            if (this->_count.compare_exchange_strong(value, desired, std::memory_order_release,
                                                     std::memory_order_relaxed))
              return ret;

            ret->CallDestroy();
          }
          else
          {
            auto ptr = RefCounted::DecodePtr<weak_value_type>(value);
            ptr->CallAddRef();
            return ptr;
          }
        }
      }
      else if (!RefCounted::IsEncodedPointer(this->_count))
      {
        WeakReference<Derived> *ret = this->CallMakeWeakReference(this->_count);
        this->_count = RefCounted::EncodePtr(ret);
        return ret;
      }
      else
      {
        auto ptr = RefCounted::DecodePtr<weak_value_type>(this->_count);
        ptr->CallAddRef();
        return ptr;
      }
    }

    weak_value_type *MakeWeakReference(intptr_t count) const
    {
      auto nonConstDerived = static_cast<Derived *>(const_cast<RefCounted *>(this));
      return new weak_value_type(count, nonConstDerived);
    }

  private:
    void CallAddRef() const noexcept
    {
      static_cast<const Derived *>(this)->AddRef();
    }

    void CallSubRef() const noexcept
    {
      static_cast<const Derived *>(this)->SubRef();
    }

    void CallDestroy() const noexcept
    {
      static_cast<const Derived *>(this)->Destroy();
    }

    auto CallMakeWeakReference(intptr_t count) const
    {
      if constexpr (RefCounted::ProvidesWeakReferences)
      {
        return static_cast<const Derived *>(this)->MakeWeakReference(count);
      }
    }

    auto CallGetWeakValue() const
    {
      return static_cast<const Derived *>(this)->GetWeakValue();
    }

    // Weak reference pointer decoding and encoding
    template <typename X>
    static X *DecodePtr(intptr_t count) noexcept
    {
      return (X *)(uintptr_t(count) << 1);
    }

    template <typename X>
    static intptr_t EncodePtr(X *ptr) noexcept
    {
      return (uintptr_t(ptr) >> 1) | uintptr_t(std::numeric_limits<intptr_t>::min());
    }

    static bool IsEncodedPointer(intptr_t count) noexcept
    {
      return count < 0;
    }

  private:
    mutable count_type _count = 1;
  };

  template <typename Owner>
  class WeakReference
  {
    template <typename T, RefCountedFlags Flags, typename TCount>
    friend class RefCounted;

    friend RefCountedTraits;

  public:
    using RefPtrTraits = RefCountedTraits;

    using strong_value_type = Owner;
    using strong_ptr = IntrusivePtr<strong_value_type, RefCountedTraits>;
    using const_strong_ptr = IntrusivePtr<const strong_value_type, RefCountedTraits>;

    static constexpr bool SingleThreaded = Owner::SingleThreaded;

  private:
    using count_type = conditional_t<WeakReference::SingleThreaded, intptr_t, std::atomic<intptr_t>>;

  public:
    WeakReference(const WeakReference &) noexcept = delete;
    WeakReference &operator=(const WeakReference &) noexcept = delete;
    WeakReference(WeakReference &&) noexcept = delete;
    WeakReference &operator=(WeakReference &&) noexcept = delete;

    void AddRef() const noexcept
    {
      if constexpr (!WeakReference::SingleThreaded)
      {
        KRYS_MAYBE_UNUSED auto oldCount = this->_count.fetch_add(1, std::memory_order_relaxed);
        assert(oldCount > 0);
        assert(oldCount < std::numeric_limits<decltype(oldCount)>::max());
      }
      else
      {
        assert(this->_count > 0);
        assert(this->_count < std::numeric_limits<decltype(this->_count)>::max());
        ++this->_count;
      }
    }

    void SubRef() const noexcept
    {
      if constexpr (!WeakReference::SingleThreaded)
      {
        auto oldCount = this->_count.fetch_sub(1, std::memory_order_release);
        assert(oldCount > 0);
        if (oldCount == 1)
        {
          std::atomic_thread_fence(std::memory_order_acquire);
          this->CallDestroy();
        }
      }
      else
      {
        assert(this->_count > 0);
        if (--this->_count == 0)
        {
          this->CallDestroy();
        }
      }
    }

    template <typename X = Owner>
    const_strong_ptr Lock() const noexcept
    {
      return const_strong_ptr::NoRef(this->CallLockOwner());
    }

    template <typename X = Owner>
    strong_ptr Lock() noexcept
    {
      return strong_ptr::NoRef(this->CallLockOwner());
    }

  protected:
    constexpr WeakReference(intptr_t initialStrong, Owner *owner) noexcept
        : _strong(initialStrong), _owner(owner)
    {
    }

    ~WeakReference() noexcept = default;

    void Destroy() const
    {
      delete static_cast<const derived_type<> *>(this);
    }

    void AddOwnerRef() noexcept
    {
      if constexpr (!WeakReference::SingleThreaded)
      {
        KRYS_MAYBE_UNUSED auto oldcount = this->_strong.fetch_add(1, std::memory_order_relaxed);
        assert(oldcount > 0);
        assert(oldcount < std::numeric_limits<decltype(oldcount)>::max());
      }
      else
      {
        assert(this->_strong > 0);
        assert(this->_strong < std::numeric_limits<decltype(this->_count)>::max());
        ++this->_strong;
      }
    }

    void SubOwnerRef() noexcept
    {
      if constexpr (!WeakReference::SingleThreaded)
      {
        auto oldCount = this->_strong.fetch_sub(1, std::memory_order_release);
        assert(oldCount > 0);
        if (oldCount == 1)
        {
          std::atomic_thread_fence(std::memory_order_acquire);
          auto owner = this->_owner;
          this->_owner = nullptr;
          owner->CallDestroy(); // this can cascade to deleting ourselves so must be the last thing
        }
      }
      else
      {
        assert(this->_strong > 0);
        if (--this->_strong == 0)
        {
          auto owner = this->_owner;
          this->_owner = nullptr;
          owner->CallDestroy(); // this can cascade to deleting ourselves so must be the last thing
        }
      }
    }

    strong_value_type *LockOwner() const noexcept
    {
      if constexpr (!WeakReference::SingleThreaded)
      {
        for (intptr_t value = this->_strong.load(std::memory_order_relaxed);;)
        {
          assert(value >= 0);

          if (value == 0)
          {
            return nullptr;
          }

          if (this->_strong.compare_exchange_strong(value, value + 1, std::memory_order_release,
                                                    std::memory_order_relaxed))
          {
            return this->_owner;
          }
        }
      }
      else
      {
        if (this->_strong == 0)
        {
          return nullptr;
        }
        ++this->_strong;
        return this->_owner;
      }
    }

    void OnOwnerDestruction() const noexcept
    {
    }

  private:
    template <typename X = Owner>
    using derived_type = remove_pointer_t<decltype(std::declval<X>().CallMakeWeakReference(0))>;

    void CallAddRef() const noexcept
    {
      static_cast<const derived_type<> *>(this)->AddRef();
    }

    void CallSubRef() const noexcept
    {
      static_cast<const derived_type<> *>(this)->SubRef();
    }

    void CallAddOwnerRef() noexcept
    {
      static_cast<derived_type<> *>(this)->AddOwnerRef();
    }

    void CallSubOwnerRef() noexcept
    {
      static_cast<derived_type<> *>(this)->SubOwnerRef();
    }

    void CallDestroy() const
    {
      static_cast<const derived_type<> *>(this)->Destroy();
    }

    strong_value_type *CallLockOwner() const noexcept
    {
      return static_cast<const derived_type<> *>(this)->LockOwner();
    }

    void CallOnOwnerDestruction() const noexcept
    {
      static_cast<const derived_type<> *>(this)->OnOwnerDestruction();
    }

  private:
    mutable count_type _count = 2;
    mutable count_type _strong = 0;
    Owner *_owner = nullptr;
  };

  template <typename T, RefCountedFlags Flags, typename TCount>
  class RefCountedAdapter : public RefCounted<RefCountedAdapter<T, Flags, TCount>, Flags, TCount>, public T
  {
    friend RefCounted<RefCountedAdapter, Flags, TCount>;

  public:
    template <typename... Args>
    requires(Constructible<T, Args...>)
    RefCountedAdapter(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
        : T(std::forward<Args>(args)...)
    {
    }

  protected:
    ~RefCountedAdapter() noexcept = default;
  };

  template <class T, RefCountedFlags Flags, class TCount>
  class RefCountedWrapper : public RefCounted<RefCountedWrapper<T, Flags, TCount>, Flags, TCount>
  {
    friend RefCounted<RefCountedWrapper, Flags, TCount>;

  public:
    template <typename... Args>
    requires(Constructible<T, Args...>)
    RefCountedWrapper(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
        : Wrapped(std::forward<Args>(args)...)
    {
    }

    T Wrapped;

  protected:
    ~RefCountedWrapper() noexcept = default;
  };
}
