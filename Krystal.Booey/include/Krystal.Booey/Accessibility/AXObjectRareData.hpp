/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Accessibility/AccessibilityObject.hpp"

namespace krys::boo::ax
{
  /// @brief What only some node objects need: a table's rows, columns and cell slots, and a row's or cell's
  /// place in its table.
  class AXObjectRareData
  {
  private:
    KRYS_NON_COPY_MOVABLE(AXObjectRareData);

    // Tables.
    AccessibilityObject::AccessibilityChildrenVector _tableRows;
    AccessibilityObject::AccessibilityChildrenVector _tableColumns;
    // The cell covering each slot (https://html.spec.whatwg.org/#concept-slots), by row then column.
    List<List<Maybe<AXID>>> _cellSlots;
    RefPtr<AccessibilityObject> _tableHeaderContainer;
    bool _isExposableTable {false};

    // Rows and cells.
    uint32 _rowIndex {0};
    uint32 _columnIndex {0};
    int32 _axColIndexFromRow {-1};
    uint32 _effectiveRowSpan {1};

  public:
    AXObjectRareData() noexcept = default;

    KRYS_NODISCARD const AccessibilityObject::AccessibilityChildrenVector &TableRows() const noexcept
    {
      return _tableRows;
    }
    KRYS_NODISCARD const AccessibilityObject::AccessibilityChildrenVector &TableColumns() const noexcept
    {
      return _tableColumns;
    }
    KRYS_NODISCARD uint32 RowCount() const noexcept
    {
      return static_cast<uint32>(_tableRows.size());
    }
    KRYS_NODISCARD uint32 ColumnCount() const noexcept
    {
      return static_cast<uint32>(_tableColumns.size());
    }
    void AppendColumn(AccessibilityObject &column) noexcept
    {
      _tableColumns.emplace_back(ShareRef(column));
    }
    void AppendRow(AccessibilityObject &row) noexcept
    {
      _tableRows.emplace_back(ShareRef(row));
    }
    KRYS_NODISCARD bool IsExposableTable() const noexcept
    {
      return _isExposableTable;
    }
    void SetIsExposableTable(bool value) noexcept
    {
      _isExposableTable = value;
    }
    KRYS_NODISCARD AccessibilityObject *TableHeaderContainer() const noexcept
    {
      return _tableHeaderContainer.get();
    }
    void SetTableHeaderContainer(AccessibilityObject &object) noexcept
    {
      _tableHeaderContainer = ShareRefPtr(&object);
    }
    KRYS_NODISCARD const List<List<Maybe<AXID>>> &CellSlots() const noexcept
    {
      return _cellSlots;
    }
    KRYS_NODISCARD List<List<Maybe<AXID>>> &MutableCellSlots() noexcept
    {
      return _cellSlots;
    }
    /// @brief Forgets what the table's children determine (rows, columns, cell slots and the header
    /// container), for when they're cleared.
    void ResetChildrenDependentTableFields() noexcept
    {
      _tableRows.clear();
      _tableColumns.clear();
      _cellSlots.clear();
      if (auto container = std::exchange(_tableHeaderContainer, nullptr))
      {
        container->DetachFromParent();
      }
    }

    KRYS_NODISCARD uint32 RowIndex() const noexcept
    {
      return _rowIndex;
    }
    void SetRowIndex(uint32 rowIndex) noexcept
    {
      _rowIndex = rowIndex;
    }
    KRYS_NODISCARD uint32 ColumnIndex() const noexcept
    {
      return _columnIndex;
    }
    void SetColumnIndex(uint32 columnIndex) noexcept
    {
      _columnIndex = columnIndex;
    }
    KRYS_NODISCARD int32 AxColIndexFromRow() const noexcept
    {
      return _axColIndexFromRow;
    }
    void SetAXColIndexFromRow(int32 index) noexcept
    {
      _axColIndexFromRow = index;
    }
    KRYS_NODISCARD uint32 EffectiveRowSpan() const noexcept
    {
      return _effectiveRowSpan;
    }
    void IncrementEffectiveRowSpan() noexcept
    {
      ++_effectiveRowSpan;
    }
    void ResetEffectiveRowSpan() noexcept
    {
      _effectiveRowSpan = 1;
    }
  };
}
