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

#include "Krystal.Booey/Accessibility/AccessibilityMenuList.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListPopup.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"

namespace krys::boo::ax
{
  AccessibilityMenuList::AccessibilityMenuList(AXID id, RenderMenuList &renderer,
                                               AXObjectCache &cache) noexcept
      : AccessibilityRenderObject(id, &renderer, renderer.GetNode(), cache),
        _popup(
          ShareRef(Downcast<AccessibilityMenuListPopup>(*cache.Create(AccessibilityRole::MenuListPopup))))
  {
  }

  AccessibilityMenuList::~AccessibilityMenuList() noexcept = default;

  Ref<AccessibilityMenuList> AccessibilityMenuList::Create(AXID id, RenderMenuList &renderer,
                                                           AXObjectCache &cache) noexcept
  {
    auto menuList = AdoptRef(*new AccessibilityMenuList(id, renderer, cache));
    menuList->_popup->SetParent(menuList.get());
    menuList->_children.push_back(menuList->_popup);
    menuList->_childrenInitialized = true;
    return menuList;
  }

  void AccessibilityMenuList::DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept
  {
    // The pop-up goes with the menu list.
    if (auto *cache = AxObjectCache(); cache && detachmentType != AccessibilityDetachmentType::CacheDestroyed)
    {
      cache->Remove(_popup->ObjectID());
    }
    _popup->DetachFromParent();
    _children.clear();
    AccessibilityRenderObject::DetachRemoteParts(detachmentType);
  }

  AccessibilityMenuListPopup &AccessibilityMenuList::Popup() const noexcept
  {
    return *_popup;
  }

  void AccessibilityMenuList::UpdateChildrenIfNecessary() noexcept
  {
    if (_subtreeDirty)
    {
      _popup->SetNeedsToUpdateSubtree();
    }
    _subtreeDirty = false;
  }

  bool AccessibilityMenuList::IsCollapsed() const noexcept
  {
    auto *menuList = DynamicDowncast<RenderMenuList>(Renderer());
    return !menuList || !menuList->PopupIsVisible();
  }

  bool AccessibilityMenuList::CanSetFocusAttribute() const noexcept
  {
    auto *element = GetElement();
    return element && !element->IsDisabledFormControl();
  }
}
