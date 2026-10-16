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
