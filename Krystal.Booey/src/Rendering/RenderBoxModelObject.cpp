#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderBoxModelObject::RenderBoxModelObject(Type type, dom::Element &element, RenderStyle &&style,
                                             EnumSet<TypeFlag> baseTypeFlags,
                                             TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderLayerModelObject(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsBoxModelObject,
                               typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBoxModelObject());
  }

  RenderBoxModelObject::RenderBoxModelObject(Type type, dom::Document &document, RenderStyle &&style,
                                             EnumSet<TypeFlag> baseTypeFlags,
                                             TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderLayerModelObject(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsBoxModelObject,
                               typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBoxModelObject());
  }

  RenderBoxModelObject::~RenderBoxModelObject() noexcept = default;

  bool RenderBoxModelObject::HasVisibleBoxDecorationStyle() const noexcept
  {
    return Style().HasBackground() || Style().HasVisibleBorderDecoration() || Style().HasUsedAppearance()
           || Style().HasBoxShadow();
  }

  void RenderBoxModelObject::UpdateFromStyle() noexcept
  {
    RenderLayerModelObject::UpdateFromStyle();

    // Set the appropriate bits for a box model object.  Since all bits are cleared in styleWillChange,
    // we only check for bits that could possibly be set to true.
    const auto &styleToUse = Style();
    SetHasVisibleBoxDecorations(HasVisibleBoxDecorationStyle());
    SetInline(styleToUse.IsDisplayInlineType());
    SetPositionState(styleToUse.GetPosition());
    SetHorizontalWritingMode(styleToUse.GetWritingMode().IsHorizontal());
    SetPaintContainmentApplies(ShouldApplyPaintContainment());
    // FIXME: Tell the frame view it has flipped block renderers, once it paints.
  }

  LayoutUnit RenderBoxModelObject::OffsetLeft() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::OffsetTop() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::OffsetWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::OffsetHeight() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingTop() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingRight() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingBottom() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingLeft() const noexcept
  {
    krys_unreachable();
  }

  LayoutSize RenderBoxModelObject::RelativePositionOffset() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBoxModelObject::ContainingBlockLogicalWidthForContent() const noexcept
  {
    krys_unreachable();
  }
}
