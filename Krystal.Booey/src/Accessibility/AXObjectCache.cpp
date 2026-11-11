/*
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityListBoxOption.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuList.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListOption.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityMenuListPopup.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityProgressIndicator.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityRenderObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilitySlider.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableColumn.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableHeaderContainer.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMeter.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderProgress.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderSlider.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <utility>

namespace krys::boo::ax
{
  namespace
  {
    bool gAccessibilityEnabled = false;

    KRYS_NODISCARD bool IsHTMLElementNamed(const dom::Node &node, dom::DOMStringView localName) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && Is<html::HTMLElement>(*element) && element->LocalName() == localName;
    }

    /// @brief Whether @p node is inside a canvas's fallback content (but not a user agent shadow tree).
    KRYS_NODISCARD bool IsInCanvasSubtree(const dom::Element &composedParent) noexcept
    {
      for (const dom::Node *ancestor = &composedParent; ancestor; ancestor = ancestor->ParentInComposedTree())
      {
        if (IsHTMLElementNamed(*ancestor, u8"canvas"))
        {
          // Descendants of the user agent shadow trees inside the canvas (such as an input's inner editor)
          // are left to their host.
          auto shadowRoot = composedParent.ShadowRoot();
          return !shadowRoot || shadowRoot->Mode() != dom::ShadowRootMode::UserAgent;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool NodeRendererIsValid(const dom::Node &node) noexcept
    {
      auto *renderer = node.Renderer();
      if (!renderer)
      {
        return false;
      }
      // A node's renderer may still be one it no longer uses while it's being torn down.
      return !renderer->BeingDestroyed();
    }
  }

  AXObjectCache::AXObjectCache(dom::Document &document) noexcept
      : _document(document), _notificationPostTimer(*this, &AXObjectCache::NotificationPostTimerFired),
        _performCacheUpdateTimer(*this, &AXObjectCache::PerformCacheUpdateTimerFired)
  {
  }

  AXObjectCache::~AXObjectCache() noexcept
  {
    _notificationPostTimer.Stop();
    _performCacheUpdateTimer.Stop();

    // Moved out: detaching an object can remove the objects it made (a table's columns) from the cache.
    auto objects = std::exchange(_objects, {});
    for (const auto &[id, object] : objects)
    {
      object->Detach(AccessibilityDetachmentType::CacheDestroyed);
    }
  }

  void AXObjectCache::EnableAccessibility() noexcept
  {
    gAccessibilityEnabled = true;
  }

  void AXObjectCache::DisableAccessibility() noexcept
  {
    gAccessibilityEnabled = false;
  }

  bool AXObjectCache::AccessibilityEnabled() noexcept
  {
    return gAccessibilityEnabled;
  }

  AccessibilityObject *AXObjectCache::RootObject() noexcept
  {
    if (!gAccessibilityEnabled)
    {
      return nullptr;
    }
    return GetOrCreate(_document.GetRenderView());
  }

  AccessibilityObject *AXObjectCache::FocusedObject() noexcept
  {
    if (!gAccessibilityEnabled)
    {
      return nullptr;
    }

    _document.UpdateStyleIfNeeded();
    if (auto *focusedElement = _document.FocusedElement())
    {
      return FocusedObjectForNode(*focusedElement);
    }
    return FocusedObjectForNode(_document);
  }

  AccessibilityObject *AXObjectCache::FocusedObjectForNode(dom::Node &node) noexcept
  {
    // FIXME: The image map's area object for a focused area element (focusedImageMapUIElement), with image
    // maps; and the active descendant (shouldFocusActiveDescendant), with relations.
    auto *focus = GetOrCreate(node);
    if (!focus)
    {
      return nullptr;
    }
    if (focus->IsIgnored())
    {
      return focus->ParentObjectUnignored();
    }
    return focus;
  }

  AccessibilityObject *AXObjectCache::Get(const dom::Node &node) const noexcept
  {
    if (auto *document = DynamicDowncast<dom::Document>(node)) KRYS_UNLIKELY
    {
      auto *renderView = document->GetRenderView();
      return renderView ? Get(*renderView) : nullptr;
    }
    auto found = _nodeIdMapping.find(&node);
    return found == _nodeIdMapping.end() ? nullptr : ObjectForID(found->second);
  }

  AccessibilityObject *AXObjectCache::Get(const RenderObject &renderer) const noexcept
  {
    if (auto *node = renderer.GetNode())
    {
      auto found = _nodeIdMapping.find(node);
      return found == _nodeIdMapping.end() ? nullptr : ObjectForID(found->second);
    }
    auto found = _renderObjectIdMapping.find(&renderer);
    return found == _renderObjectIdMapping.end() ? nullptr : ObjectForID(found->second);
  }

  AccessibilityObject *AXObjectCache::ObjectForID(AXID id) const noexcept
  {
    auto found = _objects.find(id);
    return found == _objects.end() ? nullptr : found->second.get();
  }

  Ref<AccessibilityRenderObject> AXObjectCache::CreateObjectFromRenderer(RenderObject &renderer) noexcept
  {
    // FIXME: The specialised objects for SVG, images and MathML, with them.
    auto *node = renderer.GetNode();
    if (auto *menuList = DynamicDowncast<RenderMenuList>(renderer))
    {
      return AccessibilityMenuList::Create(GenerateID(), *menuList, *this);
    }
    if (Is<RenderProgress>(renderer) || Is<RenderMeter>(renderer) || Is<html::HTMLProgressElement>(node)
        || Is<html::HTMLMeterElement>(node))
    {
      return AccessibilityProgressIndicator::Create(GenerateID(), renderer, *this);
    }
    if (Is<RenderSlider>(renderer))
    {
      return AccessibilitySlider::Create(GenerateID(), renderer, *this);
    }
    return AccessibilityRenderObject::Create(GenerateID(), renderer, *this);
  }

  Ref<AccessibilityNodeObject> AXObjectCache::CreateFromNode(dom::Node &node) noexcept
  {
    // FIXME: The specialised objects for image map areas and SVG, with them.
    if (auto *element = DynamicDowncast<dom::Element>(node);
        element && (Is<html::HTMLProgressElement>(*element) || Is<html::HTMLMeterElement>(*element)))
    {
      return AccessibilityProgressIndicator::Create(GenerateID(), *element, *this);
    }
    return AccessibilityRenderObject::Create(GenerateID(), node, *this);
  }

  AccessibilityObject *AXObjectCache::Create(AccessibilityRole role) noexcept
  {
    RefPtr<AccessibilityObject> object;
    switch (role)
    {
      case AccessibilityRole::SliderThumb:
        object = AccessibilitySliderThumb::Create(GenerateID(), *this);
        break;
      case AccessibilityRole::MenuListPopup:
        object = AccessibilityMenuListPopup::Create(GenerateID(), *this);
        break;
      case AccessibilityRole::Column: object = AccessibilityTableColumn::Create(GenerateID(), *this); break;
      case AccessibilityRole::TableHeaderContainer:
        object = AccessibilityTableHeaderContainer::Create(GenerateID(), *this);
        break;
      default:
        // FIXME: Spin buttons and their parts, with them.
        break;
    }
    if (!object)
    {
      return nullptr;
    }
    CacheAndInitializeWrapper(*object, nullptr, nullptr);
    return object.get();
  }

  void AXObjectCache::CacheAndInitializeWrapper(AccessibilityObject &object, const dom::Node *node,
                                                const RenderObject *renderer) noexcept
  {
    auto id = object.ObjectID();
    if (node)
    {
      _nodeIdMapping.insert_or_assign(node, id);
    }
    else if (renderer)
    {
      _renderObjectIdMapping.insert_or_assign(renderer, id);
    }

    _objects.insert_or_assign(id, ShareRef(object));
    object.Init();
    AttachWrapper(object);
  }

  AccessibilityObject *AXObjectCache::GetOrCreate(dom::Node &node, IsPartOfRelation isPartOfRelation) noexcept
  {
    if (auto *object = Get(node))
    {
      return object;
    }
    return GetOrCreateSlow(node, isPartOfRelation);
  }

  AccessibilityObject *AXObjectCache::GetOrCreateSlow(dom::Node &node,
                                                      IsPartOfRelation isPartOfRelation) noexcept
  {
    krys_debug_assert(&node.NodeDocument() == &_document);

    // An option or option group is its select's, rendered or not: a list box's child, or (for an option) a
    // child of a menu list's pop-up, as long as the menu list is rendered.
    auto *option = DynamicDowncast<html::HTMLOptionElement>(node);
    auto *group = DynamicDowncast<html::HTMLOptGroupElement>(node);
    if (option || group)
    {
      auto *select = option ? option->OwnerSelectElement() : group->OwnerSelectElement();
      if (!select)
      {
        return nullptr;
      }
      RefPtr<AccessibilityObject> object;
      if (select->UsesMenuList())
      {
        if (!option || !select->Renderer())
        {
          return nullptr;
        }
        object = AccessibilityMenuListOption::Create(GenerateID(), *option, *this);
      }
      else
      {
        object = AccessibilityListBoxOption::Create(GenerateID(), Downcast<html::HTMLElement>(node), *this);
      }
      CacheAndInitializeWrapper(*object, &node, nullptr);
      return object.get();
    }

    if (NodeRendererIsValid(node))
    {
      return GetOrCreate(*node.Renderer());
    }

    if (auto *document = DynamicDowncast<dom::Document>(node)) KRYS_UNLIKELY
    {
      return GetOrCreate(document->GetRenderView());
    }

    auto *composedParent = node.ParentElementInComposedTree();
    if (!composedParent)
    {
      return nullptr;
    }

    bool inCanvasSubtree = IsInCanvasSubtree(*composedParent);
    // FIXME: Descendants of the targets of relations (isDescendantOfRelatedNode), with relations.
    if (isPartOfRelation == IsPartOfRelation::No)
    {
      bool insideMeterElement = IsHTMLElementNamed(*composedParent, u8"meter");
      auto *element = DynamicDowncast<dom::Element>(node);
      bool hasDisplayContents = element && element->HasDisplayContents();
      bool isPopover = element && Is<html::HTMLElement>(*element) && element->HasAttribute(u8"popover");
      bool isFocused = element && _document.FocusedElement() == element;
      bool isAreaElement = IsHTMLElementNamed(node, u8"area");
      if (!inCanvasSubtree && !insideMeterElement && !hasDisplayContents && !isPopover && !isFocused
          && !isAreaElement)
      {
        return nullptr;
      }
    }

    // Fallback content is only focusable as long as the canvas is displayed and visible. Update the style
    // before focusability gets checked.
    if (inCanvasSubtree)
    {
      _document.UpdateStyleIfNeeded();
    }

    auto object = CreateFromNode(node);
    krys_debug_assert(!Get(node));
    CacheAndInitializeWrapper(*object, &node, nullptr);
    // Compute the object's initial ignored state.
    object->RecomputeIsIgnored();
    // Asking whether it's ignored can detach it.
    if (object->IsDetached())
    {
      return nullptr;
    }
    return object.get();
  }

  AccessibilityObject *AXObjectCache::GetOrCreate(RenderObject &renderer) noexcept
  {
    if (auto *object = Get(renderer))
    {
      return object;
    }

    // Don't create an object for this renderer if it's being destroyed.
    if (renderer.BeingDestroyed())
    {
      return nullptr;
    }

    auto object = CreateObjectFromRenderer(renderer);
    krys_debug_assert(!Get(renderer));
    CacheAndInitializeWrapper(*object, renderer.GetNode(), &renderer);
    // Compute the object's initial ignored state.
    object->RecomputeIsIgnored();
    // Asking whether it's ignored can detach it.
    if (object->IsDetached())
    {
      return nullptr;
    }
    return object.get();
  }

  void AXObjectCache::Remove(AXID id) noexcept
  {
    auto found = _objects.find(id);
    if (found == _objects.end())
    {
      return;
    }
    auto object = krys::move(found->second);
    _objects.erase(found);

    // FIXME: Removing the object's relations (removeAllRelations), with relations.
    object->Detach(AccessibilityDetachmentType::ElementDestroyed);
    _deferredChildrenChangedList.remove(id);
    _deferredRendererChangedList.remove(id);
    _deferredRecomputeTableIsExposedList.remove(id);
    _deferredRecomputeTableCellSlotsList.remove(id);
  }

  void AXObjectCache::Remove(RenderObject &renderer) noexcept
  {
    if (auto *node = renderer.GetNode())
    {
      // Objects with nodes are removed when their node is, not their renderer.
      if (auto *object = DynamicDowncast<AccessibilityRenderObject>(Get(*node)))
      {
        if (object->SetRendererIfNeeded(nullptr))
        {
          // The object still needs updating for the renderer it lost.
          _deferredRendererChangedList.add(object->ObjectID());
        }
      }
      return;
    }

    auto found = _renderObjectIdMapping.find(&renderer);
    if (found == _renderObjectIdMapping.end())
    {
      return;
    }
    auto id = found->second;
    _renderObjectIdMapping.erase(found);
    Remove(id);
  }

  void AXObjectCache::Remove(dom::Node &node) noexcept
  {
    auto found = _nodeIdMapping.find(&node);
    if (found != _nodeIdMapping.end())
    {
      auto id = found->second;
      _nodeIdMapping.erase(found);
      Remove(id);
    }

    if (auto *renderer = node.Renderer())
    {
      Remove(*renderer);
    }
    // FIXME: Removing the element from the deferred attribute, text and modal changes, with them.
  }

  void AXObjectCache::ChildrenChanged(dom::Node &node) noexcept
  {
    // A rendered node's renderer reports its children changing.
    if (!node.Renderer())
    {
      ChildrenChanged(Get(node));
    }
  }

  void AXObjectCache::ChildrenChanged(RenderObject &renderer, RenderObject *newChild) noexcept
  {
    if (renderer.IsAnonymous())
    {
      // The tree is built from the DOM, so an anonymous renderer rarely has an object, and its subtree can
      // change with no other renderer reporting it: tell the nearest ancestor that has one.
      for (auto *ancestor = &renderer; ancestor; ancestor = ancestor->Parent())
      {
        if (auto *object = Get(*ancestor))
        {
          ChildrenChanged(object);
          break;
        }
      }
    }
    else
    {
      ChildrenChanged(Get(renderer));
    }

    // FIXME: Deferring the added or removed element's menu-opened and live region checks
    // (deferElementAddedOrRemoved), with them.
    (void)newChild;
  }

  void AXObjectCache::ChildrenChanged(AccessibilityObject *object) noexcept
  {
    if (!object)
    {
      return;
    }
    _deferredChildrenChangedList.add(object->ObjectID());

    // Adding or removing rows can make a layout table a data table or the other way round.
    auto *element = object->GetElement();
    if (Is<html::HTMLTableElement>(element))
    {
      DeferRecomputeTableIsExposed(element);
    }
    else if (auto *section = DynamicDowncast<html::HTMLTableSectionElement>(element))
    {
      DeferRecomputeTableIsExposed(const_cast<html::HTMLTableElement *>(section->FindParentTable()));
    }
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::DeferRecomputeTableIsExposed(dom::Element *element) noexcept
  {
    // A table with no object yet works it out when it gets one.
    auto *table = DynamicDowncast<html::HTMLTableElement>(element);
    auto *object = table ? Get(*table) : nullptr;
    if (!object)
    {
      return;
    }
    _deferredRecomputeTableIsExposedList.add(object->ObjectID());
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::DeferRecomputeTableCellSlots(AccessibilityNodeObject &table) noexcept
  {
    _deferredRecomputeTableCellSlotsList.add(table.ObjectID());
    StartPerformCacheUpdateTimer();
  }

  void AXObjectCache::StartPerformCacheUpdateTimer() noexcept
  {
    if (!_performCacheUpdateTimer.IsActive())
    {
      _performCacheUpdateTimer.StartOneShot({});
    }
  }

  void AXObjectCache::HandleChildrenChanged(AccessibilityObject &object) noexcept
  {
    // A menu list's children are its pop-up's.
    if (auto *menuList = DynamicDowncast<AccessibilityMenuList>(object))
    {
      HandleChildrenChanged(menuList->Popup());
    }
    else if (auto *popup = DynamicDowncast<AccessibilityMenuListPopup>(object))
    {
      popup->HandleChildrenChanged();
      return;
    }
    else if (auto *nodeObject = DynamicDowncast<AccessibilityNodeObject>(object);
             nodeObject && nodeObject->IsTable())
    {
      DeferRecomputeTableCellSlots(*nodeObject);
    }
    else if (auto *parentTable = DynamicDowncast<AccessibilityNodeObject>(
               nodeObject ? nodeObject->ParentTableIfExposedTableRow() : nullptr))
    {
      DeferRecomputeTableCellSlots(*parentTable);
    }
    if (!object.GetNode() && !object.Renderer())
    {
      return;
    }

    // Make the subtree dirty so everything below is updated.
    object.SetNeedsToUpdateSubtree();
    object.RecomputeIsIgnored();

    // Go up the existing ancestors and mark their children out of date.
    for (auto *parent = &object; parent; parent = parent->ParentObject())
    {
      parent->SetNeedsToUpdateChildren();
      // FIXME: Live region, non-native text control value, label and description notifications for the
      // ancestors, with them.
    }

    // A list's role depends on its children.
    if (auto *nodeObject = DynamicDowncast<AccessibilityNodeObject>(object);
        nodeObject && nodeObject->IsAccessibilityList())
    {
      object.UpdateRole();
    }
  }

  void AXObjectCache::HandleAllDeferredChildrenChanged() noexcept
  {
    // Handling a change can defer another (an object's ignored state changing), so repeat until none are
    // left.
    while (!_deferredChildrenChangedList.isEmpty())
    {
      auto deferred = std::exchange(_deferredChildrenChangedList, {});
      List<Ref<AccessibilityObject>> objects;
      for (auto id : deferred)
      {
        if (auto *object = ObjectForID(id))
        {
          objects.push_back(ShareRef(*object));
        }
      }
      for (const auto &object : objects)
      {
        HandleChildrenChanged(*object);
      }
      for (const auto &object : objects)
      {
        PostPlatformNotification(*object, AXNotification::ChildrenChanged);
      }
    }
  }

  void AXObjectCache::HandleRoleChanged(AccessibilityObject &object, AccessibilityRole previousRole) noexcept
  {
    // The new role may change whether the object is ignored and what its parent's children are.
    (void)previousRole;
    object.RecomputeIsIgnored();
    PostNotification(&object, AXNotification::RoleChanged);
  }

  void AXObjectCache::RecomputeIsIgnored(dom::Node &node) noexcept
  {
    if (auto *object = Get(node))
    {
      object->RecomputeIsIgnored();
    }
  }

  void AXObjectCache::RecomputeIsIgnored(RenderObject &renderer) noexcept
  {
    if (auto *object = Get(renderer))
    {
      object->RecomputeIsIgnored();
    }
  }

  void AXObjectCache::PerformDeferredCacheUpdate() noexcept
  {
    if (_performingDeferredCacheUpdate)
    {
      return;
    }
    SetForScope performing(_performingDeferredCacheUpdate, true);
    _performCacheUpdateTimer.Stop();

    if (!_document.View())
    {
      return;
    }
    // The tree is only updated correctly against a clean layout.
    (void)_document.UpdateLayoutIgnorePendingStylesheets();

    auto rendererChanged = std::exchange(_deferredRendererChangedList, {});
    for (auto id : rendererChanged)
    {
      if (auto object = ShareRefPtr(ObjectForID(id)))
      {
        object->UpdateRole();
        object->RecomputeIsIgnored();
      }
    }

    auto recomputeTableIsExposed = std::exchange(_deferredRecomputeTableIsExposedList, {});
    for (auto id : recomputeTableIsExposed)
    {
      if (auto table = ShareRefPtr(DynamicDowncast<AccessibilityNodeObject>(ObjectForID(id))))
      {
        table->RecomputeIsExposableIfNecessary();
      }
    }

    HandleAllDeferredChildrenChanged();

    auto recomputeTableCellSlots = std::exchange(_deferredRecomputeTableCellSlotsList, {});
    for (auto id : recomputeTableCellSlots)
    {
      if (auto table = ShareRefPtr(DynamicDowncast<AccessibilityNodeObject>(ObjectForID(id))))
      {
        table->SetCellSlotsDirty();
      }
    }

    // FIXME: The other deferred updates (attribute changes, text changes, recomputing ignored states,
    // modals, relations and focus), with the changes they come from.
  }

  void AXObjectCache::PostNotification(AccessibilityObject *object, AXNotification notification) noexcept
  {
    if (!object)
    {
      object = Get(_document);
    }
    if (!object)
    {
      return;
    }

    _notificationsToPost.emplace_back(ShareRef(*object), notification);
    if (!_notificationPostTimer.IsActive())
    {
      _notificationPostTimer.StartOneShot({});
    }
  }

  void AXObjectCache::PostNotification(dom::Node *node, AXNotification notification) noexcept
  {
    // Get an existing object (creating one here can lay out, in the middle of a render tree update): the
    // node's or its nearest ancestor's.
    for (; node; node = node->ParentNode())
    {
      if (auto *object = Get(*node))
      {
        PostNotification(object, notification);
        return;
      }
    }
  }

  void AXObjectCache::PostNotification(RenderObject *renderer, AXNotification notification) noexcept
  {
    for (; renderer; renderer = renderer->Parent())
    {
      if (auto *object = Get(*renderer))
      {
        PostNotification(object, notification);
        return;
      }
    }
  }

  void AXObjectCache::NotificationPostTimerFired() noexcept
  {
    // Accessibility may have been turned off since the notifications were queued.
    if (!gAccessibilityEnabled || !_document.HasLivingRenderTree())
    {
      return;
    }

    // Posting can queue more notifications; those wait for the next time the timer fires.
    auto notifications = std::exchange(_notificationsToPost, {});
    for (auto &[object, notification] : notifications)
    {
      if (object->IsDetached() || !object->AxObjectCache())
      {
        continue;
      }
      if (notification == AXNotification::MenuOpened)
      {
        // Only notify if the object is in fact a menu.
        object->UpdateChildrenIfNecessary();
        if (object->Role() != AccessibilityRole::Menu)
        {
          continue;
        }
      }
      PostPlatformNotification(*object, notification);
    }
  }

  void AXObjectCache::AttachWrapper(AccessibilityObject &object) noexcept
  {
    // FIXME: Creating the platform's wrapper for the object (the chrome client's attachWrapper), with the
    // host's accessibility bridge.
    (void)object;
  }

  void AXObjectCache::DetachWrapper(AccessibilityObject &object,
                                    AccessibilityDetachmentType detachmentType) noexcept
  {
    // FIXME: Releasing the platform's wrapper for the object (the chrome client's detachWrapper), with the
    // host's accessibility bridge.
    (void)object;
    (void)detachmentType;
  }

  void AXObjectCache::PostPlatformNotification(AccessibilityObject &object,
                                               AXNotification notification) noexcept
  {
    // FIXME: Posting the notification to the platform (the chrome client's postAccessibilityNotification),
    // with the host's accessibility bridge.
    (void)object;
    (void)notification;
  }

  utf8_stringview NotificationToString(AXNotification notification) noexcept
  {
    switch (notification)
    {
#define KRYS_AX_NOTIFICATION_NAME(name)                                                                      \
  case AXNotification::name: return u8"" #name;
      KRYS_AX_NOTIFICATIONS(KRYS_AX_NOTIFICATION_NAME)
#undef KRYS_AX_NOTIFICATION_NAME
    }
    return {};
  }
}
