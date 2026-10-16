#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderBlock::RenderBlock(Type type, dom::Element &element, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBox(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsRenderBlock,
                  typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBlock());
  }

  RenderBlock::RenderBlock(Type type, dom::Document &document, RenderStyle &&style,
                           EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBox(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsRenderBlock,
                  typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBlock());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBlock::~RenderBlock() noexcept = default;

  utf8_stringview RenderBlock::RenderName() const noexcept
  {
    if (IsBody())
    {
      return u8"RenderBody"; // FIXME: Temporary hack until we know that the regression tests pass.
    }
    if (IsFieldset())
    {
      return u8"RenderFieldSet"; // FIXME: Remove eventually, but done to keep tests from breaking.
    }
    if (IsFloating())
    {
      return u8"RenderBlock (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderBlock (positioned)";
    }
    if (IsAnonymousBlock())
    {
      return u8"RenderBlock (anonymous)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderBlock (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderBlock (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderBlock (relative positioned)";
    }
    if (IsStickilyPositioned())
    {
      return u8"RenderBlock (sticky positioned)";
    }
    return u8"RenderBlock";
  }

  LayoutUnit RenderBlock::PaddingBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBlock::PaddingBoxHeight() const noexcept
  {
    krys_unreachable();
  }
}
