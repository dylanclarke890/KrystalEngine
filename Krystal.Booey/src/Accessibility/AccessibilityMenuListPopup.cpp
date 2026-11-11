/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityMenuListPopup.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListOption.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"

namespace krys::boo::ax
{
  Ref<AccessibilityMenuListPopup> AccessibilityMenuListPopup::Create(AXID id, AXObjectCache &cache) noexcept
  {
    return AdoptRef(*new AccessibilityMenuListPopup(id, cache));
  }

  AccessibilityMenuListPopup::AccessibilityMenuListPopup(AXID id, AXObjectCache &cache) noexcept
      : AccessibilityMockObject(id, cache)
  {
  }

  bool AccessibilityMenuListPopup::IsEnabled() const noexcept
  {
    auto *parent = _parent.get();
    return parent && parent->IsEnabled();
  }

  void AccessibilityMenuListPopup::UpdateChildrenIfNecessary() noexcept
  {
    if (_childrenDirty || _subtreeDirty)
    {
      HandleChildrenChanged();
      _childrenDirty = false;
      _subtreeDirty = false;
      return;
    }
    AccessibilityMockObject::UpdateChildrenIfNecessary();
  }

  void AccessibilityMenuListPopup::AddChildren() noexcept
  {
    _childrenInitialized = true;
    auto *parent = _parent.get();
    auto *select = parent ? DynamicDowncast<html::HTMLSelectElement>(parent->GetNode()) : nullptr;
    auto *cache = AxObjectCache();
    if (!select || !cache)
    {
      return;
    }
    for (const auto &item : select->ListItems())
    {
      if (!item || !item->IsConnected())
      {
        continue;
      }
      if (auto *option = DynamicDowncast<AccessibilityMenuListOption>(cache->GetOrCreate(*item)))
      {
        option->SetParent(this);
        AddChild(option, DescendIfIgnored::No);
      }
    }
  }

  void AccessibilityMenuListPopup::HandleChildrenChanged() noexcept
  {
    auto *cache = AxObjectCache();
    if (!cache)
    {
      return;
    }
    // Options no longer in the document lose their objects (their nodes may live on).
    for (auto index = _children.size(); index > 0; --index)
    {
      auto child = _children[index - 1];
      auto *node = child->GetNode();
      if (!node || !node->IsConnected())
      {
        child->DetachFromParent();
        cache->Remove(child->ObjectID());
      }
    }
    AccessibilityMockObject::ClearChildren();
    AddChildren();
  }
}
