#pragma once

#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCounted.hpp"

namespace Krys
{
  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  class AbstractRefCountedAndCanMakeWeakPtr : public AbstractRefCounted,
                                              public CanMakeWeakPtr<T, InitializationMode>
  {
  };

  template <typename T, WeakPtrFactoryInitialization InitializationMode = WeakPtrFactoryInitialization::Lazy>
  class AbstractRefCountedAndCanMakeSingleThreadWeakPtr
      : public AbstractRefCounted,
        public CanMakeSingleThreadWeakPtr<T, InitializationMode>
  {
  };
}
