#pragma once

#include "Krystal.Lib/Atomics.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
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
  public:
    ThreadSafeWeakPtrControlBlock *weakRef()
    {
      std::scoped_lock locker {m_lock};
      ++m_weakReferenceCount;
      return this;
    }

    void weakDeref()
    {
      bool shouldDeleteControlBlock {false};
      {
        std::scoped_lock locker {m_lock};
        assert(m_weakReferenceCount);
        if (!--m_weakReferenceCount && !m_strongReferenceCount)
        {
          shouldDeleteControlBlock = true;
        }
      }

      if (shouldDeleteControlBlock)
      {
        delete this;
      }
    }

    void strongRef() const
    {
      std::scoped_lock locker {m_lock};
      assert(m_object);
      ++m_strongReferenceCount;
    }

    template <typename T>
    void strongDeref() const
    {
      T *object;
      {
        std::scoped_lock locker {m_lock};
        assert(m_object);
        if (--m_strongReferenceCount) [[likely]]
        {
          return;
        }
        object = static_cast<T *>(std::exchange(m_object, nullptr));
        // We need to take a weak ref so `this` survives until the `delete object` below.
        // This comes up when destructors try to eagerly remove themselves from WeakHashSets.
        // e.g.
        // ~MyObject() { m_weakSet.remove(this); }
        // if m_weakSet has the last reference to the ControlBlock then we could end up doing
        // an amortized clean up, which removes the ControlBlock and destroys it. Then when we
        // check m_weakSet's backing table after the cleanup we UAF the ControlBlock.
        m_weakReferenceCount++;
      }

      delete static_cast<const T *>(object);

      bool hasOtherWeakRefs;
      {
        // We retained ourselves above.
        std::scoped_lock locker {m_lock};
        hasOtherWeakRefs = --m_weakReferenceCount;
        // release the lock here so we don't do it in Locker's destuctor after we've already called delete.
      }

      if (!hasOtherWeakRefs)
      {
        delete this;
      }
    }

    template <typename U>
    RefPtr<U> makeStrongReferenceIfPossible(const U *maybeInteriorPointer) const
    {
      std::scoped_lock locker {m_lock};
      // N.B. We don't just return m_object here since a ThreadSafeWeakPtr could be calling with a pointer to
      // some interior pointer when there is multiple inheritance.
      // Consider:
      // struct Cat : public ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<Cat>;
      // struct Dog { virtual ThreadSafeWeakPtrControlBlock& controlBlock() const = 0; };
      // struct CatDog : public Cat, public Dog {
      //     ThreadSafeWeakPtrControlBlock& controlBlock() const { return Cat::controlBlock(); }
      // };
      //
      // If we have a ThreadSafeWeakPtr<Dog> from a CatDog then we want to return maybeInteriorPointer's Dog*
      // and not m_object's CatDog* pointer.
      if (m_object)
      {
        // Calling the RefPtr constructor would call strongRef() and deadlock.
        ++m_strongReferenceCount;
        return adoptRef(const_cast<U *>(maybeInteriorPointer));
      }
      return nullptr;
    }

    // These should really only be used for debugging and shouldn't be used to guard any checks in production,
    // unless you really know what you're doing. This is because they're prone to time of check time of use
    // bugs. Consider: if (!objectHasStartedDeletion())
    //     strongRef();
    // Between objectHasStartedDeletion() and strongRef() another thread holding the sole remaining reference
    // to the underlying object could release it's reference and start deletion.
    bool objectHasStartedDeletion() const
    {
      std::scoped_lock locker {m_lock};
      return !m_object;
    }
    uint32_t weakRefCount() const
    {
      std::scoped_lock locker {m_lock};
      return m_weakReferenceCount;
    }

    uint32_t refCount() const
    {
      std::scoped_lock locker {m_lock};
      return m_strongReferenceCount;
    }

    bool hasOneRef() const
    {
      std::scoped_lock locker {m_lock};
      return m_strongReferenceCount == 1;
    }

  private:
    template <typename>
    friend class ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr;
    template <typename T>
    explicit ThreadSafeWeakPtrControlBlock(const ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr<T> *object)
        : m_object(const_cast<T *>(static_cast<const T *>(object)))
    {
    }

    void setStrongReferenceCountDuringInitialization(uint32_t count)
    {
      m_strongReferenceCount = count;
    }

    mutable std::mutex m_lock;
    mutable uint32_t m_strongReferenceCount {1};
    mutable uint32_t m_weakReferenceCount {0};
    mutable void *m_object {nullptr};
  };

  struct ThreadSafeWeakPtrControlBlockRefDerefTraits
  {
    static KRYS_ALWAYS_INLINE ThreadSafeWeakPtrControlBlock *refIfNotNull(ThreadSafeWeakPtrControlBlock *ptr)
    {
      if (ptr) [[likely]]
        return ptr->weakRef();
      return nullptr;
    }

    static KRYS_ALWAYS_INLINE void derefIfNotNull(ThreadSafeWeakPtrControlBlock *ptr)
    {
      if (ptr) [[likely]]
        ptr->weakDeref();
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

    void AddRef() const
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

      std::bit_cast<ThreadSafeWeakPtrControlBlock *>(m_bits.loadRelaxed())->strongRef();
    }

    void SubRef() const
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
        ->template strongDeref<T, destructionThread>();
    }

    uint32 refCount() const
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

      return std::bit_cast<ThreadSafeWeakPtrControlBlock *>(bits)->refCount();
    }

    bool hasOneRef() const
    {
      return refCount() == 1;
    }

    // Ideally this would have been private but AbstractRefCounted subclasses need to be able to access this
    // function to provide its result to ThreadSafeWeakHashSet.
    uint32_t weakRefCount() const
    {
      return !isStrongOnly(m_bits.loadRelaxed()) ? controlBlock().weakRefCount() : 0;
    }

  protected:
    ThreadSafeRefCountedAndCanMakeThreadSafeWeakPtr() = default;
    ThreadSafeWeakPtrControlBlock &controlBlock() const
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
          controlBlock->setStrongReferenceCountDuringInitialization((bits & ~strongOnlyFlag) / refIncrement);
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
               ? m_controlBlock->template makeStrongReferenceIfPossible<T>(m_objectOfCorrectType.ptr())
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
    // from ThreadSafeWeakPtrControlBlock::m_object and don't support structs larger than 65535.
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
      return m_weak.tag();
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
      return isWeak() ? m_weak.get() : m_strong;
    }

    // NB. This function is not atomic so it's not safe to call get() while this transition is happening.
    RefPtr<T> convertToWeak()
    {
      assert(isStrong());
      RefPtr<T> strong = Krys::Move(m_strong);
      m_weak = strong;
      m_weak.setTag(Status::Weak);
      assert(isWeak());
      return strong;
    }

    T *tryConvertToStrong()
    {
      assert(isWeak());
      RefPtr<T> strong = m_weak.get();
      m_weak.setTag(Status::Strong);
      m_weak = nullptr;
      m_strong = Krys::Move(strong);
      assert(isStrong());
      return m_strong.get();
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
      m_strong = strongReference;
      assert(isStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(const RefPtr<U> &strongReference)
    {
      assert(isStrong());
      m_strong = strongReference;
      assert(isStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(Ref<U> &&strongReference)
    {
      assert(isStrong());
      m_strong = Krys::Move(strongReference);
      assert(isStrong());
    }

    template <typename U>
    ThreadSafeWeakOrStrongPtr(RefPtr<U> &&strongReference)
    {
      assert(isStrong());
      m_strong = Krys::Move(strongReference);
      assert(isStrong());
    }

    ~ThreadSafeWeakOrStrongPtr()
    {
      if (isStrong())
        m_strong.~RefPtr<T>();
      else
        m_weak.~ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>>();
    }

    template <typename U>
    void swap(ThreadSafeWeakOrStrongPtr<U> &other)
    {
      if (isStrong())
      {
        if (other.isStrong())
        {
          std::swap(m_strong, other.m_strong);
          return;
        }
        auto weak = std::exchange(other.m_weak, ThreadSafeWeakPtr<U, EnumTaggingTraits<U, Status>> {});
        assert(other.isStrong());
        other.m_strong = std::exchange(m_strong, nullptr);
        m_weak = Krys::Move(weak);
        assert(isWeak());
        return;
      }

      if (other.isWeak())
      {
        std::swap(m_weak, other.m_weak);
        return;
      }

      auto strong = std::exchange(other.m_strong, nullptr);
      other.m_weak = std::exchange(m_weak, ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>> {});
      assert(other.isWeak());
      assert(isStrong());
      m_strong = Krys::Move(strong);
    }

  private:
    template <typename U>
    void copyConstructFrom(const ThreadSafeWeakOrStrongPtr<U> &other)
    {
      assert(isStrong());
      if (other.isWeak())
      {
        m_weak = other.m_weak;
        assert(isWeak());
      }
      else
      {
        m_strong = other.m_strong;
        assert(isStrong());
      }
    }

    template <typename U>
    void moveConstructFrom(ThreadSafeWeakOrStrongPtr<U> &&other)
    {
      assert(isStrong());
      if (other.isWeak())
      {
        m_weak = std::exchange(other.m_weak, ThreadSafeWeakPtr<U, EnumTaggingTraits<U, Status>> {});
        assert(isWeak());
        assert(other.isStrong());
      }
      else
      {
        m_strong = std::exchange(other.m_strong, nullptr);
        assert(isStrong());
        assert(other.isStrong());
      }
    }

    union
    {
      ThreadSafeWeakPtr<T, EnumTaggingTraits<T, Status>> m_weak {};
      RefPtr<T> m_strong;
    };
  };
}
