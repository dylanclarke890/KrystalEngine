#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtrV1/IntrusivePtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <atomic>
#include <cassert>
#include <limits>

namespace Krys
{
  enum class RefCountedFlag : uint8
  {
    None = 0,
    ProvideWeakReferences = 1 << 0,
    SingleThreaded = 1 << 1,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::RefCountedFlag, 3u)

namespace Krys
{
  template <RefCountedFlag Flags>
  using DefaultRefCountType =
    conditional_t<HasFlag(Flags, RefCountedFlag::ProvideWeakReferences), intptr_t, int32>;

  template <typename Derived, RefCountedFlag Flags = RefCountedFlag::None,
            typename TCount = DefaultRefCountType<Flags>>
  class RefCounted;

  template <typename T, RefCountedFlag Flags = RefCountedFlag::None,
            typename TCount = DefaultRefCountType<Flags>>
  class RefCountedAdapter;

  template <typename T, RefCountedFlag Flags = RefCountedFlag::None,
            typename TCount = DefaultRefCountType<Flags>>
  class RefCountedWrapper;

  template <typename Derived>
  using WeakRefCounted = RefCounted<Derived, RefCountedFlag::ProvideWeakReferences>;

  template <typename Derived>
  using WeakRefCountedAdapter = RefCountedAdapter<Derived, RefCountedFlag::ProvideWeakReferences>;

  template <typename Derived>
  using WeakRefCountedWrapper = RefCountedWrapper<Derived, RefCountedFlag::ProvideWeakReferences>;

  template <typename Derived, typename TCount = DefaultRefCountType<RefCountedFlag::SingleThreaded>>
  using SingleThreadRefCounted = RefCounted<Derived, RefCountedFlag::SingleThreaded, TCount>;

  template <typename Derived, typename TCount = DefaultRefCountType<RefCountedFlag::SingleThreaded>>
  using SingleThreadRefCountedAdapter = RefCountedAdapter<Derived, RefCountedFlag::SingleThreaded, TCount>;

  template <typename Derived, typename TCount = DefaultRefCountType<RefCountedFlag::SingleThreaded>>
  using SingleThreadRefCountedWrapper = RefCountedWrapper<Derived, RefCountedFlag::SingleThreaded, TCount>;

  template <typename Derived>
  using SingleThreadWeakRefCounted =
    RefCounted<Derived, RefCountedFlag::ProvideWeakReferences | RefCountedFlag::SingleThreaded>;

  template <typename Derived>
  using SingleThreadWeakRefCountedAdapter =
    RefCountedAdapter<Derived, RefCountedFlag::ProvideWeakReferences | RefCountedFlag::SingleThreaded>;

  template <typename Derived>
  using SingleThreadWeakRefCountedWrapper =
    RefCountedWrapper<Derived, RefCountedFlag::ProvideWeakReferences | RefCountedFlag::SingleThreaded>;

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

  template <typename Derived, RefCountedFlag Flags, typename TCount>
  class RefCounted
  {
  public:
    static constexpr bool ProvidesWeakReferences = HasFlag(Flags, RefCountedFlag::ProvideWeakReferences);
    static constexpr bool SingleThreaded = HasFlag(Flags, RefCountedFlag::SingleThreaded);

    static_assert(!RefCounted::ProvidesWeakReferences || SameType<TCount, intptr_t>,
                  "TCount must be intptr_t (the default) when providing weak references");

    static_assert(Integral<TCount>, "TCount must be an integral type");

    static_assert(RefCounted::SingleThreaded || std::atomic<TCount>::is_always_lock_free,
                  "TCount must be such that std::atomic<TCount> is always lock free");

    using RefPtrTraits = RefCountedTraits;
    using RefCountedBase = RefCounted;

    using weak_value_type = conditional_t<RefCounted::ProvidesWeakReferences, WeakReference<Derived>, void>;

    using weak_ptr = conditional_t<RefCounted::ProvidesWeakReferences,
                                   IntrusivePtr<weak_value_type, RefCountedTraits>, void>;

    using const_weak_ptr = conditional_t<RefCounted::ProvidesWeakReferences,
                                         IntrusivePtr<const weak_value_type, RefCountedTraits>, void>;

    using count_type = conditional_t<SingleThreaded, TCount, std::atomic<TCount>>;

  private:
    mutable count_type _count = 1;

  private:
    template <typename Owner>
    friend class WeakReference;

    friend RefCountedTraits;

  public:
    RefCounted(const RefCounted &) noexcept = delete;
    RefCounted &operator=(const RefCounted &) noexcept = delete;
    RefCounted(RefCounted &&) noexcept = delete;
    RefCounted &operator=(RefCounted &&) noexcept = delete;

    void AddRef() const noexcept
    {
      if constexpr (RefCounted::ProvidesWeakReferences)
      {
        if constexpr (RefCounted::SingleThreaded)
        {
          assert(this->_count != 0);
          if (RefCounted::IsEncodedPointer(this->_count))
          {
            auto ptr = RefCounted::DecodePtr<weak_value_type>(this->_count);
            ptr->CallAddOwnerRef();
          }
          else
          {
            assert(this->_count < std::numeric_limits<decltype(this->_count)>::max());
            ++this->_count;
          }
        }
        else
        {
          for (intptr_t value = this->_count.load(std::memory_order_relaxed);;)
          {
            assert(value != 0);
            if (RefCounted::IsEncodedPointer(value))
            {
              auto ptr = RefCounted::DecodePtr<weak_value_type>(value);
              ptr->CallAddOwnerRef();
              return;
            }
            else
            {
              assert(value < std::numeric_limits<decltype(value)>::max());
              if (this->_count.compare_exchange_strong(value, value + 1, std::memory_order_release,
                                                       std::memory_order_relaxed))
              {
                return;
              }
            }
          }
        }
      }
      else
      {
        if constexpr (RefCounted::SingleThreaded)
        {
          assert(this->_count > 0 && this->_count < std::numeric_limits<decltype(this->_count)>::max());
          ++this->_count;
        }
        else
        {
          KRYS_MAYBE_UNUSED auto oldCount = this->_count.fetch_add(1, std::memory_order_relaxed);
          assert(oldCount > 0 && oldCount < std::numeric_limits<decltype(oldCount)>::max());
        }
      }
    }

    void SubRef() const noexcept
    {
      if constexpr (RefCounted::ProvidesWeakReferences)
      {
        if constexpr (RefCounted::SingleThreaded)
        {
          assert(this->_count != 0);
          if (!RefCounted::IsEncodedPointer(this->_count))
          {
            if (--this->_count == 0)
            {
              this->CallDestroy();
            }
          }
          else
          {
            auto ptr = RefCounted::DecodePtr<weak_value_type>(this->_count);
            ptr->CallSubOwnerRef();
          }
        }
        else
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
      }
      else
      {
        if constexpr (RefCounted::SingleThreaded)
        {
          assert(this->_count > 0);
          if (--this->_count == 0)
          {
            this->CallDestroy();
          }
        }
        else
        {
          auto oldCount = this->_count.fetch_sub(1, std::memory_order_release);
          assert(oldCount > 0);
          if (oldCount == 1)
          {
            std::atomic_thread_fence(std::memory_order_acquire);
            this->CallDestroy();
          }
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

    RawPtr<const weak_value_type> GetWeakValue() const
    requires(RefCounted::ProvidesWeakReferences)
    {
      if constexpr (!RefCounted::SingleThreaded)
      {
        for (intptr_t value = this->_count.load(std::memory_order_acquire);;)
        {
          if (RefCounted::IsEncodedPointer(value))
          {
            auto ptr = RefCounted::DecodePtr<weak_value_type>(value);
            ptr->CallAddRef();
            return ptr;
          }
          else
          {
            WeakReference<Derived> *ret = this->CallMakeWeakReference(value);
            uintptr_t desired = RefCounted::EncodePtr(ret);
            if (this->_count.compare_exchange_strong(value, desired, std::memory_order_release,
                                                     std::memory_order_relaxed))
            {
              return ret;
            }

            ret->CallDestroy();
          }
        }
      }
      else if (RefCounted::IsEncodedPointer(this->_count))
      {
        auto ptr = RefCounted::DecodePtr<weak_value_type>(this->_count);
        ptr->CallAddRef();
        return ptr;
      }
      else
      {
        RawPtr<WeakReference<Derived>> ret = this->CallMakeWeakReference(this->_count);
        this->_count = RefCounted::EncodePtr(ret);
        return ret;
      }
    }

    RawPtr<weak_value_type> MakeWeakReference(intptr_t count) const
    {
      auto nonConstDerived = static_cast<RawPtr<Derived>>(const_cast<RawPtr<RefCounted>>(this));
      return new weak_value_type(count, nonConstDerived);
    }

  private:
    void CallAddRef() const noexcept
    {
      static_cast<RawPtr<const Derived>>(this)->AddRef();
    }

    void CallSubRef() const noexcept
    {
      static_cast<RawPtr<const Derived>>(this)->SubRef();
    }

    void CallDestroy() const noexcept
    {
      static_cast<RawPtr<const Derived>>(this)->Destroy();
    }

    auto CallMakeWeakReference(intptr_t count) const
    {
      if constexpr (RefCounted::ProvidesWeakReferences)
      {
        return static_cast<RawPtr<const Derived>>(this)->MakeWeakReference(count);
      }
    }

    auto CallGetWeakValue() const
    {
      return static_cast<RawPtr<const Derived>>(this)->GetWeakValue();
    }

    // Weak reference pointer decoding and encoding
    template <typename X>
    static RawPtr<X> DecodePtr(intptr_t count) noexcept
    {
      return (RawPtr<X>)(uintptr_t(count) << 1);
    }

    template <typename X>
    static intptr_t EncodePtr(RawPtr<X> ptr) noexcept
    {
      return (uintptr_t(ptr) >> 1) | uintptr_t(std::numeric_limits<intptr_t>::min());
    }

    static bool IsEncodedPointer(intptr_t count) noexcept
    {
      return count < 0;
    }
  };

  template <typename Owner>
  class WeakReference
  {
    template <typename T, RefCountedFlag Flags, typename TCount>
    friend class RefCounted;

    friend RefCountedTraits;

  public:
    static constexpr bool SingleThreaded = Owner::SingleThreaded;

    using RefPtrTraits = RefCountedTraits;
    using strong_value_type = Owner;
    using strong_ptr = IntrusivePtr<strong_value_type, RefCountedTraits>;
    using const_strong_ptr = IntrusivePtr<const strong_value_type, RefCountedTraits>;
    using count_type = conditional_t<WeakReference::SingleThreaded, intptr_t, std::atomic<intptr_t>>;

  private:
    mutable count_type _count = 2;
    mutable count_type _strong = 0;
    RawPtr<Owner> _owner = nullptr;

  private:
  public:
    WeakReference(const WeakReference &) noexcept = delete;
    WeakReference &operator=(const WeakReference &) noexcept = delete;
    WeakReference(WeakReference &&) noexcept = delete;
    WeakReference &operator=(WeakReference &&) noexcept = delete;

    void AddRef() const noexcept
    {
      if constexpr (WeakReference::SingleThreaded)
      {
        assert(this->_count > 0 && this->_count < std::numeric_limits<decltype(this->_count)>::max());
        ++this->_count;
      }
      else
      {
        KRYS_MAYBE_UNUSED auto oldCount = this->_count.fetch_add(1, std::memory_order_relaxed);
        assert(oldCount > 0 && oldCount < std::numeric_limits<decltype(oldCount)>::max());
      }
    }

    void SubRef() const noexcept
    {
      if constexpr (WeakReference::SingleThreaded)
      {
        assert(this->_count > 0);
        if (--this->_count == 0)
        {
          this->CallDestroy();
        }
      }
      else
      {
        auto oldCount = this->_count.fetch_sub(1, std::memory_order_release);
        assert(oldCount > 0);
        if (oldCount == 1)
        {
          std::atomic_thread_fence(std::memory_order_acquire);
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
    constexpr WeakReference(intptr_t initialStrong, RawPtr<Owner> owner) noexcept
        : _strong(initialStrong), _owner(owner)
    {
    }

    ~WeakReference() noexcept = default;

    void Destroy() const
    {
      delete static_cast<RawPtr<const derived_type<>>>(this);
    }

    void AddOwnerRef() noexcept
    {
      if constexpr (WeakReference::SingleThreaded)
      {
        assert(this->_strong > 0 && this->_strong < std::numeric_limits<decltype(this->_count)>::max());
        ++this->_strong;
      }
      else
      {
        KRYS_MAYBE_UNUSED auto oldCount = this->_strong.fetch_add(1, std::memory_order_relaxed);
        assert(oldCount > 0 && oldCount < std::numeric_limits<decltype(oldCount)>::max());
      }
    }

    void SubOwnerRef() noexcept
    {
      if constexpr (WeakReference::SingleThreaded)
      {
        assert(this->_strong > 0);
        if (--this->_strong == 0)
        {
          auto owner = this->_owner;
          this->_owner = nullptr;
          owner->CallDestroy(); // this can cascade to deleting ourselves so must be the last thing
        }
      }
      else
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
    }

    RawPtr<strong_value_type> LockOwner() const noexcept
    {
      if constexpr (WeakReference::SingleThreaded)
      {
        if (this->_strong == 0)
        {
          return nullptr;
        }

        ++this->_strong;
        return this->_owner;
      }
      else
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
    }

    void OnOwnerDestruction() const noexcept
    {
    }

  private:
    template <typename X = Owner>
    using derived_type = remove_pointer_t<decltype(std::declval<X>().CallMakeWeakReference(0))>;

    void CallAddRef() const noexcept
    {
      static_cast<RawPtr<const derived_type<>>>(this)->AddRef();
    }

    void CallSubRef() const noexcept
    {
      static_cast<RawPtr<const derived_type<>>>(this)->SubRef();
    }

    void CallAddOwnerRef() noexcept
    {
      static_cast<RawPtr<derived_type<>>>(this)->AddOwnerRef();
    }

    void CallSubOwnerRef() noexcept
    {
      static_cast<RawPtr<derived_type<>>>(this)->SubOwnerRef();
    }

    void CallDestroy() const
    {
      static_cast<RawPtr<const derived_type<>>>(this)->Destroy();
    }

    strong_value_type *CallLockOwner() const noexcept
    {
      return static_cast<RawPtr<const derived_type<>>>(this)->LockOwner();
    }

    void CallOnOwnerDestruction() const noexcept
    {
      static_cast<RawPtr<const derived_type<>>>(this)->OnOwnerDestruction();
    }
  };

  template <typename T, RefCountedFlag Flags, typename TCount>
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

  template <typename T, RefCountedFlag Flags, typename TCount>
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
