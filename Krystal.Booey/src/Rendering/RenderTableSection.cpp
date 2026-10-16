#include "Krystal.Booey/Rendering/RenderTableSection.hpp"

namespace krys::boo
{
  RenderTableSection::RenderTableSection(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBox(Type::TableSection, element, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableSection());
  }

  RenderTableSection::RenderTableSection(dom::Document &document, RenderStyle &&style) noexcept
      : RenderBox(Type::TableSection, document, krys::move(style))
  {
    SetInline(false);
    krys_debug_assert(IsRenderTableSection());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableSection::~RenderTableSection() noexcept = default;
}
