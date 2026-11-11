/*
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/Accessibility/AXNotifications.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  class RenderObject;

  namespace dom
  {
    class Document;
    class Element;
    class Node;
  }
}

namespace krys::boo::ax
{
  class AccessibilityNodeObject;
  class AccessibilityRenderObject;

  /// @brief A document's accessibility tree: creates the accessibility object for each node or renderer
  /// that needs one, keeps them up to date as the DOM and render tree change, and posts notifications of
  /// the changes to the platform.
  /// @note The document creates its cache once accessibility is enabled (`EnableAccessibility`), which the
  /// host does when assistive technology asks for the tree.
  class AXObjectCache final : public CanMakeWeakPtr<AXObjectCache>
  {
    KRYS_NON_COPY_MOVABLE(AXObjectCache);

  public:
    /// @brief Whether `GetOrCreate` may create an object for a node with no renderer that's part of a
    /// relation (such as a label referred to by `aria-labelledby`), which would otherwise not get one.
    enum class IsPartOfRelation : bool
    {
      No,
      Yes,
    };

  private:
    dom::Document &_document;
    HashMap<AXID, Ref<AccessibilityObject>> _objects;
    HashMap<const dom::Node *, AXID> _nodeIdMapping;
    // Renderers with no node (anonymous ones, such as list markers) are looked up by renderer.
    HashMap<const RenderObject *, AXID> _renderObjectIdMapping;
    uint64 _nextID {1};

    List<Pair<Ref<AccessibilityObject>, AXNotification>> _notificationsToPost;
    Timer _notificationPostTimer;

    ListHashSet<AXID> _deferredChildrenChangedList;
    ListHashSet<AXID> _deferredRendererChangedList;
    Timer _performCacheUpdateTimer;
    bool _performingDeferredCacheUpdate {false};

  public:
    explicit AXObjectCache(dom::Document &document) noexcept;
    ~AXObjectCache() noexcept;

    /// @brief Turns accessibility on: documents create their caches when next asked.
    static void EnableAccessibility() noexcept;
    /// @brief Turns accessibility off: documents create no new caches (existing ones stay).
    static void DisableAccessibility() noexcept;
    KRYS_NODISCARD static bool AccessibilityEnabled() noexcept;

    KRYS_NODISCARD dom::Document &GetDocument() const noexcept
    {
      return _document;
    }

    /// @brief The root of the tree: the document's web area.
    // FIXME: A scroll view object above the web area, with the specialised objects.
    KRYS_NODISCARD AccessibilityObject *RootObject() noexcept;
    /// @brief The object for the focused element (or its active descendant), or the nearest unignored
    /// ancestor if that object is ignored; the web area if nothing is focused.
    KRYS_NODISCARD AccessibilityObject *FocusedObject() noexcept;

    // Lookup and creation.

    KRYS_NODISCARD AccessibilityObject *Get(const dom::Node &node) const noexcept;
    KRYS_NODISCARD AccessibilityObject *Get(const RenderObject &renderer) const noexcept;
    KRYS_NODISCARD AccessibilityObject *ObjectForID(AXID id) const noexcept;

    /// @brief The object for @p node, created if it needs one: if it's rendered, or if it isn't but
    /// accessibility needs it anyway (`display: contents`, canvas fallback content, a focused node,
    /// @p isPartOfRelation).
    AccessibilityObject *GetOrCreate(dom::Node &node,
                                     IsPartOfRelation isPartOfRelation = IsPartOfRelation::No) noexcept;
    AccessibilityObject *GetOrCreate(dom::Node *node,
                                     IsPartOfRelation isPartOfRelation = IsPartOfRelation::No) noexcept
    {
      return node ? GetOrCreate(*node, isPartOfRelation) : nullptr;
    }
    /// @brief The object for @p renderer (its node's, if it has one), created if needed.
    AccessibilityObject *GetOrCreate(RenderObject &renderer) noexcept;
    AccessibilityObject *GetOrCreate(RenderObject *renderer) noexcept
    {
      return renderer ? GetOrCreate(*renderer) : nullptr;
    }

    /// @brief Removes and detaches @p node's object.
    void Remove(dom::Node &node) noexcept;
    /// @brief Removes and detaches @p renderer's object if it has no node; otherwise the node's object
    /// just loses its renderer.
    void Remove(RenderObject &renderer) noexcept;
    void Remove(AXID id) noexcept;

    // Changes.

    /// @brief @p node's children changed; for a rendered node, the renderer's children-changed covers it.
    void ChildrenChanged(dom::Node &node) noexcept;
    void ChildrenChanged(RenderObject &renderer, RenderObject *newChild = nullptr) noexcept;
    void ChildrenChanged(AccessibilityObject *object) noexcept;
    /// @brief Tells the platform @p object's role changed from @p previousRole.
    void HandleRoleChanged(AccessibilityObject &object, AccessibilityRole previousRole) noexcept;
    void RecomputeIsIgnored(dom::Node &node) noexcept;
    void RecomputeIsIgnored(RenderObject &renderer) noexcept;

    /// @brief Brings the tree up to date with the changes deferred since the last update: children
    /// changed and renderers lost.
    void PerformDeferredCacheUpdate() noexcept;

    // Notifications.

    /// @brief Queues @p notification for @p object, to post to the platform once the current task ends.
    void PostNotification(AccessibilityObject *object, AXNotification notification) noexcept;
    void PostNotification(dom::Node *node, AXNotification notification) noexcept;
    void PostNotification(RenderObject *renderer, AXNotification notification) noexcept;

  private:
    KRYS_NODISCARD AXID GenerateID() noexcept
    {
      return static_cast<AXID>(_nextID++);
    }
    KRYS_NODISCARD Ref<AccessibilityRenderObject> CreateObjectFromRenderer(RenderObject &renderer) noexcept;
    KRYS_NODISCARD Ref<AccessibilityNodeObject> CreateFromNode(dom::Node &node) noexcept;
    void CacheAndInitializeWrapper(AccessibilityObject &object, const dom::Node *node,
                                   const RenderObject *renderer) noexcept;
    AccessibilityObject *GetOrCreateSlow(dom::Node &node, IsPartOfRelation isPartOfRelation) noexcept;
    KRYS_NODISCARD AccessibilityObject *FocusedObjectForNode(dom::Node &node) noexcept;

    void HandleChildrenChanged(AccessibilityObject &object) noexcept;
    void HandleAllDeferredChildrenChanged() noexcept;
    void NotificationPostTimerFired() noexcept;
    void PerformCacheUpdateTimerFired() noexcept
    {
      PerformDeferredCacheUpdate();
    }

    // The platform's side: the host's accessibility bridge.
    void AttachWrapper(AccessibilityObject &object) noexcept;
    void DetachWrapper(AccessibilityObject &object, AccessibilityDetachmentType detachmentType) noexcept;
    void PostPlatformNotification(AccessibilityObject &object, AXNotification notification) noexcept;

    friend class AccessibilityObject;
  };
}
