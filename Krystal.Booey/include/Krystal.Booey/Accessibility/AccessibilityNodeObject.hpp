/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"
#include "Krystal.Booey/HTML/Types/TagName.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo::ax
{
  class AXObjectRareData;

  /// @brief An accessibility object for a DOM node: its role, states and name come from the node, and its
  /// children from the node's children in the composed tree. Nodes with no renderer that accessibility
  /// still needs (`display: contents`, canvas fallback content) get one of these; rendered ones get an
  /// `AccessibilityRenderObject`.
  class AccessibilityNodeObject : public AccessibilityObject
  {
  private:
    WeakPtr<dom::Node> _node;
    UniquePtr<AXObjectRareData> _rareData;

  protected:
    AccessibilityRole _ariaRole {AccessibilityRole::Unknown};

  public:
    KRYS_NODISCARD static Ref<AccessibilityNodeObject> Create(AXID id, dom::Node &node,
                                                              AXObjectCache &cache) noexcept;
    ~AccessibilityNodeObject() noexcept override;

    void Init() noexcept override;
    KRYS_NODISCARD bool IsDetached() const noexcept override
    {
      return !_node;
    }
    KRYS_NODISCARD bool IsAccessibilityNodeObject() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD dom::Node *GetNode() const noexcept override
    {
      return _node.get();
    }

    KRYS_NODISCARD AccessibilityObject *ParentObject() const noexcept override;
    void AddChildren() noexcept override;
    KRYS_NODISCARD bool CanHaveChildren() const noexcept override;
    void UpdateChildrenIfNecessary() noexcept override;
    void ClearChildren() noexcept override;
    void SetNeedsToUpdateChildren() noexcept override
    {
      _childrenDirty = true;
    }
    void SetNeedsToUpdateSubtree() noexcept override
    {
      _subtreeDirty = true;
    }
    KRYS_NODISCARD bool NeedsToUpdateChildren() const noexcept override
    {
      return _childrenDirty;
    }

    KRYS_NODISCARD AccessibilityRole AriaRoleAttribute() const noexcept override
    {
      return _ariaRole;
    }

    KRYS_NODISCARD bool IsFocused() const noexcept override;
    KRYS_NODISCARD bool IsEnabled() const noexcept override;
    KRYS_NODISCARD bool IsChecked() const noexcept override;
    KRYS_NODISCARD bool IsIndeterminate() const noexcept override;
    KRYS_NODISCARD bool IsPressed() const noexcept override;
    KRYS_NODISCARD bool IsSelected() const noexcept override;
    KRYS_NODISCARD bool IsRequired() const noexcept override;
    KRYS_NODISCARD bool IsMultiSelectable() const noexcept override;
    KRYS_NODISCARD bool SupportsExpanded() const noexcept override;
    KRYS_NODISCARD bool IsExpanded() const noexcept override;
    KRYS_NODISCARD bool CanSetFocusAttribute() const noexcept override;
    KRYS_NODISCARD uint32 HeadingLevel() const noexcept override;

    KRYS_NODISCARD dom::DOMString AccessibleName() const noexcept override;
    KRYS_NODISCARD dom::DOMString AccessibleDescription() const noexcept override;
    KRYS_NODISCARD dom::DOMString StringValue() const noexcept override;

    /// @brief Whether the node is an `input` or `textarea` that edits text.
    KRYS_NODISCARD bool IsNativeTextControl() const noexcept;
    /// @brief Whether the element is the top of a `contenteditable` region.
    KRYS_NODISCARD bool HasContentEditableAttributeSet() const noexcept;
    /// @brief Whether the element has `aria-label`, `aria-labelledby` or `title`, which give it a name.
    KRYS_NODISCARD bool HasAccNameAttribute() const noexcept;

    KRYS_NODISCARD float ValueForRange() const noexcept override;
    KRYS_NODISCARD float MaxValueForRange() const noexcept override;
    KRYS_NODISCARD float MinValueForRange() const noexcept override;
    KRYS_NODISCARD dom::DOMString ValueDescription() const noexcept override;
    KRYS_NODISCARD Maybe<AccessibilityOrientation> ExplicitOrientation() const noexcept override
    {
      return OrientationFromARIA();
    }

    /// @brief Whether the element is a list: `role=list` or `directory`, or a `ul`, `ol`, `dl` or `menu` with
    /// no role.
    KRYS_NODISCARD bool IsAccessibilityList() const noexcept;
    KRYS_NODISCARD bool IsDescriptionList() const noexcept;
    KRYS_NODISCARD bool IsUnorderedList() const noexcept;
    KRYS_NODISCARD bool IsOrderedList() const noexcept;

    KRYS_NODISCARD bool IsTable() const noexcept override;
    KRYS_NODISCARD bool IsExposableTable() const noexcept override;
    KRYS_NODISCARD bool IsAriaTable() const noexcept override;
    KRYS_NODISCARD bool IsTableRow() const noexcept override;
    KRYS_NODISCARD bool IsExposedTableRow() const noexcept override;
    KRYS_NODISCARD bool IsARIAGridRow() const noexcept override;
    KRYS_NODISCARD bool IsTableCell() const noexcept override;
    KRYS_NODISCARD bool IsExposedTableCell() const noexcept override;
    KRYS_NODISCARD bool IsColumnHeader() const noexcept override;
    KRYS_NODISCARD bool IsRowHeader() const noexcept override;
    KRYS_NODISCARD AccessibilityObject *ParentTable() const noexcept override;
    KRYS_NODISCARD AccessibilityChildrenVector Rows() noexcept override;
    KRYS_NODISCARD AccessibilityChildrenVector Columns() noexcept override;
    KRYS_NODISCARD AccessibilityChildrenVector Cells() noexcept override;
    KRYS_NODISCARD uint32 RowCount() noexcept override;
    KRYS_NODISCARD uint32 ColumnCount() noexcept override;
    KRYS_NODISCARD AccessibilityObject *CellForColumnAndRow(uint32 column, uint32 row) noexcept override;
    KRYS_NODISCARD AccessibilityObject *TableHeaderContainer() noexcept override;
    KRYS_NODISCARD AccessibilityChildrenVector RowHeaders() noexcept override;
    KRYS_NODISCARD int32 AxRowCount() const noexcept override;
    KRYS_NODISCARD int32 AxColumnCount() const noexcept override;
    KRYS_NODISCARD Maybe<uint32> AxRowIndex() const noexcept override;
    KRYS_NODISCARD Maybe<uint32> AxColumnIndex() const noexcept override;
    KRYS_NODISCARD dom::DOMString AxRowIndexText() const noexcept override;
    KRYS_NODISCARD dom::DOMString AxColumnIndexText() const noexcept override;
    KRYS_NODISCARD Pair<uint32, uint32> RowIndexRange() const noexcept override;
    KRYS_NODISCARD Pair<uint32, uint32> ColumnIndexRange() const noexcept override;
    /// @brief A cell's `colspan` (or else `aria-colspan`), clamped to 1 to 1000.
    KRYS_NODISCARD uint32 ColSpan() const noexcept;
    /// @brief A cell's `rowspan` (or else `aria-rowspan`), 0 meaning the rest of its row group.
    KRYS_NODISCARD uint32 RowSpan() const noexcept;
    /// @brief The row a cell is in, if the row is exposed.
    KRYS_NODISCARD AccessibilityNodeObject *ParentRow() const noexcept;
    /// @brief A row's table, if the table is exposed.
    KRYS_NODISCARD AccessibilityObject *ParentTableIfExposedTableRow() const noexcept;
    /// @brief Marks a table's cell slots out of date (with its children, which they're worked out with).
    void SetCellSlotsDirty() noexcept;
    /// @brief Works out again whether a table is exposed, updating its role and its cells' if it changed.
    void RecomputeIsExposableIfNecessary() noexcept;
    /// @brief Gives a row's cells their `aria-colindex`, from the row's.
    void SetAXColIndexFromRow(int32 index) noexcept;

  protected:
    AccessibilityNodeObject(AXID id, dom::Node *node, AXObjectCache &cache) noexcept;

    KRYS_NODISCARD AccessibilityRole DetermineAccessibilityRole() noexcept override;
    void RecomputeAriaRole() noexcept override
    {
      _ariaRole = DetermineAriaRoleAttribute();
    }
    KRYS_NODISCARD bool ComputeIsIgnored() const noexcept override;
    void DetachRemoteParts(AccessibilityDetachmentType detachmentType) noexcept override;

    /// @brief Whether `pre`, `kbd` and the like map to `Inline` (inside an inline renderer) or `TextGroup`.
    enum class TreatStyleFormatGroupAsInline : bool
    {
      No,
      Yes,
    };
    /// @brief The role the node's host language gives it (HTML-AAM's mappings), ignoring `role`.
    KRYS_NODISCARD AccessibilityRole
      DetermineAccessibilityRoleFromNode(TreatStyleFormatGroupAsInline treatStyleFormatGroupAsInline =
                                           TreatStyleFormatGroupAsInline::No) const noexcept;
    KRYS_NODISCARD AccessibilityRole RoleFromInputElement(const html::HTMLInputElement &input) const noexcept;
    /// @brief The role from the `role` attribute, if it names one that applies here.
    KRYS_NODISCARD AccessibilityRole DetermineAriaRoleAttribute() const noexcept;
    KRYS_NODISCARD AccessibilityRole RemapAriaRoleDueToParent(AccessibilityRole role) const noexcept;

    KRYS_NODISCARD bool IsDescendantOfElementType(Span<const html::TagName> tagNames) const noexcept;
    KRYS_NODISCARD Maybe<AccessibilityOrientation> OrientationFromARIA() const noexcept;
    /// @brief A list's role once its children are known: WebKit's heuristic makes a list with no items, or
    /// with no visible markers outside navigation, a group.
    KRYS_NODISCARD AccessibilityRole DetermineListRoleWithCleanChildren() noexcept;
    /// @brief Whether the element has something that makes it worth exposing even if its kind wouldn't be
    /// (an accessible name from an attribute, a `title` and so on).
    KRYS_NODISCARD bool HasAttributesRequiredForInclusion() const noexcept;
    /// @brief Whether the element is focusable but has no role of its own (`<div tabindex=0>`).
    KRYS_NODISCARD bool IsGenericFocusableElement() const noexcept;
    KRYS_NODISCARD bool IsStyleFormatGroup() const noexcept;
    /// @brief Whether the element is hidden by `display: none` or `visibility: hidden`.
    KRYS_NODISCARD bool IsRenderHidden() const noexcept;

    KRYS_NODISCARD AXObjectRareData &EnsureRareData() noexcept;
    /// @brief An exposed table's rows, then its columns and header container (which are ignored), working
    /// out its cell slots.
    void AddTableChildrenAndCellSlots() noexcept;
    /// @brief Whether the table is exposed: an ARIA table, or a data table rather than one for layout.
    KRYS_NODISCARD bool ComputeIsTableExposableThroughAccessibility() const noexcept
    {
      return IsAriaTable() || IsDataTable();
    }

  private:
    /// @brief The largest `rowspan` (https://html.spec.whatwg.org/#attr-tdth-rowspan).
    constexpr static uint32 MaxRowSpan = 65'534;

    /// @brief WebKit's heuristic for whether a table is for data (exposed) or for layout.
    KRYS_NODISCARD bool IsDataTable() const noexcept;
    KRYS_NODISCARD bool IsARIAGridCell() const noexcept;
    KRYS_NODISCARD bool IsTableHeaderCell() const noexcept;
    /// @brief The table's rare data, with its children (and so its rows, columns and cell slots) up to date.
    KRYS_NODISCARD AXObjectRareData *RareDataWithCleanTableChildren() noexcept;
    /// @brief Works out the table's cell slots and adds its caption and rows as children; returns the number
    /// of columns.
    uint32 ComputeCellSlots() noexcept;
    /// @brief Updates the roles of the table's rows and cells, which depend on whether it's exposed.
    void UpdateRowDescendantRoles() noexcept;
    void SetRowIndex(uint32 rowIndex) noexcept;
    void SetColumnIndex(uint32 columnIndex) noexcept;
    void IncrementEffectiveRowSpan() noexcept;
    void ResetEffectiveRowSpan() noexcept;
    void EnsureIndexesUpToDate() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ax::AccessibilityNodeObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::ax::AccessibilityObject &object) noexcept
  {
    return object.IsAccessibilityNodeObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
