#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo
{
  RenderInline::RenderInline(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBoxModelObject(type, element, krys::move(style), TypeFlag::IsRenderInline, {})
  {
    SetChildrenInline(true);
    krys_debug_assert(IsRenderInline());
  }

  RenderInline::RenderInline(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBoxModelObject(type, document, krys::move(style), TypeFlag::IsRenderInline, {})
  {
    SetChildrenInline(true);
    krys_debug_assert(IsRenderInline());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderInline::~RenderInline() noexcept = default;

  void RenderInline::UpdateFromStyle() noexcept
  {
    RenderBoxModelObject::UpdateFromStyle();

    // FIXME: Support transforms and reflections on inline flows someday.
    SetHasTransformRelatedProperty(false);
  }

  utf8_stringview RenderInline::RenderName() const noexcept
  {
    if (IsRelativelyPositioned())
    {
      return u8"RenderInline (relative positioned)";
    }
    if (IsStickilyPositioned())
    {
      return u8"RenderInline (sticky positioned)";
    }
    // FIXME: Temporary hack while the new generated content system is being implemented.
    if (IsPseudoElement())
    {
      return u8"RenderInline (generated)";
    }
    if (IsAnonymous())
    {
      return u8"RenderInline (generated)";
    }
    return u8"RenderInline";
  }

  LayoutUnit RenderInline::InnerPaddingBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderInline::InnerPaddingBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  bool IsEmptyInline(const RenderInline &renderer) noexcept
  {
    for (auto &current : ChildrenOfType<RenderObject>(renderer))
    {
      if (current.IsFloatingOrOutOfFlowPositioned())
      {
        continue;
      }
      if (auto *text = DynamicDowncast<RenderText>(current))
      {
        if (!text->ContainsOnlyCollapsibleWhitespace())
        {
          return false;
        }
        continue;
      }
      auto *renderInline = DynamicDowncast<RenderInline>(current);
      if (!renderInline || !IsEmptyInline(*renderInline))
      {
        return false;
      }
    }
    return true;
  }
}
