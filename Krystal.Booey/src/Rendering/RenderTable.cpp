#include "Krystal.Booey/Rendering/RenderTable.hpp"

namespace krys::boo
{
  RenderTable::RenderTable(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(type, element, krys::move(style), {})
  {
  }

  RenderTable::RenderTable(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlock(type, document, krys::move(style), {})
  {
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTable::~RenderTable() noexcept = default;
}
