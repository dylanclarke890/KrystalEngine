#pragma once

#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"

namespace Krys
{
  template <typename T>
  class RefCountedAndCanMakeWeakPtr : public CanMakeWeakPtr<T>, public RefCounted<T>
  {
  public:
    void AddRef() const noexcept
    {
      RefCounted<T>::AddRef();
    }
    void SubRef() const noexcept
    {
      RefCounted<T>::SubRef();
    }
  };
}
