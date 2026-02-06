#pragma once

#include "Krystal.Lib/Pointers/RefCounted/CompactPointerTuple.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CompactPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefPtr.hpp"

namespace Krys
{
  template <typename T, typename Type>
  class CompactRefPtrTuple final
  {
    static_assert(Krys::allowCompactPointers<T>());

  public:
    CompactRefPtrTuple() = default;
    CompactRefPtrTuple(T *pointer, Type type)
    {
      setPointer(pointer);
      setType(type);
    }

    CompactRefPtrTuple(RefPtr<T> &&pointer, Type type)
    {
      setPointer(WTF::move(pointer));
      setType(type);
    }

    CompactRefPtrTuple(const CompactRefPtrTuple &other)
    {
      setPointer(other.pointer());
      setType(other.type());
    }

    CompactRefPtrTuple(CompactRefPtrTuple &&other)
    {
      m_data.setPointer(other.pointer());
      m_data.setType(other.type());
      other.m_data.setPointer(nullptr);
      other.m_data.setType({});
    }

    CompactRefPtrTuple &operator=(const CompactRefPtrTuple &other)
    {
      CompactRefPtrTuple copied(other);
      swap(copied);
      return *this;
    }

    CompactRefPtrTuple &operator=(CompactRefPtrTuple &&other)
    {
      CompactRefPtrTuple moved(WTF::move(other));
      swap(moved);
      return *this;
    }

    ~CompactRefPtrTuple()
    {
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(m_data.pointer());
    }

    T *pointer() const KRYS_LIFETIME_BOUND
    {
      return m_data.pointer();
    }

    void setPointer(T *pointer)
    {
      auto *old = m_data.pointer();
      m_data.setPointer(Krys::DefaultRefDerefTraits<T>::refIfNotNull(pointer));
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(old);
    }

    void setPointer(RefPtr<T> &&pointer)
    {
      auto willRelease = WTF::move(pointer);
      auto *old = m_data.pointer();
      m_data.setPointer(willRelease.leakRef());
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(old);
    }

    void setPointer(Ref<T> &&pointer)
    {
      auto willRelease = WTF::move(pointer);
      auto *old = m_data.pointer();
      m_data.setPointer(&willRelease.leakRef());
      Krys::DefaultRefDerefTraits<T>::derefIfNotNull(old);
    }

    Type type() const
    {
      return m_data.type();
    }
    void setType(Type type)
    {
      m_data.setType(type);
    }

    void swap(CompactRefPtrTuple<T, Type> &other)
    {
      m_data.swap(other.m_data);
    }

  private:
    CompactPointerTuple<T *, Type> m_data;
  };
}
