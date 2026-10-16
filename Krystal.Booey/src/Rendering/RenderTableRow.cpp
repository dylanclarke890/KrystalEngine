#include "Krystal.Booey/Rendering/RenderTableRow.hpp"

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
}
