#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactPointerTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"
#include <memory>

namespace Krys
{
  template <typename T, typename Type, typename Deleter = std::default_delete<T>>
  class CompactUniquePtrTuple;

  template <typename T, typename Type, typename... Args>
  KRYS_ALWAYS_INLINE CompactUniquePtrTuple<T, Type> MakeCompactUniquePtr(Args &&...args)
  {
    return CompactUniquePtrTuple<T, Type>(CreateUnique<T>(std::forward<Args>(args)...));
  }

  template <typename T, typename Type, typename Deleter, typename... Args>
  KRYS_ALWAYS_INLINE CompactUniquePtrTuple<T, Type, Deleter> MakeCompactUniquePtr(Args &&...args)
  {
    return CompactUniquePtrTuple<T, Type, Deleter>(CreateUnique<T>(std::forward<Args>(args)...));
  }

  template <typename T, typename Type, typename Deleter>
  class CompactUniquePtrTuple final : NonCopyable<CompactUniquePtrTuple>
  {
    static_assert(Krys::allowCompactPointers<T>());

  public:
    CompactUniquePtrTuple() = default;

    template <typename U, typename UDeleter>
    requires(std::same_as<UDeleter, Deleter> || std::same_as<UDeleter, std::default_delete<U>>)
    CompactUniquePtrTuple(CompactUniquePtrTuple<U, Type, UDeleter> &&other)
        : m_data {std::exchange(other.m_data, {})}
    {
    }

    ~CompactUniquePtrTuple()
    {
      setPointer(nullptr);
    }

    template <typename U, typename UDeleter>
    requires(std::same_as<UDeleter, Deleter> || std::same_as<UDeleter, std::default_delete<U>>)
    CompactUniquePtrTuple<T, Type, Deleter> &operator=(CompactUniquePtrTuple<U, Type, UDeleter> &&other)
    {
      CompactUniquePtrTuple moved {WTF::move(other)};
      std::swap(m_data, moved.m_data);
      return *this;
    }

    T *pointer() const KRYS_LIFETIME_BOUND
    {
      return m_data.pointer();
    }

    std::unique_ptr<T, Deleter> moveToUniquePtr()
    {
      T *pointer = m_data.pointer();
      m_data.setPointer(nullptr);
      return std::unique_ptr<T, Deleter>(pointer);
    }

    void setPointer(std::nullptr_t)
    {
      deletePointer();
      m_data.setPointer(nullptr);
    }

    template <typename U, typename UDeleter>
    requires(std::same_as<UDeleter, Deleter> || std::same_as<UDeleter, std::default_delete<U>>)
    void setPointer(std::unique_ptr<U, UDeleter> &&pointer)
    {
      deletePointer();
      m_data.setPointer(pointer.release());
    }

    Type type() const
    {
      return m_data.type();
    }

    void setType(Type type)
    {
      m_data.setType(type);
    }

  private:
    CompactUniquePtrTuple(std::unique_ptr<T> &&pointer)
    {
      m_data.setPointer(pointer.release());
    }

    void deletePointer()
    {
      if (T *pointer = m_data.pointer())
        Deleter()(pointer);
    }

    template <typename U, typename E, typename... Args>
    friend CompactUniquePtrTuple<U, E> MakeCompactUniquePtr(Args &&...args);

    template <typename U, typename E, typename D, typename... Args>
    friend CompactUniquePtrTuple<U, E, D> MakeCompactUniquePtr(Args &&...args);

    template <typename, typename, typename>
    friend class CompactUniquePtrTuple;

    CompactPointerTuple<T *, Type> m_data;
  };
}