#include "Krystal.Booey/Rendering/RenderDeprecatedFlexibleBox.hpp"

namespace krys::boo
{
  RenderDeprecatedFlexibleBox::RenderDeprecatedFlexibleBox(dom::Element &element,
                                                           RenderStyle &&style) noexcept
      : RenderBlock(Type::DeprecatedFlexibleBox, element, krys::move(style), {})
  {
    SetChildrenInline(false); // All of our children must be block-level
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderDeprecatedFlexibleBox::~RenderDeprecatedFlexibleBox() noexcept = default;

  utf8_stringview RenderDeprecatedFlexibleBox::RenderName() const noexcept
  {
    if (IsFloating())
    {
      return u8"RenderDeprecatedFlexibleBox (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderDeprecatedFlexibleBox (positioned)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderDeprecatedFlexibleBox (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderDeprecatedFlexibleBox (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderDeprecatedFlexibleBox (relative positioned)";
    }
    return u8"RenderDeprecatedFlexibleBox";
  }
}
