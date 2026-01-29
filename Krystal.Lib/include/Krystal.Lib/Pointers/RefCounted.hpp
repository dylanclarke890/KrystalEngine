#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <atomic>
#include <cassert>
#include <limits>

namespace Krys
{
  // MARK:- RefCountedFlags
  enum class RefCountedFlags : uint8
  {
    None = 0,
    ProvideWeakReferences = 1,
    SingleThreaded = 2
  };

  template <>
  constexpr inline bool EnableEnumFlags<RefCountedFlags> = true;

  // MARK:- Forward Declarations

  template <RefCountedFlags Flags>
  using DefaultRefCountType =
    conditional_t<HasFlag(Flags, RefCountedFlags::ProvideWeakReferences), intptr_t, int>;

  template <class Derived, RefCountedFlags Flags = RefCountedFlags::None,
            class CountType = DefaultRefCountType<Flags>>
  class RefCounted;

  template <class T, RefCountedFlags Flags = RefCountedFlags::None,
            class CountType = DefaultRefCountType<Flags>>
  class RefCountedAdapter;

  template <class T, RefCountedFlags Flags = RefCountedFlags::None,
            class CountType = DefaultRefCountType<Flags>>
  class RefCountedWrapper;

  template <class Derived>
  using WeakRefCounted = RefCounted<Derived, RefCountedFlags::ProvideWeakReferences>;

  template <class Derived>
  using WeakRefCountedAdapter = RefCountedAdapter<Derived, RefCountedFlags::ProvideWeakReferences>;

  template <class Derived>
  using WeakRefCountedWrapper = RefCountedWrapper<Derived, RefCountedFlags::ProvideWeakReferences>;

  template <class Derived, class CountType = DefaultRefCountType<RefCountedFlags::SingleThreaded>>
  using SingleThreadRefCounted = RefCounted<Derived, RefCountedFlags::SingleThreaded, CountType>;

  template <class Derived, class CountType = DefaultRefCountType<RefCountedFlags::SingleThreaded>>
  using SingleThreadRefCountedAdapter = RefCountedAdapter<Derived, RefCountedFlags::SingleThreaded, CountType>;

  template <class Derived, class CountType = DefaultRefCountType<RefCountedFlags::SingleThreaded>>
  using SingleThreadRefCountedWrapper = RefCountedWrapper<Derived, RefCountedFlags::SingleThreaded, CountType>;

  template <class Derived>
  using SingleThreadWeakRefCounted =
    RefCounted<Derived, RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>;

  template <class Derived>
  using SingleThreadWeakRefCountedAdapter =
    RefCountedAdapter<Derived, RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>;

  template <class Derived>
  using SingleThreadWeakRefCountedWrapper =
    RefCountedWrapper<Derived, RefCountedFlags::ProvideWeakReferences | RefCountedFlags::SingleThreaded>;

  template <class Owner>
  class WeakReference;

  // MARK:-

  struct RefCountedTraits
  {
    template <class T>
    static void add_ref(const T *obj) noexcept
    {
      obj->call_add_ref();
    }

    template <class T>
    static void sub_ref(const T *obj) noexcept
    {
      obj->call_sub_ref();
    }
  };

  // MARK:-

  template <class Derived, RefCountedFlags Flags, class CountType>
  class RefCounted
  {
    template <class Owner>
    friend class WeakReference;
    friend RefCountedTraits;

  public:
    using RefPtrTraits = RefCountedTraits;
    using ref_counted_base = RefCounted;

    static constexpr bool provides_weak_references = HasFlag(Flags, RefCountedFlags::ProvideWeakReferences);
    static constexpr bool single_threaded = HasFlag(Flags, RefCountedFlags::SingleThreaded);

  public:
    using weak_value_type =
      conditional_t<RefCounted::provides_weak_references, WeakReference<Derived>, void>;
    using weak_ptr = conditional_t<RefCounted::provides_weak_references,
                                   IntrusivePtr<weak_value_type, RefCountedTraits>, void>;
    using const_weak_ptr = conditional_t<RefCounted::provides_weak_references,
                                         IntrusivePtr<const weak_value_type, RefCountedTraits>, void>;

  private:
    static_assert(!RefCounted::provides_weak_references
                    || (RefCounted::provides_weak_references && std::is_same_v<CountType, intptr_t>),
                  "CountType must be intptr_t (the default) when providing weak references");
    static_assert(std::is_integral_v<CountType>, "CountType must be an integral type");
    static_assert(RefCounted::single_threaded || std::atomic<CountType>::is_always_lock_free,
                  "CountType must be such that std::atomic<CountType> is alwayd lock free");

    using count_type = conditional_t<RefCounted::single_threaded, CountType, std::atomic<CountType>>;

  public:
    RefCounted(const RefCounted &) noexcept = delete;
    RefCounted &operator=(const RefCounted &) noexcept = delete;
    RefCounted(RefCounted &&) noexcept = delete;
    RefCounted &operator=(RefCounted &&) noexcept = delete;

    void add_ref() const noexcept;
    void sub_ref() const noexcept;

    template <class X = Derived,
              class = std::enable_if_t<DependentBool<RefCounted::provides_weak_references, X>>>
    weak_ptr get_weak_ptr()
    {
      return weak_ptr::noref(
        const_cast<WeakReference<X> *>(const_cast<const RefCounted *>(this)->call_get_weak_value()));
    }

    template <class X = Derived,
              class = std::enable_if_t<DependentBool<RefCounted::provides_weak_references, X>>>
    const_weak_ptr get_weak_ptr() const
    {
      return const_weak_ptr::noref(this->call_get_weak_value());
    }

  protected:
    RefCounted() noexcept = default;
    ~RefCounted() noexcept;

    void destroy() const noexcept
    {
      delete static_cast<const Derived *>(this);
    }

    const weak_value_type *get_weak_value() const;

    weak_value_type *make_weak_reference(intptr_t count) const
    {
      auto non_const_derived = static_cast<Derived *>(const_cast<RefCounted *>(this));
      return new weak_value_type(count, non_const_derived);
    }

  private:
    // CRTP access
    void call_add_ref() const noexcept
    {
      static_cast<const Derived *>(this)->add_ref();
    }
    void call_sub_ref() const noexcept
    {
      static_cast<const Derived *>(this)->sub_ref();
    }

    void call_destroy() const noexcept
    {
      static_cast<const Derived *>(this)->destroy();
    }

    auto call_make_weak_reference(intptr_t count) const
    {
      if constexpr (RefCounted::provides_weak_references)
        return static_cast<const Derived *>(this)->make_weak_reference(count);
    }

    auto call_get_weak_value() const
    {
      return static_cast<const Derived *>(this)->get_weak_value();
    }

    // Weak reference pointer decoding and encoding
    template <class X>
    static X *decode_pointer(intptr_t count) noexcept
    {
      return (X *)(uintptr_t(count) << 1);
    }

    template <class X>
    static intptr_t encode_pointer(X *ptr) noexcept
    {
      return (uintptr_t(ptr) >> 1) | uintptr_t(std::numeric_limits<intptr_t>::min());
    }

    static bool is_encoded_pointer(intptr_t count) noexcept
    {
      return count < 0;
    }

  private:
    mutable count_type m_count = 1;
  };

  template <class Owner>
  class WeakReference
  {
    template <class T, RefCountedFlags Flags, class CountType>
    friend class RefCounted;
    friend RefCountedTraits;

  public:
    using RefPtrTraits = RefCountedTraits;
    using strong_value_type = Owner;
    using strong_ptr = IntrusivePtr<strong_value_type, RefCountedTraits>;
    using const_strong_ptr = IntrusivePtr<const strong_value_type, RefCountedTraits>;

    static constexpr bool single_threaded = Owner::single_threaded;

  private:
    using count_type = std::conditional_t<WeakReference::single_threaded, intptr_t, std::atomic<intptr_t>>;

  public:
    WeakReference(const WeakReference &) noexcept = delete;
    WeakReference &operator=(const WeakReference &) noexcept = delete;
    WeakReference(WeakReference &&) noexcept = delete;
    WeakReference &operator=(WeakReference &&) noexcept = delete;

    void add_ref() const noexcept;
    void sub_ref() const noexcept;

    template <class X = Owner>
    const_strong_ptr lock() const noexcept
    {
      return const_strong_ptr::noref(this->call_lock_owner());
    }

    template <class X = Owner>
    strong_ptr lock() noexcept
    {
      return strong_ptr::noref(this->call_lock_owner());
    }

  protected:
    constexpr WeakReference(intptr_t initial_strong, Owner *owner) noexcept
        : m_strong(initial_strong), m_owner(owner)
    {
    }

    ~WeakReference() noexcept = default;

    void destroy() const
    {
      delete static_cast<const derived_type<> *>(this);
    }

    void add_owner_ref() noexcept;
    void sub_owner_ref() noexcept;

    strong_value_type *lock_owner() const noexcept;

    void on_owner_destruction() const noexcept
    {
    }

  private:
    template <class X = Owner>
    using derived_type = std::remove_pointer_t<decltype(std::declval<X>().call_make_weak_reference(0))>;

    void call_add_ref() const noexcept
    {
      static_cast<const derived_type<> *>(this)->add_ref();
    }
    void call_sub_ref() const noexcept
    {
      static_cast<const derived_type<> *>(this)->sub_ref();
    }
    void call_add_owner_ref() noexcept
    {
      static_cast<derived_type<> *>(this)->add_owner_ref();
    }
    void call_sub_owner_ref() noexcept
    {
      static_cast<derived_type<> *>(this)->sub_owner_ref();
    }
    void call_destroy() const
    {
      static_cast<const derived_type<> *>(this)->destroy();
    }
    strong_value_type *call_lock_owner() const noexcept
    {
      return static_cast<const derived_type<> *>(this)->lock_owner();
    }
    void call_on_owner_destruction() const noexcept
    {
      static_cast<const derived_type<> *>(this)->on_owner_destruction();
    }

  private:
    mutable count_type m_count = 2;
    mutable count_type m_strong = 0;
    Owner *m_owner = nullptr;
  };

  template <class T, RefCountedFlags Flags, class CountType>
  class RefCountedAdapter : public RefCounted<RefCountedAdapter<T, Flags, CountType>, Flags, CountType>,
                              public T
  {
    friend RefCounted<RefCountedAdapter<T, Flags, CountType>, Flags, CountType>;

  public:
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    RefCountedAdapter(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        : T(std::forward<Args>(args)...)
    {
    }

  protected:
    ~RefCountedAdapter() noexcept = default;
  };

  template <class T, RefCountedFlags Flags, class CountType>
  class RefCountedWrapper : public RefCounted<RefCountedWrapper<T, Flags, CountType>, Flags, CountType>
  {
    friend RefCounted<RefCountedWrapper<T, Flags, CountType>, Flags, CountType>;

  public:
    template <class... Args, class = std::enable_if_t<std::is_constructible_v<T, Args...>>>
    RefCountedWrapper(Args &&...args) noexcept(noexcept(T(std::forward<Args>(args)...)))
        : wrapped(std::forward<Args>(args)...)
    {
    }

    T wrapped;

  protected:
    ~RefCountedWrapper() noexcept = default;
  };

  // MARK:- Implementation

  template <class Owner>
  inline void WeakReference<Owner>::add_ref() const noexcept
  {
    if constexpr (!WeakReference::single_threaded)
    {
      KRYS_MAYBE_UNUSED auto oldcount = this->m_count.fetch_add(1, std::memory_order_relaxed);
      assert(oldcount > 0);
      assert(oldcount < std::numeric_limits<decltype(oldcount)>::max());
    }
    else
    {
      assert(this->m_count > 0);
      assert(this->m_count < std::numeric_limits<decltype(this->m_count)>::max());
      ++this->m_count;
    }
  }

  template <class Owner>
  inline void WeakReference<Owner>::sub_ref() const noexcept
  {
    if constexpr (!WeakReference::single_threaded)
    {
      auto oldcount = this->m_count.fetch_sub(1, std::memory_order_release);
      assert(oldcount > 0);
      if (oldcount == 1)
      {
        std::atomic_thread_fence(std::memory_order_acquire);
        this->call_destroy();
      }
    }
    else
    {
      assert(this->m_count > 0);
      if (--this->m_count == 0)
        this->call_destroy();
    }
  }

  template <class Owner>
  inline void WeakReference<Owner>::add_owner_ref() noexcept
  {
    if constexpr (!WeakReference::single_threaded)
    {
      KRYS_MAYBE_UNUSED auto oldcount = this->m_strong.fetch_add(1, std::memory_order_relaxed);
      assert(oldcount > 0);
      assert(oldcount < std::numeric_limits<decltype(oldcount)>::max());
    }
    else
    {
      assert(this->m_strong > 0);
      assert(this->m_strong < std::numeric_limits<decltype(this->m_count)>::max());
      ++this->m_strong;
    }
  }

  template <class Owner>
  inline void WeakReference<Owner>::sub_owner_ref() noexcept
  {
    if constexpr (!WeakReference::single_threaded)
    {
      auto oldcount = this->m_strong.fetch_sub(1, std::memory_order_release);
      assert(oldcount > 0);
      if (oldcount == 1)
      {
        std::atomic_thread_fence(std::memory_order_acquire);
        auto owner = this->m_owner;
        this->m_owner = nullptr;
        owner->call_destroy(); // this can cascade to deleting ourselves so must be the last thing
      }
    }
    else
    {
      assert(this->m_strong > 0);
      if (--this->m_strong == 0)
      {
        auto owner = this->m_owner;
        this->m_owner = nullptr;
        owner->call_destroy(); // this can cascade to deleting ourselves so must be the last thing
      }
    }
  }

  template <class Owner>
  inline auto WeakReference<Owner>::lock_owner() const noexcept -> strong_value_type *
  {
    if constexpr (!WeakReference::single_threaded)
    {
      for (intptr_t value = this->m_strong.load(std::memory_order_relaxed);;)
      {
        assert(value >= 0);

        if (value == 0)
          return nullptr;

        if (this->m_strong.compare_exchange_strong(value, value + 1, std::memory_order_release,
                                                   std::memory_order_relaxed))
          return this->m_owner;
      }
    }
    else
    {
      if (this->m_strong == 0)
        return nullptr;
      ++this->m_strong;
      return this->m_owner;
    }
  }

  template <class Derived, RefCountedFlags Flags, class CountType>
  inline void RefCounted<Derived, Flags, CountType>::add_ref() const noexcept
  {
    if constexpr (!RefCounted::provides_weak_references)
    {
      if constexpr (!RefCounted::single_threaded)
      {
        KRYS_MAYBE_UNUSED auto oldcount = this->m_count.fetch_add(1, std::memory_order_relaxed);
        assert(oldcount > 0);
        assert(oldcount < std::numeric_limits<decltype(oldcount)>::max());
      }
      else
      {
        assert(this->m_count > 0);
        assert(this->m_count < std::numeric_limits<decltype(this->m_count)>::max());
        ++this->m_count;
      }
    }
    else
    {
      if constexpr (!RefCounted::single_threaded)
      {
        for (intptr_t value = this->m_count.load(std::memory_order_relaxed);;)
        {
          assert(value != 0);
          if (!RefCounted::is_encoded_pointer(value))
          {
            assert(value < std::numeric_limits<decltype(value)>::max());
            if (this->m_count.compare_exchange_strong(value, value + 1, std::memory_order_release,
                                                      std::memory_order_relaxed))
              return;
          }
          else
          {
            auto ptr = RefCounted::decode_pointer<weak_value_type>(value);
            ptr->call_add_owner_ref();
            return;
          }
        }
      }
      else
      {
        assert(this->m_count != 0);
        if (!RefCounted::is_encoded_pointer(this->m_count))
        {
          assert(this->m_count < std::numeric_limits<decltype(this->m_count)>::max());
          ++this->m_count;
        }
        else
        {
          auto ptr = RefCounted::decode_pointer<weak_value_type>(this->m_count);
          ptr->call_add_owner_ref();
        }
      }
    }
  }

  template <class Derived, RefCountedFlags Flags, class CountType>
  inline void RefCounted<Derived, Flags, CountType>::sub_ref() const noexcept
  {
    if constexpr (!RefCounted::provides_weak_references)
    {
      if constexpr (!RefCounted::single_threaded)
      {
        auto oldcount = this->m_count.fetch_sub(1, std::memory_order_release);
        assert(oldcount > 0);
        if (oldcount == 1)
        {
          std::atomic_thread_fence(std::memory_order_acquire);
          this->call_destroy();
        }
      }
      else
      {
        assert(this->m_count > 0);
        if (--this->m_count == 0)
          this->call_destroy();
      }
    }
    else
    {
      if constexpr (!RefCounted::single_threaded)
      {
        for (intptr_t value = this->m_count.load(std::memory_order_relaxed);;)
        {
          assert(value != 0);
          if (!RefCounted::is_encoded_pointer(value))
          {
            if (this->m_count.compare_exchange_strong(value, value - 1, std::memory_order_release,
                                                      std::memory_order_relaxed))
            {
              if (value == 1)
              {
                std::atomic_thread_fence(std::memory_order_acquire);
                this->call_destroy();
              }
              return;
            }
          }
          else
          {
            auto ptr = RefCounted::decode_pointer<weak_value_type>(value);
            ptr->call_sub_owner_ref();
            return;
          }
        }
      }
      else
      {
        assert(this->m_count != 0);
        if (!RefCounted::is_encoded_pointer(this->m_count))
        {
          if (--this->m_count == 0)
            this->call_destroy();
        }
        else
        {
          auto ptr = RefCounted::decode_pointer<weak_value_type>(this->m_count);
          ptr->call_sub_owner_ref();
        }
      }
    }
  }

  template <class Derived, RefCountedFlags Flags, class CountType>
  inline auto RefCounted<Derived, Flags, CountType>::get_weak_value() const -> const weak_value_type *
  {
    static_assert(RefCounted::provides_weak_references, "class doesn't provide weak references");

    if constexpr (!RefCounted::single_threaded)
    {
      for (intptr_t value = this->m_count.load(std::memory_order_acquire);;)
      {
        if (!RefCounted::is_encoded_pointer(value))
        {
          WeakReference<Derived> *ret = this->call_make_weak_reference(value);
          uintptr_t desired = RefCounted::encode_pointer(ret);
          if (this->m_count.compare_exchange_strong(value, desired, std::memory_order_release,
                                                    std::memory_order_relaxed))
            return ret;

          ret->call_destroy();
        }
        else
        {
          auto ptr = RefCounted::decode_pointer<weak_value_type>(value);
          ptr->call_add_ref();
          return ptr;
        }
      }
    }
    else
    {
      if (!RefCounted::is_encoded_pointer(this->m_count))
      {
        WeakReference<Derived> *ret = this->call_make_weak_reference(this->m_count);
        this->m_count = RefCounted::encode_pointer(ret);
        return ret;
      }
      else
      {
        auto ptr = RefCounted::decode_pointer<weak_value_type>(this->m_count);
        ptr->call_add_ref();
        return ptr;
      }
    }
  }

  template <class Derived, RefCountedFlags Flags, class CountType>
  inline RefCounted<Derived, Flags, CountType>::~RefCounted() noexcept
  {
    KRYS_MAYBE_UNUSED auto valid_count = [](auto val)
    {
      return val == 0 || val == 1;
    };

    if constexpr (RefCounted::provides_weak_references)
    {
      if constexpr (!RefCounted::single_threaded)
      {
        intptr_t value = this->m_count.load(std::memory_order_relaxed);
        if (RefCounted::is_encoded_pointer(value))
        {
          auto ptr = RefCounted::decode_pointer<const weak_value_type>(value);
          assert(valid_count(ptr->m_strong.load(std::memory_order_relaxed)));
          ptr->call_on_owner_destruction();
          ptr->call_sub_ref();
        }
        else
        {
          assert(valid_count(value));
        }
      }
      else
      {
        if (RefCounted::is_encoded_pointer(this->m_count))
        {
          auto ptr = RefCounted::decode_pointer<const weak_value_type>(this->m_count);
          assert(valid_count(ptr->m_strong));
          ptr->call_on_owner_destruction();
          ptr->call_sub_ref();
        }
        else
        {
          assert(valid_count(this->m_count));
        }
      }
    }
    else
    {
      if constexpr (!RefCounted::single_threaded)
        assert(valid_count(this->m_count.load(std::memory_order_relaxed)));
      else
        assert(valid_count(this->m_count));
    }
  }
}
