/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserver.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  void IdTargetObserverRegistry::AddObserver(DOMStringAtom id, IdTargetObserver &observer) noexcept
  {
    if (id.View().empty())
    {
      return;
    }

    auto &set = _registry[id];
    if (!set)
    {
      set = CreateUnique<ObserverSet>();
    }
    set->Observers.insert(&observer);
  }

  void IdTargetObserverRegistry::RemoveObserver(DOMStringAtom id, IdTargetObserver &observer) noexcept
  {
    if (id.View().empty() || _registry.empty())
    {
      return;
    }

    auto iterator = _registry.find(id);
    krys_debug_assert(iterator != _registry.end());
    auto *set = iterator->second.get();
    set->Observers.erase(&observer);
    if (set->Observers.empty() && set != _notifyingObserversInSet)
    {
      _registry.erase(iterator);
    }
  }

  void IdTargetObserverRegistry::NotifyObserversInternal(Element &element, DOMStringAtom id) noexcept
  {
    krys_debug_assert(!_registry.empty());

    auto iterator = _registry.find(id);
    if (iterator == _registry.end())
    {
      return;
    }
    _notifyingObserversInSet = iterator->second.get();

    List<IdTargetObserver *> observers(_notifyingObserversInSet->Observers.begin(),
                                       _notifyingObserversInSet->Observers.end());
    for (auto *observer : observers)
    {
      if (_notifyingObserversInSet->Observers.contains(observer))
      {
        observer->IdTargetChanged(element);
      }
    }

    bool hasRemainingObservers = !_notifyingObserversInSet->Observers.empty();
    _notifyingObserversInSet = nullptr;

    if (!hasRemainingObservers)
    {
      _registry.erase(id);
    }
  }
}
