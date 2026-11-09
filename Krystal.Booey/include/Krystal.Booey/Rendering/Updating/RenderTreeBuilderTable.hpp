/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class RenderTable;
  class RenderTableCell;
  class RenderTableRow;
  class RenderTableSection;

  /// @brief Attaches children to table parts, wrapping them in the anonymous table, section, row and cell
  /// boxes the table model needs.
  class RenderTreeBuilder::Table
  {
    KRYS_NON_COPY_MOVABLE(Table);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit Table(RenderTreeBuilder &builder) noexcept;

    KRYS_NODISCARD RenderElement &FindOrCreateParentForChild(RenderTableRow &parent,
                                                             const RenderObject &child,
                                                             RenderObject *&beforeChild) noexcept;
    KRYS_NODISCARD RenderElement &FindOrCreateParentForChild(RenderTableSection &parent,
                                                             const RenderObject &child,
                                                             RenderObject *&beforeChild) noexcept;
    KRYS_NODISCARD RenderElement &FindOrCreateParentForChild(RenderTable &parent, const RenderObject &child,
                                                             RenderObject *&beforeChild) noexcept;

    void Attach(RenderTable &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void Attach(RenderTableSection &parent, RenderPtr<RenderObject> child,
                RenderObject *beforeChild) noexcept;
    void Attach(RenderTableRow &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;

    KRYS_NODISCARD bool ChildRequiresTable(const RenderElement &parent, const RenderObject &child) noexcept;

    void CollapseAndDestroyAnonymousSiblingCells(const RenderTableCell &willBeDestroyed) noexcept;
    void CollapseAndDestroyAnonymousSiblingRows(const RenderTableRow &willBeDestroyed) noexcept;

    KRYS_NODISCARD static RenderPtr<RenderTable>
      CreateAnonymousTableWithStyle(dom::Document &document, const RenderStyle &style) noexcept;
    KRYS_NODISCARD static RenderPtr<RenderTableSection>
      CreateAnonymousTableSectionWithStyle(dom::Document &document, const RenderStyle &style) noexcept;
    KRYS_NODISCARD static RenderPtr<RenderTableRow>
      CreateAnonymousTableRowWithStyle(dom::Document &document, const RenderStyle &style) noexcept;
    KRYS_NODISCARD static RenderPtr<RenderTableCell>
      CreateAnonymousTableCellWithStyle(dom::Document &document, const RenderStyle &style) noexcept;

  private:
    template <typename Parent, typename Child>
    KRYS_NODISCARD RenderPtr<RenderObject> CollapseAndDetachAnonymousNextSibling(Parent *parent,
                                                                                 Child *previousSibling,
                                                                                 Child *nextSibling) noexcept;
  };
}
