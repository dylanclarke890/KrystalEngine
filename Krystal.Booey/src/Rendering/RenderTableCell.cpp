#include "Krystal.Booey/Rendering/RenderTableCell.hpp"

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
}
