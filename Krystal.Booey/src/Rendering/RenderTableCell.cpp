#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"

namespace krys::boo
{
  RenderTableCell::RenderTableCell(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCell, element, krys::move(style))
  {
  }

  RenderTableCell::RenderTableCell(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCell, document, krys::move(style))
  {
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCell::~RenderTableCell() noexcept = default;

  RenderTableCell *RenderTableCell::NextCell() const noexcept
  {
    return Downcast<RenderTableCell>(NextSibling());
  }

  RenderTableCell *RenderTableCell::PreviousCell() const noexcept
  {
    return Downcast<RenderTableCell>(PreviousSibling());
  }

  RenderTableRow *RenderTableCell::Row() const noexcept
  {
    return Downcast<RenderTableRow>(static_cast<RenderObject *>(Parent()));
  }

  RenderTableSection *RenderTableCell::Section() const noexcept
  {
    auto *row = Row();
    if (!row)
    {
      return nullptr;
    }
    return row->Section();
  }
}
