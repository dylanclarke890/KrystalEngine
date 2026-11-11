/*
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (C) 2008, 2009, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nuanti Ltd.
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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityRole.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class RenderObject;
  class RenderStyle;
  class LocalFrameView;
  class Page;

  namespace dom
  {
    class Document;
    class Element;
    class Node;
  }
}

namespace krys::boo::ax
{
  class AXObjectCache;

  /// @brief An accessibility object's identifier, unique within its cache.
  enum class AXID : uint64
  {
  };

  /// @brief Why an accessibility object is being detached from its cache.
  enum class AccessibilityDetachmentType : uint8
  {
    CacheDestroyed,
    ElementDestroyed,
    ElementChanged,
  };

  /// @brief Whether an object must be in the tree, must be left out, or is left to the usual rules.
  enum class AccessibilityObjectInclusion : uint8
  {
    IncludeObject,
    IgnoreObject,
    DefaultBehavior,
  };

  /// @brief A checkbox's, radio button's or toggle button's state.
  enum class AccessibilityButtonState : uint8
  {
    Off,
    On,
    Mixed,
  };

  /// @brief Which way a slider, scroll bar, list and so on runs.
  enum class AccessibilityOrientation : uint8
  {
    Undefined,
    Vertical,
    Horizontal,
  };

  /// @brief How one object relates to another (each relation has its reverse, such as `LabeledBy` and
  /// `LabelFor`): set up by ARIA's relation attributes, a label's control, and a cell's headers.
  enum class AXRelation : uint8
  {
    None,
    ActiveDescendant,
    ActiveDescendantOf,
    ControlledBy,
    ControllerFor,
    DescribedBy,
    DescriptionFor,
    Details,
    DetailsFor,
    ErrorMessage,
    ErrorMessageFor,
    FlowsFrom,
    FlowsTo,
    Headers,
    HeaderFor,
    LabeledBy,
    LabelFor,
    OwnedBy,
    OwnerFor,
  };

  class AccessibilityObject;
  /// @brief An mmultiscripts element's scripts, as subscript and superscript pairs.
  using AccessibilityMathMultiscriptPairs = List<Pair<AccessibilityObject *, AccessibilityObject *>>;

  /// @brief An object in the accessibility tree: what assistive technology sees of a node or renderer.
  /// Objects are created and owned by an `AXObjectCache`; an object's children are the unignored objects
  /// below it, with each ignored object replaced by its own children.
  /// @note WebKit's `AXCoreObject` and `AccessibilityObject` are one class here: there's no isolated tree
  /// for `AXCoreObject` to abstract over.
  class AccessibilityObject : public RefCounted<AccessibilityObject>,
                              public CanMakeWeakPtr<AccessibilityObject>
  {
    friend class AXObjectCache;

  public:
    using AccessibilityChildrenVector = List<Ref<AccessibilityObject>>;

    /// @brief Whether `InsertChild` replaces an ignored child with the child's own children.
    enum class DescendIfIgnored : bool
    {
      No,
      Yes,
    };

  private:
    AXID _id;
    AXObjectCache *_axObjectCache;

  protected:
    AccessibilityRole _role {AccessibilityRole::Unknown};
    AccessibilityObjectInclusion _lastKnownIsIgnoredValue {AccessibilityObjectInclusion::DefaultBehavior};
    AccessibilityChildrenVector _children;
    uint32 _indexInParent {0};
    bool _childrenInitialized {false};
    bool _childrenDirty {false};
    bool _subtreeDirty {false};

  public:
    virtual ~AccessibilityObject() noexcept;

    /// @brief Sets the object up once its cache holds it: its role, then anything that depends on it.
    virtual void Init() noexcept;

    KRYS_NODISCARD AXID ObjectID() const noexcept
    {
      return _id;
    }
    KRYS_NODISCARD AccessibilityRole Role() const noexcept
    {
      return _role;
    }
    /// @brief The cache holding the object; null once it's detached.
    KRYS_NODISCARD AXObjectCache *AxObjectCache() const noexcept
    {
      return _axObjectCache;
    }

    /// @brief Removes the object from the tree: clears its children, lets go of its node and renderer and
    /// tells the platform its wrapper is gone.
    void Detach(AccessibilityDetachmentType detachmentType) noexcept;
    KRYS_NODISCARD virtual bool IsDetached() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsAccessibilityNodeObject() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsAccessibilityRenderObject() const noexcept
    {
      return false;
    }
    /// @brief Whether the object stands for no node or renderer of its own (a slider's thumb, a menu list's
    /// pop-up), its parent being set by whoever made it.
    KRYS_NODISCARD virtual bool IsMockObject() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMenuList() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMenuListPopup() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMenuListOption() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsListBoxOption() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsScrollView() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsAccessibilitySVGObject() const noexcept
    {
      return false;
    }
    /// @brief Whether this is an `svg` root's object.
    KRYS_NODISCARD virtual bool IsAccessibilitySVGRoot() const noexcept
    {
      return false;
    }
    /// @brief Whether this is a MathML element's object.
    KRYS_NODISCARD virtual bool IsMathElement() const noexcept
    {
      return false;
    }

    /// @brief A description for debugging: the role, ID, and the renderer or node.
    KRYS_NODISCARD utf8_string DebugDescription() const noexcept;

    // The DOM and the render tree.

    KRYS_NODISCARD virtual dom::Node *GetNode() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD dom::Element *GetElement() const noexcept;
    KRYS_NODISCARD virtual RenderObject *Renderer() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual dom::Document *GetDocument() const noexcept;
    KRYS_NODISCARD Page *GetPage() const noexcept;
    KRYS_NODISCARD LocalFrameView *DocumentFrameView() const noexcept;
    /// @brief The style of the object's renderer, or else its element's computed style.
    KRYS_NODISCARD const RenderStyle *Style() const noexcept;

    // The tree.

    KRYS_NODISCARD virtual AccessibilityObject *ParentObject() const noexcept = 0;
    /// @brief The nearest ancestor that isn't ignored: the object's parent in the tree.
    KRYS_NODISCARD AccessibilityObject *ParentObjectUnignored() const noexcept;
    /// @brief The object's children, added first if they're out of date and @p updateChildrenIfNeeded.
    KRYS_NODISCARD const AccessibilityChildrenVector &Children(bool updateChildrenIfNeeded = true) noexcept;
    KRYS_NODISCARD bool ChildrenInitialized() const noexcept
    {
      return _childrenInitialized;
    }
    KRYS_NODISCARD uint32 IndexInParent() const noexcept
    {
      return _indexInParent;
    }
    KRYS_NODISCARD bool HasDirtySubtree() const noexcept
    {
      return _subtreeDirty;
    }

    virtual void AddChildren() noexcept
    {
    }
    KRYS_NODISCARD virtual bool CanHaveChildren() const noexcept
    {
      return true;
    }
    virtual void UpdateChildrenIfNecessary() noexcept;
    /// @brief Marks the children out of date, so the next `Children()` adds them again.
    virtual void SetNeedsToUpdateChildren() noexcept
    {
    }
    /// @brief Marks the children and their subtrees out of date.
    virtual void SetNeedsToUpdateSubtree() noexcept
    {
    }
    KRYS_NODISCARD virtual bool NeedsToUpdateChildren() const noexcept
    {
      return false;
    }
    virtual void ClearChildren() noexcept;
    /// @brief Forgets the parent a mock object was given (its parent is clearing its children).
    virtual void DetachFromParent() noexcept
    {
    }

    /// @brief Inserts @p child at @p index; an ignored child's children instead if @p descendIfIgnored.
    void InsertChild(AccessibilityObject &child, uint32 index,
                     DescendIfIgnored descendIfIgnored = DescendIfIgnored::Yes) noexcept;
    void AddChild(AccessibilityObject *child,
                  DescendIfIgnored descendIfIgnored = DescendIfIgnored::Yes) noexcept
    {
      if (child)
      {
        InsertChild(*child, static_cast<uint32>(_children.size()), descendIfIgnored);
      }
    }

    KRYS_NODISCARD bool IsDescendantOfObject(const AccessibilityObject &ancestor) const noexcept;
    KRYS_NODISCARD bool IsAncestorOfObject(const AccessibilityObject &descendant) const noexcept;
    KRYS_NODISCARD bool IsDescendantOfRole(AccessibilityRole role) const noexcept;

    // Whether the object is in the tree.

    /// @brief Whether the object is left out of the tree (its children take its place).
    KRYS_NODISCARD bool IsIgnored() const noexcept;
    /// @brief Works out `IsIgnored()` again, telling the parent its children changed if it did.
    void RecomputeIsIgnored() noexcept
    {
      (void)IsIgnored();
    }
    void RecomputeIsIgnoredForDescendants(bool includeSelf = false) noexcept;
    /// @brief The common reasons to include or ignore an object (inertness, hiddenness, a presentational
    /// ancestor), before the rules for its kind.
    KRYS_NODISCARD AccessibilityObjectInclusion DefaultObjectInclusion() const noexcept;
    KRYS_NODISCARD bool IsIgnoredByDefault() const noexcept
    {
      return DefaultObjectInclusion() == AccessibilityObjectInclusion::IgnoreObject;
    }
    /// @brief Whether `aria-hidden="true"` applies to the object itself.
    KRYS_NODISCARD bool IsARIAHidden() const noexcept;
    /// @brief Whether `aria-hidden="true"` applies to the object or an ancestor.
    KRYS_NODISCARD bool IsAXHidden() const noexcept;

    // Roles.

    KRYS_NODISCARD virtual AccessibilityRole AriaRoleAttribute() const noexcept
    {
      return AccessibilityRole::Unknown;
    }
    KRYS_NODISCARD bool HasExplicitGenericRole() const noexcept
    {
      return AriaRoleAttribute() == AccessibilityRole::Generic;
    }
    /// @brief Works out the role again, telling the cache if it changed.
    void UpdateRole() noexcept;
    /// @brief The first role in @p value (a space-separated list of ARIA roles) that isn't skipped.
    KRYS_NODISCARD static AccessibilityRole AriaRoleToRole(dom::DOMStringView value) noexcept;
    KRYS_NODISCARD static AccessibilityRole
      AriaRoleToRole(dom::DOMStringView value, const Func<bool(AccessibilityRole)> &skipRole) noexcept;
    /// @brief The ARIA role the object maps to (`computedrole` in WebDriver), empty for none.
    KRYS_NODISCARD utf8_stringview ComputedRoleString() const noexcept;
    /// @brief Whether the role makes its descendants presentational (ARIA "children presentational").
    KRYS_NODISCARD bool AriaRoleHasPresentationalChildren() const noexcept;
    /// @brief Whether the role is one of those ARIA defines as a widget an author may make (button,
    /// checkbox and so on).
    KRYS_NODISCARD static bool IsARIAControl(AccessibilityRole role) noexcept;

    KRYS_NODISCARD bool IsHeading() const noexcept
    {
      return _role == AccessibilityRole::Heading;
    }
    KRYS_NODISCARD bool IsLink() const noexcept
    {
      return _role == AccessibilityRole::Link;
    }
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return _role == AccessibilityRole::Image;
    }
    KRYS_NODISCARD bool IsWebArea() const noexcept
    {
      return _role == AccessibilityRole::WebArea;
    }
    KRYS_NODISCARD bool IsCheckbox() const noexcept
    {
      return _role == AccessibilityRole::Checkbox;
    }
    KRYS_NODISCARD bool IsRadioButton() const noexcept
    {
      return _role == AccessibilityRole::RadioButton;
    }
    KRYS_NODISCARD bool IsCheckboxOrRadio() const noexcept
    {
      return IsCheckbox() || IsRadioButton();
    }
    KRYS_NODISCARD bool IsSwitch() const noexcept
    {
      return _role == AccessibilityRole::Switch;
    }
    KRYS_NODISCARD bool IsToggleButton() const noexcept
    {
      return _role == AccessibilityRole::ToggleButton;
    }
    KRYS_NODISCARD bool IsListBox() const noexcept
    {
      return _role == AccessibilityRole::ListBox;
    }
    KRYS_NODISCARD bool IsListItem() const noexcept
    {
      return _role == AccessibilityRole::ListItem;
    }
    KRYS_NODISCARD bool IsMenu() const noexcept
    {
      return _role == AccessibilityRole::Menu;
    }
    KRYS_NODISCARD bool IsMenuBar() const noexcept
    {
      return _role == AccessibilityRole::MenuBar;
    }
    KRYS_NODISCARD bool IsComboBox() const noexcept
    {
      return _role == AccessibilityRole::ComboBox;
    }
    KRYS_NODISCARD bool IsTree() const noexcept
    {
      return _role == AccessibilityRole::Tree;
    }
    KRYS_NODISCARD bool IsStaticText() const noexcept
    {
      return _role == AccessibilityRole::StaticText;
    }
    KRYS_NODISCARD bool IsSummary() const noexcept
    {
      return _role == AccessibilityRole::Summary;
    }
    KRYS_NODISCARD bool IsProgressIndicator() const noexcept
    {
      return _role == AccessibilityRole::ProgressIndicator || _role == AccessibilityRole::Meter;
    }
    KRYS_NODISCARD bool IsSlider() const noexcept
    {
      return _role == AccessibilityRole::Slider;
    }
    KRYS_NODISCARD bool IsSpinButton() const noexcept
    {
      return _role == AccessibilityRole::SpinButton;
    }
    KRYS_NODISCARD bool IsList() const noexcept
    {
      return _role == AccessibilityRole::List || _role == AccessibilityRole::DescriptionList;
    }
    KRYS_NODISCARD bool IsMenuItem() const noexcept;
    KRYS_NODISCARD bool IsMenuRelated() const noexcept;
    KRYS_NODISCARD bool IsButton() const noexcept;
    KRYS_NODISCARD bool IsControl() const noexcept;
    KRYS_NODISCARD bool IsTextControl() const noexcept;
    /// @brief Whether the object edits text without being an `input` or `textarea` (`contenteditable`, or a
    /// text box role).
    KRYS_NODISCARD virtual bool IsNonNativeTextControl() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsGroup() const noexcept;
    KRYS_NODISCARD bool IsLandmark() const noexcept;
    KRYS_NODISCARD bool IsRangeControl() const noexcept;
    /// @brief Whether the object is an image map's `area`.
    KRYS_NODISCARD bool IsImageMapLink() const noexcept;
    KRYS_NODISCARD bool IsRootWebArea() const noexcept;
    // Relations.

    /// @brief The objects this one is related to by @p relation.
    KRYS_NODISCARD AccessibilityChildrenVector RelatedObjects(AXRelation relation) const noexcept;
    /// @brief The object `aria-activedescendant` makes the active one, if any.
    KRYS_NODISCARD AccessibilityObject *ActiveDescendant() const noexcept;
    KRYS_NODISCARD AccessibilityChildrenVector ActiveDescendantOfObjects() const noexcept
    {
      return RelatedObjects(AXRelation::ActiveDescendantOf);
    }
    KRYS_NODISCARD AccessibilityChildrenVector ControlledObjects() const noexcept
    {
      return RelatedObjects(AXRelation::ControllerFor);
    }
    KRYS_NODISCARD AccessibilityChildrenVector Controllers() const noexcept
    {
      return RelatedObjects(AXRelation::ControlledBy);
    }
    KRYS_NODISCARD AccessibilityChildrenVector DescribedByObjects() const noexcept
    {
      return RelatedObjects(AXRelation::DescribedBy);
    }
    KRYS_NODISCARD AccessibilityChildrenVector DescriptionForObjects() const noexcept
    {
      return RelatedObjects(AXRelation::DescriptionFor);
    }
    KRYS_NODISCARD AccessibilityChildrenVector DetailedByObjects() const noexcept
    {
      return RelatedObjects(AXRelation::Details);
    }
    KRYS_NODISCARD AccessibilityChildrenVector DetailsForObjects() const noexcept
    {
      return RelatedObjects(AXRelation::DetailsFor);
    }
    KRYS_NODISCARD AccessibilityChildrenVector ErrorMessageObjects() const noexcept
    {
      return RelatedObjects(AXRelation::ErrorMessage);
    }
    KRYS_NODISCARD AccessibilityChildrenVector ErrorMessageForObjects() const noexcept
    {
      return RelatedObjects(AXRelation::ErrorMessageFor);
    }
    KRYS_NODISCARD AccessibilityChildrenVector FlowToObjects() const noexcept
    {
      return RelatedObjects(AXRelation::FlowsTo);
    }
    KRYS_NODISCARD AccessibilityChildrenVector FlowFromObjects() const noexcept
    {
      return RelatedObjects(AXRelation::FlowsFrom);
    }
    KRYS_NODISCARD AccessibilityChildrenVector LabeledByObjects() const noexcept
    {
      return RelatedObjects(AXRelation::LabeledBy);
    }
    KRYS_NODISCARD AccessibilityChildrenVector LabelForObjects() const noexcept
    {
      return RelatedObjects(AXRelation::LabelFor);
    }
    KRYS_NODISCARD AccessibilityChildrenVector OwnedObjects() const noexcept
    {
      return RelatedObjects(AXRelation::OwnerFor);
    }
    KRYS_NODISCARD AccessibilityChildrenVector Owners() const noexcept
    {
      return RelatedObjects(AXRelation::OwnedBy);
    }
    /// @brief Whether the focus is on the active descendant when the object is focused (by its ARIA role).
    KRYS_NODISCARD bool ShouldFocusActiveDescendant() const noexcept;
    /// @brief Whether the role is one that can have an active descendant.
    KRYS_NODISCARD bool SupportsActiveDescendant() const noexcept;

    /// @brief Whether the role is one whose children can be selected (with `aria-selected`, or as the focused
    /// or active descendant).
    KRYS_NODISCARD bool CanHaveSelectedChildren() const noexcept;
    /// @brief Whether the role is one whose row count can change (by expanding or collapsing rows).
    KRYS_NODISCARD bool SupportsRowCountChange() const noexcept;
    /// @brief Whether the object's name comes from the text under it, so it changes with that text.
    KRYS_NODISCARD bool DependsOnTextUnderElement() const noexcept;

    // Attributes.

    /// @brief @p element's @p name attribute, or (for ARIA attributes) its custom element's default; empty if
    /// neither is set.
    KRYS_NODISCARD static dom::DOMString AttributeWithDefaultARIA(const dom::Element &element,
                                                                  dom::DOMStringView name) noexcept;
    /// @brief Whether the element has @p name, or (for ARIA attributes) its custom element has a default.
    KRYS_NODISCARD bool HasAttribute(dom::DOMStringView name) const noexcept;
    /// @brief The element's @p name attribute, or (for ARIA attributes) its custom element's default;
    /// empty if neither is set.
    KRYS_NODISCARD dom::DOMString GetAttribute(dom::DOMStringView name) const noexcept;
    /// @brief `GetAttribute` with leading and trailing ASCII whitespace stripped.
    KRYS_NODISCARD dom::DOMString GetAttributeTrimmed(dom::DOMStringView name) const noexcept;
    /// @brief `GetAttribute` as an integer, 0 if it isn't one.
    KRYS_NODISCARD int32 IntegralAttribute(dom::DOMStringView name) const noexcept;
    /// @brief Whether the element has any global ARIA state or property.
    KRYS_NODISCARD bool SupportsARIAAttributes() const noexcept;

    // Live regions.

    /// @brief The live region status @p role has when `aria-live` isn't set: "assertive" for alerts, "polite"
    /// for logs and statuses, "off" for timers and marquees, and empty for the others.
    KRYS_NODISCARD static dom::DOMStringView DefaultLiveRegionStatusForRole(AccessibilityRole role) noexcept;
    /// @brief Whether @p status makes a region announce its changes ("polite" or "assertive").
    KRYS_NODISCARD static bool LiveRegionStatusIsEnabled(dom::DOMStringView status) noexcept;
    /// @brief `aria-live`, or the role's default.
    KRYS_NODISCARD dom::DOMString LiveRegionStatus() const noexcept;
    /// @brief `aria-relevant`, or its default ("additions text").
    KRYS_NODISCARD dom::DOMString LiveRegionRelevant() const noexcept;
    /// @brief Whether the region is announced as a whole when part of it changes: `aria-atomic`, true by
    /// default for alerts and statuses.
    KRYS_NODISCARD bool LiveRegionAtomic() const noexcept;
    /// @brief Whether `aria-busy` says the object is being updated (so its changes shouldn't be announced
    /// yet).
    KRYS_NODISCARD bool IsBusy() const noexcept;
    /// @brief Whether the object is a live region: one that announces its changes or, unless
    /// @p excludeIfOff, one whose status is "off".
    KRYS_NODISCARD bool SupportsLiveRegion(bool excludeIfOff = true) const noexcept;
    /// @brief The nearest live region at or above the object.
    KRYS_NODISCARD AccessibilityObject *LiveRegionAncestor(bool excludeIfOff = true) const noexcept;

    // States.

    KRYS_NODISCARD virtual bool IsFocused() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsEnabled() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool IsChecked() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsIndeterminate() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsPressed() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsSelected() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsRequired() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMultiSelectable() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsVisited() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool SupportsExpanded() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsExpanded() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool CanSetFocusAttribute() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool CanSetSelectedAttribute() const noexcept
    {
      return false;
    }
    /// @brief Whether a collapsible object (a menu list's pop-up) is collapsed.
    KRYS_NODISCARD virtual bool IsCollapsed() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual AccessibilityButtonState CheckboxOrRadioValue() const noexcept;
    /// @brief The heading level, 1 to 6 for HTML headings or `aria-level` for ARIA ones; 0 if not a heading.
    KRYS_NODISCARD virtual uint32 HeadingLevel() const noexcept
    {
      return 0;
    }

    // Text.

    /// @brief The accessible name, as accname computes it.
    KRYS_NODISCARD virtual dom::DOMString AccessibleName() const noexcept
    {
      return {};
    }
    /// @brief The accessible description, as accname computes it.
    KRYS_NODISCARD virtual dom::DOMString AccessibleDescription() const noexcept
    {
      return {};
    }
    /// @brief The object's value as text (a text control's value, a static text's text and so on).
    KRYS_NODISCARD virtual dom::DOMString StringValue() const noexcept
    {
      return {};
    }

    // Ranges (sliders, progress indicators, meters, scroll bars and spin buttons).

    KRYS_NODISCARD virtual float ValueForRange() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual float MaxValueForRange() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual float MinValueForRange() const noexcept
    {
      return 0;
    }
    /// @brief The value as text (`aria-valuetext`), if the author gave one.
    KRYS_NODISCARD virtual dom::DOMString ValueDescription() const noexcept
    {
      return {};
    }
    /// @brief The orientation the author or the control gives, if any.
    KRYS_NODISCARD virtual Maybe<AccessibilityOrientation> ExplicitOrientation() const noexcept
    {
      return null;
    }
    /// @brief The explicit orientation, or else the role's default.
    KRYS_NODISCARD AccessibilityOrientation Orientation() const noexcept;

    // Tables, rows and cells.

    /// @brief Whether the object is a table (a `table` element, a table renderer or a table role), whether
    /// or not it's exposed as one.
    KRYS_NODISCARD virtual bool IsTable() const noexcept
    {
      return false;
    }
    /// @brief Whether the object is a table that's exposed as one: an ARIA table, or a data table rather
    /// than one for layout.
    KRYS_NODISCARD virtual bool IsExposableTable() const noexcept
    {
      return false;
    }
    /// @brief Whether the object has the `grid`, `table` or `treegrid` role.
    KRYS_NODISCARD virtual bool IsAriaTable() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsTableRow() const noexcept
    {
      return false;
    }
    /// @brief Whether the object is a row of an exposed table.
    KRYS_NODISCARD virtual bool IsExposedTableRow() const noexcept
    {
      return false;
    }
    /// @brief Whether the object has the `row` role.
    KRYS_NODISCARD virtual bool IsARIAGridRow() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsTableCell() const noexcept
    {
      return false;
    }
    /// @brief Whether the object is a cell of an exposed table.
    KRYS_NODISCARD virtual bool IsExposedTableCell() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsColumnHeader() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsRowHeader() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsTableColumn() const noexcept
    {
      return _role == AccessibilityRole::Column;
    }
    KRYS_NODISCARD bool IsTreeGrid() const noexcept
    {
      return _role == AccessibilityRole::TreeGrid;
    }
    KRYS_NODISCARD bool HasGridRole() const noexcept
    {
      return _role == AccessibilityRole::Grid || _role == AccessibilityRole::TreeGrid;
    }
    KRYS_NODISCARD bool HasCellRole() const noexcept;
    KRYS_NODISCARD bool HasCellOrRowRole() const noexcept
    {
      return HasCellRole() || _role == AccessibilityRole::Row;
    }
    /// @brief Whether the object is a `thead`, `tbody` or `tfoot`.
    KRYS_NODISCARD bool HasRowGroupTag() const noexcept;

    /// @brief A row's or cell's table.
    KRYS_NODISCARD virtual AccessibilityObject *ParentTable() const noexcept
    {
      return nullptr;
    }
    /// @brief The nearest exposed table above the object (or the object itself, if @p includeSelf).
    KRYS_NODISCARD AccessibilityObject *ExposedTableAncestor(bool includeSelf = false) const noexcept;
    /// @brief A table's rows.
    KRYS_NODISCARD virtual AccessibilityChildrenVector Rows() noexcept
    {
      return {};
    }
    /// @brief A table's columns (which aren't in the tree).
    KRYS_NODISCARD virtual AccessibilityChildrenVector Columns() noexcept
    {
      return {};
    }
    /// @brief A table's cells, row by row.
    KRYS_NODISCARD virtual AccessibilityChildrenVector Cells() noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual uint32 RowCount() noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual uint32 ColumnCount() noexcept
    {
      return 0;
    }
    /// @brief The table's cell covering the slot at @p column and @p row, if any.
    KRYS_NODISCARD virtual AccessibilityObject *CellForColumnAndRow(uint32 column, uint32 row) noexcept
    {
      (void)column;
      (void)row;
      return nullptr;
    }
    /// @brief A table's column headers, together in an object that isn't in the tree.
    KRYS_NODISCARD virtual AccessibilityObject *TableHeaderContainer() noexcept
    {
      return nullptr;
    }
    /// @brief A cell's column headers (the headers above it in its column), or a table's (each column's).
    KRYS_NODISCARD AccessibilityChildrenVector ColumnHeaders() noexcept;
    /// @brief A cell's row headers (the headers before it in its row), or a table's or row's (each row's).
    KRYS_NODISCARD virtual AccessibilityChildrenVector RowHeaders() noexcept
    {
      return {};
    }
    /// @brief A column's header cell.
    KRYS_NODISCARD AccessibilityObject *ColumnHeader() noexcept;
    /// @brief A row's header cell: its first cell, if that's a `th` and the rest aren't; for a grid row, its
    /// first row header.
    KRYS_NODISCARD AccessibilityObject *RowHeader() noexcept;
    /// @brief A table's `aria-rowcount`, if valid (-1 for an unknown count); 0 otherwise.
    KRYS_NODISCARD virtual int32 AxRowCount() const noexcept
    {
      return 0;
    }
    /// @brief A table's `aria-colcount`, if valid (-1 for an unknown count); 0 otherwise.
    KRYS_NODISCARD virtual int32 AxColumnCount() const noexcept
    {
      return 0;
    }
    /// @brief A row's or cell's `aria-rowindex` (a cell's falling back to its row's).
    KRYS_NODISCARD virtual Maybe<uint32> AxRowIndex() const noexcept
    {
      return null;
    }
    /// @brief A row's or cell's `aria-colindex` (a cell's falling back to the one its row gives it).
    KRYS_NODISCARD virtual Maybe<uint32> AxColumnIndex() const noexcept
    {
      return null;
    }
    KRYS_NODISCARD virtual dom::DOMString AxRowIndexText() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual dom::DOMString AxColumnIndexText() const noexcept
    {
      return {};
    }
    /// @brief A column's index in its table.
    KRYS_NODISCARD virtual uint32 ColumnIndex() const noexcept
    {
      return 0;
    }
    /// @brief A cell's first row and the number of rows it spans.
    KRYS_NODISCARD virtual Pair<uint32, uint32> RowIndexRange() const noexcept
    {
      return {0, 1};
    }
    /// @brief A cell's first column and the number of columns it spans.
    KRYS_NODISCARD virtual Pair<uint32, uint32> ColumnIndexRange() const noexcept
    {
      return {0, 1};
    }
    /// @brief A cell's `scope`.
    KRYS_NODISCARD dom::DOMString CellScope() const noexcept
    {
      return GetAttribute(u8"scope");
    }
    /// @brief The ID of a cell's nearest `thead`, `tbody` or `tfoot`.
    KRYS_NODISCARD Maybe<AXID> RowGroupAncestorID() const noexcept;
    KRYS_NODISCARD bool IsTableCellInSameRowGroup(const AccessibilityObject &otherTableCell) const noexcept;
    /// @brief Brings a table's cell slots, and so its cells' indexes, up to date.
    void EnsureCellIndexesUpToDate() noexcept
    {
      UpdateChildrenIfNecessary();
    }

    // MathML.

    /// @brief Whether the object is in a math tree but left out of it (an anonymous box, an element such as
    /// mstyle that isn't exposed).
    KRYS_NODISCARD virtual bool IsIgnoredElementWithinMathTree() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathFraction() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathSubscriptSuperscript() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathRow() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathUnderOver() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathRoot() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathSquareRoot() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathToken() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathOperator() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathFenceOperator() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathSeparatorOperator() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathText() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathNumber() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathIdentifier() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathMultiscript() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathTable() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathTableRow() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsMathTableCell() const noexcept
    {
      return false;
    }
    /// @brief A root's radicand: a square root's children, or a root's first child.
    KRYS_NODISCARD virtual Maybe<AccessibilityChildrenVector> MathRadicand() noexcept
    {
      return null;
    }
    /// @brief A root's index.
    KRYS_NODISCARD virtual AccessibilityObject *MathRootIndexObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityObject *MathNumeratorObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityObject *MathDenominatorObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityObject *MathUnderObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityObject *MathOverObject() noexcept
    {
      return nullptr;
    }
    /// @brief A script's or under-over's base.
    KRYS_NODISCARD virtual AccessibilityObject *MathBaseObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityObject *MathSubscriptObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityObject *MathSuperscriptObject() noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual AccessibilityMathMultiscriptPairs MathPrescripts() noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual AccessibilityMathMultiscriptPairs MathPostscripts() noexcept
    {
      return {};
    }

    // Geometry.

    /// @brief The object's bounds, in the root view's coordinates.
    KRYS_NODISCARD virtual LayoutRect ElementRect() const noexcept
    {
      return {};
    }

  protected:
    AccessibilityObject(AXID id, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD virtual AccessibilityRole DetermineAccessibilityRole() noexcept = 0;
    virtual void RecomputeAriaRole() noexcept
    {
    }
    /// @brief Whether the rules for the object's kind leave it out of the tree.
    KRYS_NODISCARD virtual bool ComputeIsIgnored() const noexcept
    {
      return true;
    }
    /// @brief Lets go of the node and renderer, and of the children.
    virtual void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept;

    /// @brief The role a `button` (or a button-like input) takes: toggle button if `aria-pressed` is set,
    /// pop-up button if it or a link around it has a pop-up.
    KRYS_NODISCARD AccessibilityRole ButtonRoleType() const noexcept;
    KRYS_NODISCARD bool HasPopup() const noexcept;
    void ResetChildrenIndexInParent() noexcept;
    /// @brief Whether the object's children are its own: a column's and a table header container's cells
    /// are their rows', which set their index.
    KRYS_NODISCARD bool ShouldSetChildIndexInParent() const noexcept
    {
      return _role != AccessibilityRole::Column && _role != AccessibilityRole::TableHeaderContainer;
    }

  private:
    KRYS_NODISCARD bool IsIgnoredWithoutCache() const noexcept;
    /// @brief Whether the object is ignored for being outside the modal dialog that's open: unless it
    /// controls or is the active descendant of something in it, or is stacked above it.
    KRYS_NODISCARD bool IgnoredFromModalPresence() const noexcept;
    /// @brief Whether the object is in @p modalNode (in the composed tree).
    KRYS_NODISCARD bool IsModalDescendant(const dom::Node &modalNode) const noexcept;
    void PostMenuClosedNotificationIfNecessary() const noexcept;
  };
}
