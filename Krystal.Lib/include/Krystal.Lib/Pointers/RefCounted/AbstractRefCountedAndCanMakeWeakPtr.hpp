#pragma once

#include "Krystal.Lib/Pointers/RefCounted/AbstractRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"

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
