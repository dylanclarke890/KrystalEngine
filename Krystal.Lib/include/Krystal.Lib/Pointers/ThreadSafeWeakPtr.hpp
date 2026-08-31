#pragma once

#include "Krystal.Lib/Atomics.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/TaggedPtr.hpp"
#include <mutex>

namespace Krys
{
  template <typename T, typename = NoTaggingTraits<T>>
  class ThreadSafeWeakPtr;

  template <typename>
  class RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe;

  class ThreadSafeWeakPtrControlBlock : public NonCopyable<ThreadSafeWeakPtrControlBlock>
  {
  private:
    mutable std::mutex _lock;
    mutable uint32 _strongCount {1};
    mutable uint32 _weakCount {0};
    mutable RawPtr<void> _object {nullptr};

  public:
    void AddRefStrong() const noexcept
    {
      std::scoped_lock locker {_lock};
      assert(_object);
      ++_strongCount;
    }

    template <typename T>
    void SubRefStrong() const noexcept
    {
      RawPtr<T> object;
      {
        std::scoped_lock locker {_lock};
        assert(_object);
        if (--_strongCount) KRYS_LIKELY
        {
          return;
        }
        object = static_cast<RawPtr<T>>(std::exchange(_object, nullptr));

        // We need to take a weak ref so `this` survives until the `delete object` below.
        // This comes up when destructors try to eagerly remove themselves from WeakHashSets.
        // e.g.
        // ~MyObject() { _weakSet.remove(this); }
        // if _weakSet has the last reference to the ControlBlock then we could end up doing
        // an amortized clean up, which removes the ControlBlock and destroys it. Then when we
        // check _weakSet's backing table after the cleanup we UAF the ControlBlock.
        _weakCount++;
      }

      delete static_cast<const T *>(object);

      bool hasOtherWeakRefs;
      {
        // We retained ourselves above.
        std::scoped_lock locker {_lock};
        hasOtherWeakRefs = --_weakCount;
        // release the lock here so we don't do it in Locker's destuctor after we've already called delete.
      }

      if (!hasOtherWeakRefs)
      {
        delete this;
      }
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      std::scoped_lock locker {_lock};
      return _strongCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      std::scoped_lock locker {_lock};
      return _strongCount == 1;
    }

    RawPtr<ThreadSafeWeakPtrControlBlock> AddRefWeak() noexcept
    {
      std::scoped_lock locker {_lock};
      ++_weakCount;
      return this;
    }

    void SubRefWeak() noexcept
    {
      bool shouldDeleteControlBlock {false};
      {
        std::scoped_lock locker {_lock};

        assert(_weakCount);
        if (!--_weakCount && !_strongCount)
        {
          shouldDeleteControlBlock = true;
        }
      }

      if (shouldDeleteControlBlock)
      {
        delete this;
      }
    }

    KRYS_NODISCARD uint32 GetWeakRefCount() const noexcept
    {
      std::scoped_lock locker {_lock};
      return _weakCount;
    }

    template <typename U>
    KRYS_NODISCARD RefPtr<U> TryCreateStrongReference(const RawPtr<U> maybeInteriorPointer) const noexcept
    {
      std::scoped_lock locker {_lock};
      // N.B. We don't just return _object here since a ThreadSafeWeakPtr could be calling with a pointer to
      // some interior pointer when there is multiple inheritance.
      // Consider:
      // struct Cat : public RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe<Cat>;
      // struct Dog { virtual ThreadSafeWeakPtrControlBlock& ControlBlock() const = 0; };
      // struct CatDog : public Cat, public Dog
      // {  ThreadSafeWeakPtrControlBlock& ControlBlock() const { return Cat::ControlBlock(); } };
      // If we have a ThreadSafeWeakPtr<Dog> from a CatDog then we want to return maybeInteriorPointer's Dog*
      // and not _object's CatDog* pointer.
      if (_object)
      {
        // Calling the RefPtr constructor would call AddRefStrong() and deadlock.
        ++_strongCount;
        return AdoptRefPtr<U>(const_cast<RawPtr<U>>(maybeInteriorPointer));
      }
      return nullptr;
    }

    // These should really only be used for debugging and shouldn't be used to guard any checks in production,
    // unless you really know what you're doing. This is because they're prone to time of check time of use
    // bugs. Consider: if (!ObjectHasStartedDeletion()) AddRefStrong();
    // Between ObjectHasStartedDeletion() and AddRefStrong() another thread holding the sole remaining
    // reference to the underlying object could release it's reference and start deletion.
    KRYS_NODISCARD bool ObjectHasStartedDeletion() const noexcept
    {
      std::scoped_lock locker {_lock};
      return !_object;
    }

  private:
    template <typename>
    friend class RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe;

    template <typename T>
    explicit ThreadSafeWeakPtrControlBlock(
      const RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe<T> *object) noexcept
        : _object(const_cast<RawPtr<T>>(static_cast<const RawPtr<T>>(object)))
    {
    }

    void SetStrongReferenceCountDuringInitialization(uint32 count) noexcept
    {
      _strongCount = count;
    }
  };

  struct WeakPtrThreadSafeControlBlockPolicy
  {
    using type = ThreadSafeWeakPtrControlBlock;

    KRYS_ALWAYS_INLINE constexpr static RawPtr<type> AddRef(RawPtr<type> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        return ptr->AddRefWeak();
      }

      return nullptr;
    }

    KRYS_ALWAYS_INLINE constexpr static type &AddRef(type &ref) noexcept
    {
      ref.AddRefWeak();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(type *ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefWeak();
      }
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static RawPtr<type>
      ValidateGetAccess(RawPtr<type> ptr) noexcept
    {
      return ptr;
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(RawPtr<type> ptr) noexcept
    {
      return ptr != nullptr;
    }
  };

  using ThreadSafeWeakPtrControlBlockPtr =
    IntrusivePtr<ThreadSafeWeakPtrControlBlock, RawPtrTraits<ThreadSafeWeakPtrControlBlock>,
                 WeakPtrThreadSafeControlBlockPolicy, IsNullable(true)>;

  template <typename T>
  class RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe
      : public NonCopyable<RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe>
  {
    static_assert(alignof(ThreadSafeWeakPtrControlBlock) >= 2);

    template <typename, typename>
    friend class ThreadSafeWeakPtr;

    template <typename>
    friend class ThreadSafeWeakHashSet;

  public:
    constexpr static uintptr_t StrongOnlyFlag = 1;
    constexpr static uintptr_t DestructionStartedFlag = 1ull << (sizeof(uintptr_t) * CHAR_BIT - 1);
    constexpr static uintptr_t RefIncrement = 2;

  private:
    mutable Atomic<uintptr_t> _bits {RefIncrement + StrongOnlyFlag};

  public:
    void AddRef() const noexcept
    {
      bool didRefStrongOnly = _bits.transaction(
        [&](uintptr_t &bits)
        {
          if (!IsStrongOnly(bits))
          {
            return false;
          }
          // FIXME: Add support for AddRef()/SubRef() during destruction like we support for other RefCounted
          // types.
          assert(!(bits & DestructionStartedFlag));
          bits += RefIncrement;
          return true;
        },
        std::memory_order_relaxed);

      if (didRefStrongOnly)
      {
        return;
      }

      std::bit_cast<RawPtr<ThreadSafeWeakPtrControlBlock>>(_bits.loadRelaxed())->AddRefStrong();
    }

    void SubRef() const noexcept
    {
      uintptr_t newStrongOnlyRefCount = 0;

      bool didDerefStrongOnly = _bits.transaction(
        [&](uintptr_t &bits)
        {
          if (!IsStrongOnly(bits))
          {
            return false;
          }

          // FIXME: Add support for ref()/deref() during destruction like we support for other RefCounted
          // types.
          assert(!(bits & DestructionStartedFlag));
          bits -= RefIncrement;
          newStrongOnlyRefCount = bits;
          return true;
        },
        std::memory_order_relaxed);

      if (didDerefStrongOnly)
      {
        if (newStrongOnlyRefCount == StrongOnlyFlag)
        {
          assert(_bits.exchangeOr(DestructionStartedFlag) == newStrongOnlyRefCount);
          delete static_cast<const T *>(this);
        }
        return;
      }

      std::bit_cast<RawPtr<ThreadSafeWeakPtrControlBlock>>(_bits.loadRelaxed())->template SubRefStrong<T>();
    }

    uint32 GetRefCount() const noexcept
    {
      uint32 bits = _bits.loadRelaxed();
      if (IsStrongOnly(bits))
      {
        // FIXME: Add support for AddRef()/SubRef() during destruction like we support for other RefCounted
        // types.
        assert(!(bits & DestructionStartedFlag));
        // Technically, this bit-and isn't needed but it's included for clarity since the compiler will elide
        // it anyway.
        return (bits & ~StrongOnlyFlag) / RefIncrement;
      }

      return std::bit_cast<ThreadSafeWeakPtrControlBlock *>(bits)->GetRefCount();
    }

    bool HasOneRef() const noexcept
    {
      return GetRefCount() == 1;
    }

    // Ideally this would have been private but AbstractRefCounted subclasses need to be able to access this
    // function to provide its result to ThreadSafeWeakHashSet.
    uint32 GetWeakRefCount() const noexcept
    {
      return !IsStrongOnly(_bits.loadRelaxed()) ? ControlBlock().weakRefCount() : 0;
    }

  protected:
    RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe() noexcept = default;

    ThreadSafeWeakPtrControlBlock &ControlBlock() const noexcept
    {
      // If we ever decided there was a lot of contention here we could have some lock bits in _bits but
      // that seems unlikely since this is a one-way street. Once we add a ControlBlock we don't go back
      // to strong only.
      uintptr_t bits = _bits.loadRelaxed();
      if (!IsStrongOnly(bits)) [[likely]]
        return *std::bit_cast<ThreadSafeWeakPtrControlBlock *>(bits);

      auto *controlBlock = new ThreadSafeWeakPtrControlBlock(this);

      bool didSetControlBlock = _bits.transaction(
        [&](uintptr_t &bits)
        {
          if (!IsStrongOnly(bits))
          {
            return false;
          }

          // It doesn't really make sense to create a ThreadSafeWeakPtr during destruction since the
          // ControlBlock has to view the object as dead. Otherwise a ThreadSafeWeakPtrFactory on an unrelated
          // thread could vend out a partially destroyed object.
          assert(!(bits & DestructionStartedFlag));
          // Technically, this bit-and isn't needed but it's included for clarity since the compiler will
          // elide it anyway.
          controlBlock->SetStrongReferenceCountDuringInitialization((bits & ~StrongOnlyFlag) / RefIncrement);
          bits = std::bit_cast<uintptr_t>(controlBlock);
          assert(!IsStrongOnly(bits));
          return true;
        },
        std::memory_order_release); // We want memory_order_release here to make sure other threads see the
                                    // right ref count / object.
      if (didSetControlBlock)
      {
        return *controlBlock;
      }

      delete controlBlock;
      return *std::bit_cast<ThreadSafeWeakPtrControlBlock *>(_bits.loadRelaxed());
    }

  private:
    static bool IsStrongOnly(uintptr_t bits)
    {
      return bits & StrongOnlyFlag;
    }
  };

  template <typename T, typename TaggingTraits /* = NoTaggingTraits<T> */>
  class ThreadSafeWeakPtr
  {
    template <typename>
    friend class RefCountedThreadSafeAndCanMakeWeakPtrThreadSafe;

    template <typename>
    friend class ThreadSafeWeakOrStrongPtr;

  private:
    TaggedPtr<T, TaggingTraits> _objectOfCorrectType;
    // FIXME: Use CompactRefPtrTuple to reduce sizeof(ThreadSafeWeakPtr) by storing just an offset
    // from ThreadSafeWeakPtrControlBlock::_object and don't support structs larger than 65535.
    // https://bugs.webkit.org/show_bug.cgi?id=283929
    ThreadSafeWeakPtrControlBlockPtr _controlBlock;

  public:
    using tag_type = typename TaggingTraits::tag_type;

    ThreadSafeWeakPtr() noexcept = default;

    ThreadSafeWeakPtr(std::nullptr_t) noexcept
    {
    }

    ThreadSafeWeakPtr(const ThreadSafeWeakPtr &other) noexcept
        : _objectOfCorrectType(other._objectOfCorrectType), _controlBlock(other._controlBlock)
    {
    }

    ThreadSafeWeakPtr(ThreadSafeWeakPtr &&other) noexcept
        : _objectOfCorrectType(std::exchange(other._objectOfCorrectType, nullptr)),
          _controlBlock(std::exchange(other._controlBlock, nullptr))
    {
    }

    template <typename U>
    requires(!IsPointer<U>)
    ThreadSafeWeakPtr(const U &retainedReference) noexcept
        : _objectOfCorrectType(static_cast<const T *>(&retainedReference)),
          _controlBlock(ControlBlock(retainedReference))
    {
    }

    template <typename U>
    ThreadSafeWeakPtr(const U *retainedPointer) noexcept
        : _objectOfCorrectType(static_cast<const T *>(retainedPointer)),
          _controlBlock(retainedPointer ? ControlBlock(*retainedPointer) : nullptr)
    {
    }

    template <typename U>
    ThreadSafeWeakPtr(const Ref<U> &strongReference) noexcept
        : _objectOfCorrectType(static_cast<const T *>(strongReference.ptr())),
          _controlBlock(ControlBlock(strongReference.get()))
    {
    }

    template <typename U>
    ThreadSafeWeakPtr(const RefPtr<U> &strongReference) noexcept
        : _objectOfCorrectType(static_cast<const T *>(strongReference.get())),
          _controlBlock(strongReference ? ControlBlock(*strongReference) : nullptr)
    {
    }

    ThreadSafeWeakPtr(ThreadSafeWeakPtrControlBlock &ControlBlock, const T &objectOfCorrectType) noexcept
        : _objectOfCorrectType(&objectOfCorrectType), _controlBlock(&ControlBlock)
    {
    }

    ThreadSafeWeakPtr &operator=(ThreadSafeWeakPtr &&other) noexcept
    {
      _controlBlock = std::exchange(other._controlBlock, nullptr);
      _objectOfCorrectType = std::exchange(other._objectOfCorrectType, nullptr);
      return *this;
    }

    ThreadSafeWeakPtr &operator=(const ThreadSafeWeakPtr &other) noexcept
    {
      _controlBlock = other._controlBlock;
      _objectOfCorrectType = other._objectOfCorrectType;
      return *this;
    }

    template <typename U>
    requires(!IsPointer<U>)
    ThreadSafeWeakPtr &operator=(const U &retainedReference) noexcept
    {
      _controlBlock = ControlBlock(retainedReference);
      _objectOfCorrectType = static_cast<const T *>(static_cast<const U *>(&retainedReference));
      return *this;
    }

    template <typename U>
    ThreadSafeWeakPtr &operator=(const U *retainedPointer) noexcept
    {
      _controlBlock = retainedPointer ? ControlBlock(*retainedPointer) : nullptr;
      _objectOfCorrectType = static_cast<const T *>(retainedPointer);
      return *this;
    }

    ThreadSafeWeakPtr &operator=(std::nullptr_t) noexcept
    {
      _controlBlock = nullptr;
      _objectOfCorrectType = nullptr;
      return *this;
    }

    template <typename U>
    ThreadSafeWeakPtr &operator=(const Ref<U> &strongReference) noexcept
    {
      _controlBlock = ControlBlock(strongReference);
      _objectOfCorrectType = static_cast<const T *>(strongReference.ptr());
      return *this;
    }

    template <typename U>
    ThreadSafeWeakPtr &operator=(const RefPtr<U> &strongReference) noexcept
    {
      _controlBlock = strongReference ? ControlBlock(*strongReference) : nullptr;
      _objectOfCorrectType = static_cast<const T *>(strongReference.get());
      return *this;
    }

    KRYS_NODISCARD RefPtr<T> get() const noexcept
    {
      return _controlBlock ? _controlBlock->template TryCreateStrongReference<T>(_objectOfCorrectType.ptr())
                           : nullptr;
    }

    void SetTag(tag_type Tag) noexcept
    {
      _objectOfCorrectType.SetTag(Tag);
    }

    KRYS_NODISCARD tag_type Tag() const noexcept
    {
      return _objectOfCorrectType.Tag();
    }

  private:
    template <typename U>
    requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
    RawPtr<ThreadSafeWeakPtrControlBlock> ControlBlock(const U &classOrChildClass) noexcept
    {
      return &classOrChildClass.ControlBlock();
    }
  };

  template <typename T>
  class ThreadSafeWeakOrStrongPtr
  {
  public:
    enum class Status
    {
      Strong = 0,
      Weak = 1
    };

  private:
    union
    {
      ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>> _weak {};
      RefPtr<T> _strong;
    };

  public:
    KRYS_NODISCARD Status GetStatus() const noexcept
    {
      return _weak.Tag();
    }

    KRYS_NODISCARD bool IsWeak() const noexcept
    {
      return GetStatus() == Status::Weak;
    }

    // This says nullptr is strong, which makes sense because you can always have a strong reference to
    // nullptr but could be a little non-intuitive.
    KRYS_NODISCARD bool IsStrong() const noexcept
    {
      return !IsWeak();
    }

    KRYS_NODISCARD RefPtr<T> get() const noexcept
    {
      return IsWeak() ? _weak.get() : _strong;
    }

    // NB. This function is not atomic so it's not safe to call get() while this transition is happening.
    KRYS_NODISCARD RefPtr<T> ConvertToWeak() noexcept
    {
      assert(IsStrong());
      RefPtr<T> strong = Krys::Move(_strong);
      _weak = strong;
      _weak.SetTag(Status::Weak);
      assert(IsWeak());
      return strong;
    }

    RawPtr<T> TryConvertToStrong() noexcept
    {
      assert(IsWeak());
      RefPtr<T> strong = _weak.get();
      _weak.SetTag(Status::Strong);
      _weak = nullptr;
      _strong = Krys::Move(strong);
      assert(IsStrong());
      return _strong.get();
    }

    ThreadSafeWeakOrStrongPtr &operator=(const ThreadSafeWeakOrStrongPtr &other) noexcept
    {
      ThreadSafeWeakOrStrongPtr copied(other);
      swap(copied);
      return *this;
    }

    ThreadSafeWeakOrStrongPtr &operator=(ThreadSafeWeakOrStrongPtr &&other) noexcept
    {
      ThreadSafeWeakOrStrongPtr moved(Krys::Move(other));
      swap(moved);
      return *this;
    }

    ThreadSafeWeakOrStrongPtr &operator=(std::nullptr_t) noexcept
    {
      ThreadSafeWeakOrStrongPtr zeroed;
      swap(zeroed);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(const RefPtr<U> &strongReference) noexcept
    {
      ThreadSafeWeakOrStrongPtr copied(strongReference);
      swap(copied);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(RefPtr<U> &&strongReference) noexcept
    {
      ThreadSafeWeakOrStrongPtr moved(Krys::Move(strongReference));
      swap(moved);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(const Ref<U> &strongReference) noexcept
    {
      ThreadSafeWeakOrStrongPtr copied(strongReference);
      swap(copied);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(Ref<U> &&strongReference) noexcept
    {
      ThreadSafeWeakOrStrongPtr moved(Krys::Move(strongReference));
      swap(moved);
      return *this;
    }

    ThreadSafeWeakOrStrongPtr() noexcept
    {
      assert(IsStrong());
    }

    ThreadSafeWeakOrStrongPtr(std::nullptr_t) noexcept
    {
      assert(IsStrong());
    }

    ThreadSafeWeakOrStrongPtr(const ThreadSafeWeakOrStrongPtr &other) noexcept
    {
      assert(IsStrong());
      CopyConstructFrom(other);
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const ThreadSafeWeakOrStrongPtr<U> &other) noexcept
    {
      assert(IsStrong());
      CopyConstructFrom(other);
    }

    ThreadSafeWeakOrStrongPtr(ThreadSafeWeakOrStrongPtr &&other) noexcept
    {
      assert(IsStrong());
      MoveConstructFrom(Krys::Move(other));
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(ThreadSafeWeakOrStrongPtr<U> &&other) noexcept
    {
      assert(IsStrong());
      MoveConstructFrom(Krys::Move(other));
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const Ref<U> &strongReference) noexcept
    {
      assert(IsStrong());
      _strong = strongReference;
      assert(IsStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const RefPtr<U> &strongReference) noexcept
    {
      assert(IsStrong());
      _strong = strongReference;
      assert(IsStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(Ref<U> &&strongReference) noexcept
    {
      assert(IsStrong());
      _strong = Krys::Move(strongReference);
      assert(IsStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(RefPtr<U> &&strongReference) noexcept
    {
      assert(IsStrong());
      _strong = Krys::Move(strongReference);
      assert(IsStrong());
    }

    ~ThreadSafeWeakOrStrongPtr() noexcept
    {
      if (IsStrong())
        _strong.~RefPtr<T>();
      else
        _weak.~ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>>();
    }

    template <typename U>
    void swap(ThreadSafeWeakOrStrongPtr<U> &other) noexcept
    {
      if (IsStrong())
      {
        if (other.IsStrong())
        {
          std::swap(_strong, other._strong);
          return;
        }
        auto weak = std::exchange(other._weak, ThreadSafeWeakPtr<U, EnumTaggingTraits<U, Status>> {});
        assert(other.IsStrong());
        other._strong = std::exchange(_strong, nullptr);
        _weak = Krys::Move(weak);
        assert(IsWeak());
        return;
      }

      if (other.IsWeak())
      {
        std::swap(_weak, other._weak);
        return;
      }

      auto strong = std::exchange(other._strong, nullptr);
      other._weak = std::exchange(_weak, ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>> {});
      assert(other.IsWeak());
      assert(IsStrong());
      _strong = Krys::Move(strong);
    }

  private:
    template <typename U>
    void CopyConstructFrom(const ThreadSafeWeakOrStrongPtr<U> &other) noexcept
    {
      assert(IsStrong());
      if (other.IsWeak())
      {
        _weak = other._weak;
        assert(IsWeak());
      }
      else
      {
        _strong = other._strong;
        assert(IsStrong());
      }
    }

    template <typename U>
    void MoveConstructFrom(ThreadSafeWeakOrStrongPtr<U> &&other) noexcept
    {
      assert(IsStrong());
      if (other.IsWeak())
      {
        _weak = std::exchange(other._weak, ThreadSafeWeakPtr<U, EnumTaggingTraits<U, Status>> {});
        assert(IsWeak());
        assert(other.IsStrong());
      }
      else
      {
        _strong = std::exchange(other._strong, nullptr);
        assert(IsStrong());
        assert(other.IsStrong());
      }
    }
  };
}
