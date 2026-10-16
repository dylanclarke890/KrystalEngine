#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"

namespace krys::boo
{
  RenderTableRow::RenderTableRow(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBox(Type::TableRow, element, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableRow());
  }

  RenderTableRow::RenderTableRow(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBox(Type::TableRow, document, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableRow());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableRow::~RenderTableRow() noexcept = default;

  RenderTableRow *RenderTableRow::NextRow() const noexcept
  {
    return Downcast<RenderTableRow>(NextSibling());
  }

  RenderTableRow *RenderTableRow::PreviousRow() const noexcept
  {
    return Downcast<RenderTableRow>(PreviousSibling());
  }

  RenderTableCell *RenderTableRow::FirstCell() const noexcept
  {
    return Downcast<RenderTableCell>(FirstChild());
  }

  RenderTableCell *RenderTableRow::LastCell() const noexcept
  {
    return Downcast<RenderTableCell>(LastChild());
  }

  RenderTableSection *RenderTableRow::Section() const noexcept
  {
    return Downcast<RenderTableSection>(static_cast<RenderObject *>(Parent()));
  }
}
