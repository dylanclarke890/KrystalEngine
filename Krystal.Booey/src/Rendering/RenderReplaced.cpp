#include "Krystal.Booey/Rendering/RenderReplaced.hpp"

namespace krys::boo
{
  RenderReplaced::RenderReplaced(Type type, dom::Element &element, RenderStyle &&style,
                                 EnumSet<ReplacedFlag> flags) noexcept
      : RenderBox(type, element, krys::move(style), {}, flags)
  {
    // FIXME: Assert the element is replaced (Element::isReplaced), once replaced elements say so.
    SetBlockLevelReplacedOrAtomicInline(true);
    krys_debug_assert(IsRenderReplaced());
  }

  RenderReplaced::RenderReplaced(Type type, dom::Document &document, RenderStyle &&style,
                                 EnumSet<ReplacedFlag> flags) noexcept
      : RenderBox(type, document, krys::move(style), {}, flags)
  {
    // FIXME: Assert the element is replaced (Element::isReplaced), once replaced elements say so.
    SetBlockLevelReplacedOrAtomicInline(true);
    krys_debug_assert(IsRenderReplaced());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderReplaced::~RenderReplaced() noexcept = default;
}
