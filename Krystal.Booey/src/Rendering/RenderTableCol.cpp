#include "Krystal.Booey/Rendering/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderTableCol::RenderTableCol(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBox(Type::TableCol, element, krys::move(style))
  {
    // init RenderObject attributes
    SetInline(true); // our object is not Inline
    krys_debug_assert(IsRenderTableCol());
  }

  RenderTableCol::RenderTableCol(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBox(Type::TableCol, document, krys::move(style))
  {
    // init RenderObject attributes
    SetInline(true); // our object is not Inline
    krys_debug_assert(IsRenderTableCol());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCol::~RenderTableCol() noexcept = default;

  bool RenderTableCol::IsTableColumnGroup() const noexcept
  {
    return Style().Display() == DisplayType::TableColumnGroup;
  }

  bool RenderTableCol::IsTableColumn() const noexcept
  {
    return Style().Display() == DisplayType::TableColumn;
  }

  bool RenderTableCol::IsChildAllowed(const RenderObject &child, const RenderStyle &style) const noexcept
  {
    // We cannot use isTableColumn here as style() may return 0.
    return style.Display() == DisplayType::TableColumn && child.IsRenderTableCol();
  }

  bool RenderTableCol::CanHaveChildren() const noexcept
  {
    // Cols cannot have children. This is actually necessary to fix a bug
    // with libraries.uc.edu, which makes a <p> be a table-column.
    return IsTableColumnGroup();
  }
}
