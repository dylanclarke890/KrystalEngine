#pragma once

#include "Krystal.Lib/Atomics.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"
#include "Krystal.Lib/Pointers/TaggedPtr.hpp"
#include <mutex>

namespace Krys
{
  template <typename T, typename = NoTaggingTraits<T>>
  class ThreadSafeWeakPtr;

  template <typename>
  class ThreadSafeWeakHashSet;

  template <typename>
  class ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr;

  class ThreadSafeWeakPtrControlBlock : public NonCopyable<ThreadSafeWeakPtrControlBlock>
  {
  private:
    mutable std::mutex _lock;
    mutable uint32 _strongCount {1};
    mutable uint32 _weakCount {0};
    mutable void *_object {nullptr};

  public:
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
        object = static_cast<T *>(std::exchange(_object, nullptr));

        // We need to take a weak ref so `this` survives until the `delete object` below.
        // This comes up when destructors try to eagerly remove themselves from WeakHashSets.
        // e.g.
        // ~MyObject() { m_weakSet.remove(this); }
        // if m_weakSet has the last reference to the ControlBlock then we could end up doing
        // an amortized clean up, which removes the ControlBlock and destroys it. Then when we
        // check m_weakSet's backing table after the cleanup we UAF the ControlBlock.
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

    template <typename U>
    KRYS_NODISCARD RefPtr<U> TryCreateStrongReference(const RawPtr<U> maybeInteriorPointer) const noexcept
    {
      std::scoped_lock locker {_lock};
      // N.B. We don't just return _object here since a ThreadSafeWeakPtr could be calling with a pointer to
      // some interior pointer when there is multiple inheritance.
      // Consider:
      // struct Cat : public ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<Cat>;
      // struct Dog { virtual ThreadSafeWeakPtrControlBlock& ControlBlock() const = 0; };
      // struct CatDog : public Cat, public Dog
      // {  ThreadSafeWeakPtrControlBlock& ControlBlock() const { return Cat::ControlBlock(); } };
      // If we have a ThreadSafeWeakPtr<Dog> from a CatDog then we want to return maybeInteriorPointer's Dog*
      // and not _object's CatDog* pointer.
      if (_object)
      {
        // Calling the RefPtr constructor would call AddRefStrong() and deadlock.
        ++_strongCount;
        return AdoptRef(const_cast<RawPtr<U>>(maybeInteriorPointer));
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

    KRYS_NODISCARD uint32 GetWeakRefCount() const noexcept
    {
      std::scoped_lock locker {_lock};
      return _weakCount;
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

  private:
    template <typename>
    friend class ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr;

    template <typename T>
    explicit ThreadSafeWeakPtrControlBlock(
      const ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<T> *object) noexcept
        : _object(const_cast<RawPtr<T>>(static_cast<const RawPtr<T>>(object)))
    {
    }

    void SetStrongReferenceCountDuringInitialization(uint32 count) noexcept
    {
      _strongCount = count;
    }
  };

  struct ThreadSafeWeakPtrControlBlockRefDerefTraits
  {
    static KRYS_ALWAYS_INLINE RawPtr<ThreadSafeWeakPtrControlBlock>
      AddRef(RawPtr<ThreadSafeWeakPtrControlBlock> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        return ptr->AddRefWeak();
      }
      return nullptr;
    }

    static KRYS_ALWAYS_INLINE void SubRef(ThreadSafeWeakPtrControlBlock *ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefWeak();
      }
    }
  };

  using ControlBlockRefPtr =
    RefPtr<ThreadSafeWeakPtrControlBlock, RawPtrTraits<ThreadSafeWeakPtrControlBlock>,
           ThreadSafeWeakPtrControlBlockRefDerefTraits>;

  template <typename T>
  class ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr
      : public NonCopyable<ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr>
  {
  public:
    static_assert(alignof(ThreadSafeWeakPtrControlBlock) >= 2);
    static constexpr uintptr_t strongOnlyFlag = 1;
    static constexpr uintptr_t destructionStartedFlag = 1ull << (sizeof(uintptr_t) * CHAR_BIT - 1);
    static constexpr uintptr_t refIncrement = 2;

    void AddRef() const noexcept
    {
      bool didRefStrongOnly = m_bits.transaction(
        [&](uintptr_t &bits)
        {
          if (!isStrongOnly(bits))
            return false;
          // FIXME: Add support for ref()/deref() during destruction like we support for other RefCounted
          // types.
          assert(!(bits & destructionStartedFlag));
          bits += refIncrement;
          return true;
        },
        std::memory_order_relaxed);
      if (didRefStrongOnly)
        return;

      std::bit_cast<ThreadSafeWeakPtrControlBlock *>(m_bits.loadRelaxed())->AddRefStrong();
    }

    void SubRef() const noexcept
    {
      uintptr_t newStrongOnlyRefCount = 0;
      bool didDerefStrongOnly = m_bits.transaction(
        [&](uintptr_t &bits)
        {
          if (!isStrongOnly(bits))
            return false;
          // FIXME: Add support for ref()/deref() during destruction like we support for other RefCounted
          // types.
          assert(!(bits & destructionStartedFlag));
          bits -= refIncrement;
          newStrongOnlyRefCount = bits;
          return true;
        },
        std::memory_order_relaxed);
      if (didDerefStrongOnly)
      {
        if (newStrongOnlyRefCount == strongOnlyFlag)
        {
          assert(m_bits.exchangeOr(destructionStartedFlag) == newStrongOnlyRefCount);
          delete static_cast<const T *>(this);
        }
        return;
      }

      std::bit_cast<ThreadSafeWeakPtrControlBlock *>(m_bits.loadRelaxed())
        ->template SubRefStrong<T, destructionThread>();
    }

    uint32 GetRefCount() const noexcept
    {
      uint32 bits = m_bits.loadRelaxed();
      if (isStrongOnly(bits))
      {
        // FIXME: Add support for ref()/deref() during destruction like we support for other RefCounted types.
        assert(!(bits & destructionStartedFlag));
        // Technically, this bit-and isn't needed but it's included for clarity since the compiler will elide
        // it anyway.
        return (bits & ~strongOnlyFlag) / refIncrement;
      }

      return std::bit_cast<ThreadSafeWeakPtrControlBlock *>(bits)->GetRefCount();
    }

    bool hasOneRef() const noexcept
    {
      return GetRefCount() == 1;
    }

    // Ideally this would have been private but AbstractRefCounted subclasses need to be able to access this
    // function to provide its result to ThreadSafeWeakHashSet.
    uint32_t weakRefCount() const noexcept
    {
      return !isStrongOnly(m_bits.loadRelaxed()) ? controlBlock().weakRefCount() : 0;
    }

  protected:
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr() noexcept = default;
    ThreadSafeWeakPtrControlBlock &controlBlock() const noexcept
    {
      // If we ever decided there was a lot of contention here we could have some lock bits in m_bits but
      // that seems unlikely since this is a one-way street. Once we add a controlBlock we don't go back
      // to strong only.
      uintptr_t bits = m_bits.loadRelaxed();
      if (!isStrongOnly(bits)) [[likely]]
        return *std::bit_cast<ThreadSafeWeakPtrControlBlock *>(bits);

      auto *controlBlock = new ThreadSafeWeakPtrControlBlock(this);

      bool didSetControlBlock = m_bits.transaction(
        [&](uintptr_t &bits)
        {
          if (!isStrongOnly(bits))
            return false;

          // It doesn't really make sense to create a ThreadSafeWeakPtr during destruction since the
          // controlBlock has to view the object as dead. Otherwise a ThreadSafeWeakPtrFactory on an unrelated
          // thread could vend out a partially destroyed object.
          assert(!(bits & destructionStartedFlag));
          // Technically, this bit-and isn't needed but it's included for clarity since the compiler will
          // elide it anyway.
          controlBlock->SetStrongReferenceCountDuringInitialization((bits & ~strongOnlyFlag) / refIncrement);
          bits = std::bit_cast<uintptr_t>(controlBlock);
          assert(!isStrongOnly(bits));
          return true;
        },
        std::memory_order_release); // We want memory_order_release here to make sure other threads see the
                                    // right ref count / object.
      if (didSetControlBlock)
        return *controlBlock;

      delete controlBlock;
      return *std::bit_cast<ThreadSafeWeakPtrControlBlock *>(m_bits.loadRelaxed());
    }

  private:
    static bool isStrongOnly(uintptr_t bits)
    {
      return bits & strongOnlyFlag;
    }
    template <typename, typename>
    friend class ThreadSafeWeakPtr;
    template <typename>
    friend class ThreadSafeWeakHashSet;

    mutable Atomic<uintptr_t> m_bits {refIncrement + strongOnlyFlag};
  };

  template <typename T, typename TaggingTraits /* = NoTaggingTraits<T> */>
  class ThreadSafeWeakPtr
  {
  public:
    using tag_type = typename TaggingTraits::tag_type;
    ThreadSafeWeakPtr() = default;

    ThreadSafeWeakPtr(std::nullptr_t)
    {
    }

    ThreadSafeWeakPtr(const ThreadSafeWeakPtr &other)
        : m_objectOfCorrectType(other.m_objectOfCorrectType), m_controlBlock(other.m_controlBlock)
    {
    }

    ThreadSafeWeakPtr(ThreadSafeWeakPtr &&other)
        : m_objectOfCorrectType(std::exchange(other.m_objectOfCorrectType, nullptr)),
          m_controlBlock(std::exchange(other.m_controlBlock, nullptr))
    {
    }

    template <typename U>
    requires(!std::is_pointer_v<U>)
    ThreadSafeWeakPtr(const U &retainedReference)
        : m_objectOfCorrectType(static_cast<const T *>(&retainedReference)),
          m_controlBlock(controlBlock(retainedReference))
    {
    }

    template <typename U>
    ThreadSafeWeakPtr(const U *retainedPointer)
        : m_objectOfCorrectType(static_cast<const T *>(retainedPointer)),
          m_controlBlock(retainedPointer ? controlBlock(*retainedPointer) : nullptr)
    {
    }

    template <typename U>
    ThreadSafeWeakPtr(const Ref<U> &strongReference)
        : m_objectOfCorrectType(static_cast<const T *>(strongReference.ptr())),
          m_controlBlock(controlBlock(strongReference.get()))
    {
    }

    template <typename U>
    ThreadSafeWeakPtr(const RefPtr<U> &strongReference)
        : m_objectOfCorrectType(static_cast<const T *>(strongReference.get())),
          m_controlBlock(strongReference ? controlBlock(*strongReference) : nullptr)
    {
    }

    ThreadSafeWeakPtr(ThreadSafeWeakPtrControlBlock &controlBlock, const T &objectOfCorrectType)
        : m_objectOfCorrectType(&objectOfCorrectType), m_controlBlock(&controlBlock)
    {
    }

    ThreadSafeWeakPtr &operator=(ThreadSafeWeakPtr &&other)
    {
      m_controlBlock = std::exchange(other.m_controlBlock, nullptr);
      m_objectOfCorrectType = std::exchange(other.m_objectOfCorrectType, nullptr);
      return *this;
    }

    ThreadSafeWeakPtr &operator=(const ThreadSafeWeakPtr &other)
    {
      m_controlBlock = other.m_controlBlock;
      m_objectOfCorrectType = other.m_objectOfCorrectType;
      return *this;
    }

    template <typename U>
    requires(!std::is_pointer_v<U>)
    ThreadSafeWeakPtr &operator=(const U &retainedReference)
    {
      m_controlBlock = controlBlock(retainedReference);
      m_objectOfCorrectType = static_cast<const T *>(static_cast<const U *>(&retainedReference));
      return *this;
    }

    template <typename U>
    ThreadSafeWeakPtr &operator=(const U *retainedPointer)
    {
      m_controlBlock = retainedPointer ? controlBlock(*retainedPointer) : nullptr;
      m_objectOfCorrectType = static_cast<const T *>(retainedPointer);
      return *this;
    }

    ThreadSafeWeakPtr &operator=(std::nullptr_t)
    {
      m_controlBlock = nullptr;
      m_objectOfCorrectType = nullptr;
      return *this;
    }

    template <typename U>
    ThreadSafeWeakPtr &operator=(const Ref<U> &strongReference)
    {
      m_controlBlock = controlBlock(strongReference);
      m_objectOfCorrectType = static_cast<const T *>(strongReference.ptr());
      return *this;
    }

    template <typename U>
    ThreadSafeWeakPtr &operator=(const RefPtr<U> &strongReference)
    {
      m_controlBlock = strongReference ? controlBlock(*strongReference) : nullptr;
      m_objectOfCorrectType = static_cast<const T *>(strongReference.get());
      return *this;
    }

    RefPtr<T> get() const
    {
      return m_controlBlock
               ? m_controlBlock->template TryCreateStrongReference<T>(m_objectOfCorrectType.ptr())
               : nullptr;
    }

    void setTag(tag_type tag)
    {
      m_objectOfCorrectType.setTag(tag);
    }
    tag_type tag() const
    {
      return m_objectOfCorrectType.tag();
    }

  private:
    template <typename U>
    requires(std::is_convertible_v<U *, T *>)
    ThreadSafeWeakPtrControlBlock *controlBlock(const U &classOrChildClass)
    {
      return &classOrChildClass.controlBlock();
    }

    template <typename, DestructionThread>
    friend class ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr;
    template <typename>
    friend class ThreadSafeWeakHashSet;
    template <typename>
    friend class ThreadSafeWeakOrStrongPtr;

    TaggedPtr<T, TaggingTraits> m_objectOfCorrectType;
    // FIXME: Use CompactRefPtrTuple to reduce sizeof(ThreadSafeWeakPtr) by storing just an offset
    // from ThreadSafeWeakPtrControlBlock::_object and don't support structs larger than 65535.
    // https://bugs.webkit.org/show_bug.cgi?id=283929
    ControlBlockRefPtr m_controlBlock;
  };

  template <class T>
  ThreadSafeWeakPtr(const T &) -> ThreadSafeWeakPtr<T>;
  template <class T>
  ThreadSafeWeakPtr(const T *) -> ThreadSafeWeakPtr<T>;

  template <typename T>
  class ThreadSafeWeakOrStrongPtr
  {
  public:
    enum class Status
    {
      Strong = 0,
      Weak = 1
    };

    Status status() const
    {
      return _weak.tag();
    }
    bool isWeak() const
    {
      return status() == Status::Weak;
    }
    // This says nullptr is strong, which makes sense because you can always have a strong reference to
    // nullptr but could be a little non-intuitive.
    bool isStrong() const
    {
      return !isWeak();
    }

    RefPtr<T> get() const
    {
      return isWeak() ? _weak.get() : _strong;
    }

    // NB. This function is not atomic so it's not safe to call get() while this transition is happening.
    RefPtr<T> convertToWeak()
    {
      assert(isStrong());
      RefPtr<T> strong = Krys::Move(_strong);
      _weak = strong;
      _weak.setTag(Status::Weak);
      assert(isWeak());
      return strong;
    }

    T *tryConvertToStrong()
    {
      assert(isWeak());
      RefPtr<T> strong = _weak.get();
      _weak.setTag(Status::Strong);
      _weak = nullptr;
      _strong = Krys::Move(strong);
      assert(isStrong());
      return _strong.get();
    }

    ThreadSafeWeakOrStrongPtr &operator=(const ThreadSafeWeakOrStrongPtr &other)
    {
      ThreadSafeWeakOrStrongPtr copied(other);
      swap(copied);
      return *this;
    }

    ThreadSafeWeakOrStrongPtr &operator=(ThreadSafeWeakOrStrongPtr &&other)
    {
      ThreadSafeWeakOrStrongPtr moved(Krys::Move(other));
      swap(moved);
      return *this;
    }

    ThreadSafeWeakOrStrongPtr &operator=(std::nullptr_t)
    {
      ThreadSafeWeakOrStrongPtr zeroed;
      swap(zeroed);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(const RefPtr<U> &strongReference)
    {
      ThreadSafeWeakOrStrongPtr copied(strongReference);
      swap(copied);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(RefPtr<U> &&strongReference)
    {
      ThreadSafeWeakOrStrongPtr moved(Krys::Move(strongReference));
      swap(moved);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(const Ref<U> &strongReference)
    {
      ThreadSafeWeakOrStrongPtr copied(strongReference);
      swap(copied);
      return *this;
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr &operator=(Ref<U> &&strongReference)
    {
      ThreadSafeWeakOrStrongPtr moved(Krys::Move(strongReference));
      swap(moved);
      return *this;
    }

    ThreadSafeWeakOrStrongPtr()
    {
      assert(isStrong());
    }

    ThreadSafeWeakOrStrongPtr(std::nullptr_t)
    {
      assert(isStrong());
    }

    ThreadSafeWeakOrStrongPtr(const ThreadSafeWeakOrStrongPtr &other)
    {
      assert(isStrong());
      copyConstructFrom(other);
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const ThreadSafeWeakOrStrongPtr<U> &other)
    {
      assert(isStrong());
      copyConstructFrom(other);
    }

    ThreadSafeWeakOrStrongPtr(ThreadSafeWeakOrStrongPtr &&other)
    {
      assert(isStrong());
      moveConstructFrom(Krys::Move(other));
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(ThreadSafeWeakOrStrongPtr<U> &&other)
    {
      assert(isStrong());
      moveConstructFrom(Krys::Move(other));
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const Ref<U> &strongReference)
    {
      assert(isStrong());
      _strong = strongReference;
      assert(isStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const RefPtr<U> &strongReference)
    {
      assert(isStrong());
      _strong = strongReference;
      assert(isStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(Ref<U> &&strongReference)
    {
      assert(isStrong());
      _strong = Krys::Move(strongReference);
      assert(isStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(RefPtr<U> &&strongReference)
    {
      assert(isStrong());
      _strong = Krys::Move(strongReference);
      assert(isStrong());
    }

    ~ThreadSafeWeakOrStrongPtr()
    {
      if (isStrong())
        _strong.~RefPtr<T>();
      else
        _weak.~ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>>();
    }

    template <typename U>
    void swap(ThreadSafeWeakOrStrongPtr<U> &other)
    {
      if (isStrong())
      {
        if (other.isStrong())
        {
          std::swap(_strong, other._strong);
          return;
        }
        auto weak = std::exchange(other._weak, ThreadSafeWeakPtr<U, EnumTaggingTraits<U, Status>> {});
        assert(other.isStrong());
        other._strong = std::exchange(_strong, nullptr);
        _weak = Krys::Move(weak);
        assert(isWeak());
        return;
      }

      if (other.isWeak())
      {
        std::swap(_weak, other._weak);
        return;
      }

      auto strong = std::exchange(other._strong, nullptr);
      other._weak = std::exchange(_weak, ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>> {});
      assert(other.isWeak());
      assert(isStrong());
      _strong = Krys::Move(strong);
    }

  private:
    template <typename U>
    void copyConstructFrom(const ThreadSafeWeakOrStrongPtr<U> &other)
    {
      assert(isStrong());
      if (other.isWeak())
      {
        _weak = other._weak;
        assert(isWeak());
      }
      else
      {
        _strong = other._strong;
        assert(isStrong());
      }
    }

    template <typename U>
    void moveConstructFrom(ThreadSafeWeakOrStrongPtr<U> &&other)
    {
      assert(isStrong());
      if (other.isWeak())
      {
        _weak = std::exchange(other._weak, ThreadSafeWeakPtr<U, EnumTaggingTraits<U, Status>> {});
        assert(isWeak());
        assert(other.isStrong());
      }
      else
      {
        _strong = std::exchange(other._strong, nullptr);
        assert(isStrong());
        assert(other.isStrong());
      }
    }

    union
    {
      ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>> _weak {};
      RefPtr<T> _strong;
    };
  };
}
