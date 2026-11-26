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

#include "Krystal.Booey/Accessibility/AXMenuList.hpp"
#include "Krystal.Booey/Accessibility/AXMenuListPopup.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"

namespace krys::boo
{
  AXMenuList::AXMenuList(AXId id, RenderMenuList &renderer, AXObjectCache &cache) noexcept
      : AXRenderObject(id, &renderer, renderer.GetNode(), cache),
        _popup(ShareRef(Downcast<AXMenuListPopup>(*cache.Create(AXRole::MenuListPopup))))
  {
  }

  AXMenuList::~AXMenuList() noexcept = default;

  Ref<AXMenuList> AXMenuList::Create(AXId id, RenderMenuList &renderer, AXObjectCache &cache) noexcept
  {
    auto menuList = AdoptRef(*new AXMenuList(id, renderer, cache));
    menuList->_popup->SetParent(menuList.get());
    menuList->_children.emplace_back(menuList->_popup);
    menuList->_childrenInitialized = true;
    return menuList;
  }

  void AXMenuList::DetachRemoteParts(AXDetachmentType detachmentType) noexcept
  {
    // The pop-up goes with the menu list.
    if (auto *cache = AxObjectCache(); cache && detachmentType != AXDetachmentType::CacheDestroyed)
    {
      cache->Remove(_popup->ObjectID());
    }
    _popup->DetachFromParent();
    _children.clear();
    AXRenderObject::DetachRemoteParts(detachmentType);
  }

  AXMenuListPopup &AXMenuList::Popup() const noexcept
  {
    return *_popup;
  }

  void AXMenuList::UpdateChildrenIfNecessary() noexcept
  {
    if (_subtreeDirty)
    {
      _popup->SetNeedsToUpdateSubtree();
    }
    _subtreeDirty = false;
  }

  bool AXMenuList::IsCollapsed() const noexcept
  {
    auto *menuList = DynamicDowncast<RenderMenuList>(Renderer());
    return !menuList || !menuList->PopupIsVisible();
  }

  void AXMenuList::DidUpdateActiveOption(int32 optionIndex) noexcept
  {
    auto *cache = AxObjectCache();
    if (!cache)
    {
      return;
    }
    // The pop-up's children may not have been made yet.
    auto &popup = Popup();
    if (optionIndex >= 0 && static_cast<size_t>(optionIndex) < popup.Children().size())
    {
      popup.DidUpdateActiveOption(optionIndex);
    }
    cache->PostNotification(this, AXNotification::MenuListValueChanged);
  }

  bool AXMenuList::Press() noexcept
  {
    auto *menuList = DynamicDowncast<RenderMenuList>(Renderer());
    if (!menuList)
    {
      return false;
    }
    auto *element = GetElement();
    auto notification = AXNotification::PressDidFail;
    if (element && !element->IsDisabledFormControl())
    {
      if (menuList->PopupIsVisible())
      {
        menuList->HidePopup();
      }
      else
      {
        menuList->ShowPopup();
      }
      notification = AXNotification::PressDidSucceed;
    }
    if (auto *cache = AxObjectCache())
    {
      cache->PostNotification(element, notification);
    }
    return true;
  }

  bool AXMenuList::CanSetFocusAttribute() const noexcept
  {
    auto *element = GetElement();
    return element && !element->IsDisabledFormControl();
  }
}
