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

#include "Krystal.Booey/Accessibility/AXNotifications.hpp"
#include "Krystal.Booey/Accessibility/AXObject.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Time/Timer.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  class ContainerNode;
  class Document;
  class Element;
  class HTMLAreaElement;
  class HTMLInputElement;
  class LocalFrameView;
  class Node;
  class RenderElement;
  class RenderObject;
  class RenderStyle;
  class Scrollbar;

  class AXNodeObject;
  class AXRenderObject;

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
    boo::Document &_document;
    HashMap<AXId, Ref<AXObject>> _objects;
    HashMap<const boo::Node *, AXId> _nodeIdMapping;
    // Renderers with no node (anonymous ones, such as list markers) are looked up by renderer.
    HashMap<const RenderObject *, AXId> _renderObjectIdMapping;
    uint64 _nextID {1};
    // The document's view's object.
    Maybe<AXId> _scrollViewID;

    List<Pair<Ref<AXObject>, AXNotification>> _notificationsToPost;

    // Text inserted or deleted, posted with the notifications.
    struct TextChange
    {
      Ref<AXObject> Object;
      AXTextChange Change;
      uint64 Offset;
      DOMString Text;
    };

    List<TextChange> _textChangesToPost;
    Timer _notificationPostTimer;

    // Each object's relations to others (both ways round), worked out for the whole document when needed.
    using AXRelations = HashMap<AXRelation, ListHashSet<AXId>>;
    HashMap<AXId, AXRelations> _relations;
    // The objects some other object is related to.
    HashSet<AXId> _relationTargets;
    // While the relations are worked out again, each owned object's owner before (so an unchanged aria-owns
    // doesn't change any children).
    HashMap<AXId, AXId> _previousOwners;
    // While a tree is being built, each object's ignored state, so it isn't worked out again for each
    // descendant (WebKit's AXComputedObjectAttributeCache); null when not caching.
    Maybe<HashMap<AXId, bool>> _computedIgnoredStates;
    bool _relationsNeedUpdate {true};

    // An attribute change waiting for its element's renderer to be up to date.
    struct DeferredAttributeChange
    {
      boo::Element *Element {nullptr};
      DOMStringAtom Name;
      DOMString OldValue;
      DOMString NewValue;
    };

    ListHashSet<AXId> _deferredChildrenChangedList;
    // Elements added to or removed from the tree, which may be live regions or menus that opened.
    ListHashSet<boo::Element *> _deferredElementAddedOrRemovedList;
    // Live regions whose contents changed, each told once when the notifications are next posted.
    ListHashSet<AXId> _changedLiveRegions;
    List<DeferredAttributeChange> _deferredAttributeChanges;
    // Nodes whose text changed, and elements whose ignored state needs working out again.
    ListHashSet<boo::Node *> _deferredTextChangedList;
    ListHashSet<boo::Element *> _deferredRecomputeIsIgnoredList;
    ListHashSet<boo::Element *> _deferredSelectedChildrenChangedList;
    // A focus change waiting for the newly focused element's renderer: the old and new focused elements.
    Maybe<Pair<RefPtr<boo::Element>, RefPtr<boo::Element>>> _deferredFocusedNodeChange;
    // Notifications for objects that may not exist yet, posted once the tree is up to date.
    List<Pair<Ref<boo::Node>, AXNotification>> _deferredNotifications;
    ListHashSet<AXId> _deferredRendererChangedList;
    // Tables whose exposed state and whose cell slots need working out again.
    ListHashSet<AXId> _deferredRecomputeTableIsExposedList;
    ListHashSet<AXId> _deferredRecomputeTableCellSlotsList;
    Timer _performCacheUpdateTimer;
    bool _performingDeferredCacheUpdate {false};

    // The modal dialogs (modal `dialog` elements and dialogs with `aria-modal="true"`), found when first
    // needed; the one that's current, whose outside is ignored; and the dialogs that may have become modal or
    // stopped being.
    List<boo::Element *> _modalElements;
    bool _modalNodesInitialized {false};
    boo::Element *_currentModalElement {nullptr};
    bool _isRetrievingCurrentModalNode {false};
    ListHashSet<boo::Element *> _deferredModalChangedList;

  public:
    explicit AXObjectCache(boo::Document &document) noexcept;

    ~AXObjectCache() noexcept;

    /// @brief Turns accessibility on: documents create their caches when next asked.
    static void EnableAccessibility() noexcept;

    /// @brief Turns accessibility off: documents create no new caches (existing ones stay).
    static void DisableAccessibility() noexcept;

    KRYS_NODISCARD static bool AccessibilityEnabled() noexcept;

    KRYS_NODISCARD boo::Document &GetDocument() const noexcept
    {
      return _document;
    }

    /// @brief The root of the tree: the document's view (above its web area), or its web area if it has no
    /// view.
    KRYS_NODISCARD AXObject *RootObject() noexcept;

    /// @brief The modal dialog the user is in, outside which everything is ignored; null if there isn't one.
    /// It's the active modal `dialog`; otherwise the deepest `aria-modal` dialog with the focus in it, if
    /// it's visible and has something to expose.
    KRYS_NODISCARD boo::Node *ModalNode() noexcept;

    /// @brief Whether the cache is working out which dialog is the modal one (when whether an object is
    /// ignored mustn't depend on it).
    KRYS_NODISCARD bool IsRetrievingCurrentModalNode() const noexcept
    {
      return _isRetrievingCurrentModalNode;
    }

    /// @brief Whether objects' ignored states are being cached (while a tree is being built).
    KRYS_NODISCARD bool IsCachingComputedIgnoredStates() const noexcept
    {
      return _computedIgnoredStates.has_value();
    }

    /// @brief Starts caching objects' ignored states, until stopped or the tree changes.
    void StartCachingComputedObjectAttributesUntilTreeMutates() noexcept
    {
      _computedIgnoredStates.emplace();
    }

    void StopCachingComputedObjectAttributes() noexcept
    {
      _computedIgnoredStates.reset();
    }

    /// @brief The cached ignored state of the object with @p id, if it's cached.
    KRYS_NODISCARD Maybe<bool> CachedIgnoredState(AXId id) const noexcept;

    /// @brief Caches the ignored state of the object with @p id, if caching.
    void CacheIgnoredState(AXId id, bool ignored) noexcept;

    /// @brief The object for the focused element (or its active descendant), or the nearest unignored
    /// ancestor if that object is ignored; the web area if nothing is focused.
    KRYS_NODISCARD AXObject *FocusedObject() noexcept;

    // Lookup and creation.

    KRYS_NODISCARD AXObject *Get(const boo::Node &node) const noexcept;

    KRYS_NODISCARD AXObject *Get(const RenderObject &renderer) const noexcept;

    KRYS_NODISCARD AXObject *ObjectForID(AXId id) const noexcept;

    /// @brief The object for @p node, created if it needs one: if it's rendered, or if it isn't but
    /// accessibility needs it anyway (`display: contents`, canvas fallback content, a focused node,
    /// @p isPartOfRelation).
    AXObject *GetOrCreate(boo::Node &node, IsPartOfRelation isPartOfRelation = IsPartOfRelation::No) noexcept;

    AXObject *GetOrCreate(boo::Node *node, IsPartOfRelation isPartOfRelation = IsPartOfRelation::No) noexcept
    {
      return node ? GetOrCreate(*node, isPartOfRelation) : nullptr;
    }

    /// @brief The object for @p renderer (its node's, if it has one), created if needed.
    AXObject *GetOrCreate(RenderObject &renderer) noexcept;

    AXObject *GetOrCreate(RenderObject *renderer) noexcept
    {
      return renderer ? GetOrCreate(*renderer) : nullptr;
    }

    /// @brief The object for the document's view (its scroll view), made if needed.
    AXObject *GetOrCreate(LocalFrameView &view) noexcept;

    /// @brief A new object for one of the view's scroll bars; its maker sets its parent.
    AXObject *Create(Scrollbar &scrollbar) noexcept;

    /// @brief A new object with no node or renderer of its own, for one of the roles that has one (a slider's
    /// thumb, a menu list's pop-up); null for the others. Its maker sets its parent.
    AXObject *Create(AXRole role) noexcept;

    /// @brief Removes and detaches @p node's object.
    void Remove(boo::Node &node) noexcept;

    /// @brief Removes and detaches @p renderer's object if it has no node; otherwise the node's object
    /// just loses its renderer.
    void Remove(RenderObject &renderer) noexcept;

    void Remove(AXId id) noexcept;

    // Changes.

    /// @brief @p node's children changed; for a rendered node, the renderer's children-changed covers it.
    void ChildrenChanged(boo::Node &node) noexcept;

    void ChildrenChanged(RenderObject &renderer, RenderObject *newChild = nullptr) noexcept;

    void ChildrenChanged(AXObject *object) noexcept;

    /// @brief Tells the platform @p object's role changed from @p previousRole.
    void HandleRoleChanged(AXObject &object, AXRole previousRole) noexcept;

    void RecomputeIsIgnored(boo::Node &node) noexcept;

    void RecomputeIsIgnored(boo::Node *node) noexcept
    {
      if (node)
      {
        RecomputeIsIgnored(*node);
      }
    }

    void RecomputeIsIgnored(RenderObject &renderer) noexcept;

    /// @brief Works out again (once the tree is next updated) whether @p element's object is ignored.
    void DeferRecomputeIsIgnored(boo::Element *element) noexcept;

    /// @brief As above, now if @p element's renderer is up to date.
    void DeferRecomputeIsIgnoredIfNeeded(boo::Element *element) noexcept;

    /// @brief An attribute of @p element changed from @p oldValue to @p newValue (null when it was or is now
    /// absent): handled now, or once the tree is next updated if the element's renderer is out of date.
    void DeferAttributeChangeIfNeeded(boo::Element &element, const DOMStringAtom &name,
                                      Maybe<DOMStringView> oldValue, Maybe<DOMStringView> newValue) noexcept;

    /// @brief The text of @p object (or under it) changed: its name and its ancestors' may have too.
    void HandleTextChanged(AXObject *object) noexcept;

    /// @brief As above for @p node's object, now if its renderer is up to date.
    void DeferTextChangedIfNeeded(boo::Node *node) noexcept;

    // Form controls and selection.

    void CheckedStateChanged(boo::Element &element) noexcept;

    void ValueChanged(boo::Element &element) noexcept;

    /// @brief @p element (an option, a cell, or an item in something with selectable children) was selected
    /// or deselected.
    void OnSelectedOptionChanged(boo::Element &element) noexcept;

    void SelectedChildrenChanged(boo::Node *node) noexcept;

    void SelectedChildrenChanged(RenderObject *renderer) noexcept;

    /// @brief As above for a select, now if its renderer is up to date.
    void DeferSelectedChildrenChangedIfNeeded(boo::Element &select) noexcept;

    /// @brief A menu list's active option changed to the one at @p optionIndex.
    void OnSelectedOptionChanged(RenderObject &menuList, int32 optionIndex) noexcept;

    /// @brief The value of the text control (or editable content) @p node is in was edited.
    void OnEditableTextValueChanged(boo::Node &node) noexcept;

    /// @brief Whether @p element is valid changed.
    void OnValidityChange(boo::Element &element) noexcept;

    /// @brief The outermost element editable to assistive technology at or above @p node: its root editable
    /// element, or the outermost text control (an ARIA text box too) above it; null if there's neither.
    KRYS_NODISCARD boo::Element *RootAXEditableElement(const boo::Node &node) noexcept;

    /// @brief @p document's title (the web area's name) may have changed.
    void OnTitleChange(boo::Document &document) noexcept;

    /// @brief @p radio joined or left the radio button group of @p group (the other buttons in it).
    void OnRadioGroupMembershipChanged(const boo::Element &radio,
                                       const List<Ref<boo::HTMLInputElement>> &group) noexcept;

    /// @brief The focus moved from @p oldElement to @p newElement (either may be null): handled now, or once
    /// the tree is next updated if the new element's renderer is out of date.
    void OnFocusChange(boo::Element *oldElement, boo::Element *newElement) noexcept;

    /// @brief Brings the tree up to date with the changes deferred since the last update.
    void PerformDeferredCacheUpdate() noexcept;

    // Rendering.

    /// @brief @p node has a new renderer: its object (if it has one) is updated for it.
    void OnRendererCreated(boo::Node &node) noexcept;

    /// @brief @p element's style changed from @p oldStyle to @p newStyle.
    void OnStyleChange(boo::Element &element, const RenderStyle *oldStyle,
                       const RenderStyle *newStyle) noexcept;

    /// @brief @p renderer became inert or not, or visible or not: its parent's children change.
    void OnInertOrVisibilityChange(RenderElement &renderer) noexcept;

    /// @brief A layout of @p layoutRoot's subtree finished.
    void OnLayoutComplete(RenderObject &layoutRoot) noexcept;

    /// @brief The document finished loading.
    void OnDocumentLoaded() noexcept;

    /// @brief The view scrolled to @p anchor (the fragment's target): its first unignored object is told.
    void HandleScrolledToAnchor(boo::Node &anchor) noexcept;

    // DOM.

    /// @brief The nodes assigned to @p slot changed.
    void OnSlottedContentChange(boo::Element &slot) noexcept;

    /// @brief The popover was shown or hidden: the elements that invoke it are told they expanded or
    /// collapsed.
    void OnPopoverToggle(boo::Element &popover) noexcept;

    /// @brief The document's design mode was turned on or off.
    void HandlePageEditabilityChanged() noexcept;

    /// @brief The element being dragged changed from @p oldElement to @p newElement.
    void OnDragElementChanged(boo::Element *oldElement, boo::Element *newElement) noexcept;

    // Relations.

    /// @brief The objects @p object is related to by @p relation (null if none), with the relations brought
    /// up to date first.
    KRYS_NODISCARD Maybe<ListHashSet<AXId>> RelatedObjectIDsFor(const AXObject &object,
                                                                AXRelation relation) noexcept;

    /// @brief @p origin's @p attribute (a relation attribute) changed its targets: the relation is set up
    /// again.
    void UpdateRelations(boo::Element &origin, DOMStringView attribute) noexcept;

    /// @brief The relations need working out again (an ID changed, so attributes may refer to other
    /// elements).
    void RelationsNeedUpdate() noexcept
    {
      _relationsNeedUpdate = true;
    }

    // Text state changes.

    /// @brief @p text was inserted or deleted in the editable region @p node, ending (for an insertion) or
    /// starting (for a deletion) at @p position: the host is told, for the object that tells it about the
    /// region's changes (the web area's if none does), with the change's offset in that object's text.
    void PostTextStateChangeNotification(boo::Node *node, AXTextChange change, DOMStringView text,
                                         const VisiblePosition &position) noexcept;

    /// @brief @p deletedText was replaced by @p insertedText, which ends at @p position, in the editable
    /// region
    /// @p node.
    void PostTextReplacementNotification(boo::Node *node, DOMStringView deletedText,
                                         DOMStringView insertedText,
                                         const VisiblePosition &position) noexcept;

    /// @brief The selection (starting at @p start) changed: `SelectedTextChanged` is posted for the object
    /// that tells the host about the changes where it is (the web area's if none does).
    void PostTextSelectionChangeNotification(const dom::Position &start) noexcept;

    // Notifications.

    /// @brief Queues @p notification for @p object, to post to the platform once the current task ends.
    void PostNotification(AXObject *object, AXNotification notification) noexcept;

    void PostNotification(boo::Node *node, AXNotification notification) noexcept;

    void PostNotification(RenderObject *renderer, AXNotification notification) noexcept;

  private:
    KRYS_NODISCARD AXId GenerateID() noexcept
    {
      return static_cast<AXId>(_nextID++);
    }

    KRYS_NODISCARD Ref<AXRenderObject> CreateObjectFromRenderer(RenderObject &renderer) noexcept;

    KRYS_NODISCARD Ref<AXNodeObject> CreateFromNode(boo::Node &node) noexcept;

    void CacheAndInitializeWrapper(AXObject &object, const boo::Node *node,
                                   const RenderObject *renderer) noexcept;

    AXObject *GetOrCreateSlow(boo::Node &node, IsPartOfRelation isPartOfRelation) noexcept;

    KRYS_NODISCARD AXObject *FocusedObjectForNode(boo::Node &node) noexcept;

    /// @brief A focused area's object: its image's child for it.
    KRYS_NODISCARD AXObject *FocusedImageMapUIElement(boo::HTMLAreaElement &area) noexcept;

    KRYS_NODISCARD static AXRelation AttributeToRelationType(DOMStringView attribute) noexcept;

    KRYS_NODISCARD static AXRelation SymmetricRelation(AXRelation relation) noexcept;

    KRYS_NODISCARD static bool IsRelationAttribute(DOMStringView attribute) noexcept;

    bool AddRelation(boo::Element &origin, boo::Element &target, AXRelation relation) noexcept;

    bool AddRelation(AXObject *origin, AXObject *target, AXRelation relation,
                     bool addSymmetricRelation = true) noexcept;

    /// @brief Sets up the relation @p origin's @p attribute gives it to the elements it refers to.
    bool AddRelation(boo::Element &origin, DOMStringView attribute) noexcept;

    /// @brief A label's relation to its control.
    bool AddLabelForRelation(boo::Element &origin) noexcept;

    bool RemoveRelation(boo::Element &origin, AXRelation relation) noexcept;

    void RemoveAllRelations(AXId id) noexcept;

    void RemoveRelationByID(AXId originID, AXId targetID, AXRelation relation) noexcept;

    void UpdateRelationsIfNeeded() noexcept;

    void UpdateRelationsForTree(boo::ContainerNode &root) noexcept;

    /// @brief Whether an ancestor of @p node is related to something (so @p node may be named by it, and
    /// needs an object).
    KRYS_NODISCARD bool IsDescendantOfRelatedNode(boo::Node &node) noexcept;

    /// @brief A label's control changed: the label's relation is set up again, unless the control is labelled
    /// by ARIA (which takes precedence).
    bool UpdateLabelFor(boo::Element &label) noexcept;

    void HandleActiveDescendantChange(boo::Element &element) noexcept;

    void HandleTabPanelSelected(boo::Element *oldElement, boo::Element *newElement) noexcept;

    KRYS_NODISCARD bool ShouldProcessAttributeChange(boo::Element &element,
                                                     DOMStringView name) const noexcept;

    void HandleAttributeChange(boo::Element &element, DOMStringView name, DOMStringView oldValue,
                               DOMStringView newValue) noexcept;

    /// @brief The `role` attribute changed: an object whose kind depends on it is made again.
    void HandleRoleChanged(boo::Element &element, DOMStringView oldValue, DOMStringView newValue) noexcept;

    void HandleAriaExpandedChange(boo::Element &element) noexcept;

    void HandleRowCountChanged(AXObject *object) noexcept;

    void HandleMenuItemSelected(boo::Element *element) noexcept;

    /// @brief Whether a focus change works out the modal dialog again (not when more changes that may change
    /// it are to follow).
    enum class UpdateModal : bool
    {
      No,
      Yes,
    };

    void HandleFocusedUIElementChanged(boo::Element *oldElement, boo::Element *newElement,
                                       UpdateModal updateModal = UpdateModal::Yes) noexcept;

    KRYS_NODISCARD static bool IsModalElement(const boo::Element &element) noexcept;

    void FindModalNodes() noexcept;

    /// @brief Whether the modal @p element has anything unignored in it (so it doesn't trap the user in
    /// nothing).
    KRYS_NODISCARD bool ModalElementHasAccessibleContent(boo::Element &element) noexcept;

    void UpdateCurrentModalNode() noexcept;

    /// @brief Whether @p node is an element that's shown: rendered, not hidden, not transparent and not
    /// `aria-hidden` (unless it's focused).
    KRYS_NODISCARD static bool IsNodeVisible(const boo::Node *node) noexcept;

    void DeferModalChange(boo::Element &element) noexcept;

    void HandleChildrenChanged(AXObject &object) noexcept;

    /// @brief The object that tells the host about @p node's changes; the web area's if none does.
    KRYS_NODISCARD AXObject *ObservableObjectForNode(boo::Node *node) noexcept;

    void DeferElementAddedOrRemoved(boo::Element *element) noexcept;

    void HandleMenuOpened(boo::Element &element) noexcept;

    void HandleLiveRegionCreated(boo::Element &element) noexcept;

    /// @brief A label (or something labelling an object) changed: the objects it labels are told.
    void HandleLabelChanged(AXObject *object) noexcept;

    void PostLiveRegionChangeNotification(AXObject &object) noexcept;

    void HandleAllDeferredChildrenChanged() noexcept;

    /// @brief Works out again (later) whether @p element's table is exposed: adding or removing rows can make
    /// a layout table a data table or the other way round.
    void DeferRecomputeTableIsExposed(boo::Element *element) noexcept;

    void DeferRecomputeTableCellSlots(AXNodeObject &table) noexcept;

    void StartPerformCacheUpdateTimer() noexcept;

    void NotificationPostTimerFired() noexcept;

    void PerformCacheUpdateTimerFired() noexcept
    {
      PerformDeferredCacheUpdate();
    }

    // The platform's side: the host's accessibility bridge.
    void DetachWrapper(AXObject &object, AXDetachmentType detachmentType) noexcept;

    void PostPlatformNotification(AXObject &object, AXNotification notification) noexcept;

    friend class AXObject;
  };

  /// @brief The text an edit replaces (the selection it started with), so inserting the edit's text can be
  /// posted as a replacement.
  /// @brief Caches objects' ignored states for its lifetime (unless the tree changes first), if the cache
  /// wasn't already: while a tree is built.
  class AXAttributeCacheScope final
  {
    KRYS_NON_COPY_MOVABLE(AXAttributeCacheScope);

  private:
    // (The scope is shorter than the cache's life, like an object's.)
    AXObjectCache *_cache;
    bool _wasAlreadyCaching {false};

  public:
    explicit AXAttributeCacheScope(AXObjectCache *cache) noexcept : _cache(cache)
    {
      if (cache)
      {
        _wasAlreadyCaching = cache->IsCachingComputedIgnoredStates();
        if (!_wasAlreadyCaching)
        {
          cache->StartCachingComputedObjectAttributesUntilTreeMutates();
        }
      }
    }

    ~AXAttributeCacheScope() noexcept
    {
      if (_cache && !_wasAlreadyCaching)
      {
        _cache->StopCachingComputedObjectAttributes();
      }
    }
  };

  class AXReplacedText
  {
  private:
    DOMString _replacedText;
    VisiblePositionIndexRange _replacedRange;

  public:
    AXReplacedText() noexcept = default;

    explicit AXReplacedText(const VisibleSelection &selection) noexcept;

    /// @brief @p insertedText replaced the text (if there was any), leaving @p selection.
    void PostTextStateChangeNotification(AXObjectCache *cache, DOMStringView insertedText,
                                         const VisibleSelection &selection) const noexcept;

    KRYS_NODISCARD const VisiblePositionIndexRange &ReplacedRange() const noexcept
    {
      return _replacedRange;
    }
  };
}
