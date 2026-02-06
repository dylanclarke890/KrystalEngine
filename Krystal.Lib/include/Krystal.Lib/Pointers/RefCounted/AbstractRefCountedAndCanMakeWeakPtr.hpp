#pragma once

#include "Krystal.Lib/Pointers/RefCounted/AbstractRefCounted.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CanMakeWeakPtr.hpp"

namespace Krys
{
  template <typename T, WeakPtrFactoryInitialization initializationMode = WeakPtrFactoryInitialization::Lazy>
  class AbstractRefCountedAndCanMakeWeakPtr : public AbstractRefCounted,
                                              public CanMakeWeakPtr<T, initializationMode>
  {
  };

  template <typename T, WeakPtrFactoryInitialization initializationMode = WeakPtrFactoryInitialization::Lazy>
  class AbstractRefCountedAndCanMakeSingleThreadWeakPtr
      : public AbstractRefCounted,
        public CanMakeSingleThreadWeakPtr<T, initializationMode>
  {
  };

}
