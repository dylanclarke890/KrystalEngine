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

#pragma once

#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"

namespace krys::boo::html
{
  class HTMLTableElement;
  class HTMLTableRowElement;

  /// @brief A table's rows: those of its `thead`s, then those that are its children or in its `tbody`s, then
  /// those of its `tfoot`s, each in tree order.
  /// @see https://html.spec.whatwg.org/#dom-table-rows
  class HTMLTableRowsCollection final : public dom::LiveHTMLCollection
  {
  private:
    Ref<HTMLTableElement> _table;

  public:
    explicit HTMLTableRowsCollection(HTMLTableElement &table) noexcept;

    /// @brief The row after @p previous in @p table's rows, or its first row if @p previous is null.
    KRYS_NODISCARD static HTMLTableRowElement *RowAfter(HTMLTableElement &table,
                                                        HTMLTableRowElement *previous) noexcept;

    /// @brief The last of @p table's rows.
    KRYS_NODISCARD static HTMLTableRowElement *LastRow(HTMLTableElement &table) noexcept;

  protected:
    KRYS_NODISCARD dom::Element *CollectionBegin() const noexcept override;
    KRYS_NODISCARD dom::Element *CollectionNext(dom::Element &element) const noexcept override;
  };
}
