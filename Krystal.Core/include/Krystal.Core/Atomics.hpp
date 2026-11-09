#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include <atomic>

namespace krys
{
  KRYS_ALWAYS_INLINE bool HasFence(std::memory_order order)
  {
    return order != std::memory_order_relaxed;
  }

  // Atomic wraps around std::atomic with the sole purpose of making the compare_exchange
  // operations not alter the expected value. This is more in line with how we typically
  // use CAS in our code.
  //
  // Atomic is a struct without explicitly defined constructors so that it can be
  // initialized at compile time.
  template <typename T>
  struct Atomic
  {
    // Don't pass a non-default value for the order parameter unless you really know
    // what you are doing and have thought about it very hard. The cost of seq_cst
    // is usually not high enough to justify the risk.
    KRYS_ALWAYS_INLINE T load(std::memory_order order = std::memory_order_seq_cst) const
    {
      return value.load(order);
    }

    KRYS_ALWAYS_INLINE T loadRelaxed() const
    {
      return load(std::memory_order_relaxed);
    }

    // This is a load that simultaneously does a full fence - neither loads nor stores will move
    // above or below it.
    KRYS_ALWAYS_INLINE T loadFullyFenced() const
    {
      Atomic<T> *ptr = const_cast<Atomic<T> *>(this);
      return ptr->exchangeAdd(T());
    }

    KRYS_ALWAYS_INLINE void store(T desired, std::memory_order order = std::memory_order_seq_cst)
    {
      value.store(desired, order);
    }

    KRYS_ALWAYS_INLINE void storeRelaxed(T desired)
    {
      store(desired, std::memory_order_relaxed);
    }

    // This is a store that simultaneously does a full fence - neither loads nor stores will move
    // above or below it.
    KRYS_ALWAYS_INLINE void storeFullyFenced(T desired)
    {
      exchange(desired);
    }

    KRYS_ALWAYS_INLINE bool compareExchangeWeak(T expected, T desired,
                                                std::memory_order order = std::memory_order_seq_cst)
    {
      T expectedOrActual = expected;
      return value.compare_exchange_weak(expectedOrActual, desired, order);
    }

    KRYS_ALWAYS_INLINE bool compareExchangeWeakRelaxed(T expected, T desired)
    {
      return compareExchangeWeak(expected, desired, std::memory_order_relaxed);
    }

    KRYS_ALWAYS_INLINE bool compareExchangeWeak(T expected, T desired, std::memory_order order_success,
                                                std::memory_order order_failure)
    {
      T expectedOrActual = expected;
      return value.compare_exchange_weak(expectedOrActual, desired, order_success, order_failure);
    }

    // WARNING: This does not have strong fencing guarantees when it fails. For example, stores could
    // sink below it in that case.
    KRYS_ALWAYS_INLINE T compareExchangeStrong(T expected, T desired,
                                               std::memory_order order = std::memory_order_seq_cst)
    {
      T expectedOrActual = expected;
      value.compare_exchange_strong(expectedOrActual, desired, order);
      return expectedOrActual;
    }

    KRYS_ALWAYS_INLINE T compareExchangeStrong(T expected, T desired, std::memory_order order_success,
                                               std::memory_order order_failure)
    {
      T expectedOrActual = expected;
      value.compare_exchange_strong(expectedOrActual, desired, order_success, order_failure);
      return expectedOrActual;
    }

    template <typename U>
    KRYS_ALWAYS_INLINE T exchangeAdd(U operand, std::memory_order order = std::memory_order_seq_cst)
    {
      return value.fetch_add(operand, order);
    }

    template <typename U>
    KRYS_ALWAYS_INLINE T exchangeAnd(U operand, std::memory_order order = std::memory_order_seq_cst)
    {
      return value.fetch_and(operand, order);
    }

    template <typename U>
    KRYS_ALWAYS_INLINE T exchangeOr(U operand, std::memory_order order = std::memory_order_seq_cst)
    {
      return value.fetch_or(operand, order);
    }

    template <typename U>
    KRYS_ALWAYS_INLINE T exchangeSub(U operand, std::memory_order order = std::memory_order_seq_cst)
    {
      return value.fetch_sub(operand, order);
    }

    template <typename U>
    KRYS_ALWAYS_INLINE T exchangeXor(U operand, std::memory_order order = std::memory_order_seq_cst)
    {
      return value.fetch_xor(operand, order);
    }

    KRYS_ALWAYS_INLINE T exchange(T newValue, std::memory_order order = std::memory_order_seq_cst)
    {
      return value.exchange(newValue, order);
    }

    // func is supposed to return false if the value is already in the desired state.
    // Returns true if the value was changed. Else returns false.
    KRYS_ALWAYS_INLINE bool transaction(const Callable<T &> auto &func,
                                        std::memory_order order = std::memory_order_seq_cst)
    {
      for (;;)
      {
        T oldValue = load(std::memory_order_relaxed);
        T newValue = oldValue;
        if (!func(newValue))
          return false;
        if (compareExchangeWeak(oldValue, newValue, order))
          return true;
      }
    }

    // func is supposed to return false if the value is already in the desired state.
    // Returns true if the value was changed. Else returns false.
    template <typename Func>
    KRYS_ALWAYS_INLINE bool transactionRelaxed(const Func &func)
    {
      return transaction(func, std::memory_order_relaxed);
    }

    Atomic() = default;
    constexpr Atomic(T initial) : value(std::forward<T>(initial))
    {
    }

    std::atomic<T> value;
  };
}
