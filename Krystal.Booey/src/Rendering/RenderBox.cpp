#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderBox::RenderBox(Type type, dom::Element &element, RenderStyle &&style, EnumSet<TypeFlag> baseTypeFlags,
                       TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBoxModelObject(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsBox,
                             typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBox());
  }

  RenderBox::RenderBox(Type type, dom::Document &document, RenderStyle &&style,
                       EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderBoxModelObject(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsBox,
                             typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBox());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderBox::~RenderBox() noexcept = default;

  void RenderBox::UpdateFromStyle() noexcept
  {
    RenderBoxModelObject::UpdateFromStyle();

    const auto &styleToUse = Style();
    bool isDocElementRenderer = IsDocumentElementRenderer();
    bool isViewObject = IsRenderView();

    // The root and the RenderView always paint their backgrounds/borders.
    if (isDocElementRenderer || isViewObject)
    {
      SetHasVisibleBoxDecorations(true);
    }

    SetFloating(!IsOutOfFlowPositioned() && styleToUse.IsFloating());

    // We also handle <body> and <html>, whose overflow applies to the viewport.
    if (!(EffectiveOverflowX() == Overflow::Visible && EffectiveOverflowY() == Overflow::Visible)
        && !isDocElementRenderer && IsRenderBlock())
    {
      bool boxHasNonVisibleOverflow = true;
      if (IsBody())
      {
        // Overflow on the body can propagate to the viewport under the following conditions.
        // (1) The root element is <html>.
        // (2) We are the primary <body> (can be checked by looking at document.body).
        // (3) The root element has visible overflow.
        // (4) No containment is set either on the body or on the html document element.
        auto &document = GetDocument();
        auto documentElement = document.DocumentElement();
        auto *documentElementRenderer = documentElement->Renderer();
        auto *htmlElement = DynamicDowncast<html::HTMLElement>(documentElement.get());
        if (htmlElement && Is<html::HTMLHtmlElement>(*htmlElement) && document.Body().get() == GetElement()
            && documentElementRenderer && documentElementRenderer->EffectiveOverflowX() == Overflow::Visible
            && styleToUse.UsedContain().IsNone() && documentElementRenderer->Style().UsedContain().IsNone())
        {
          boxHasNonVisibleOverflow = false;
        }
      }
      // Check for overflow clip.
      // It's sufficient to just check one direction, since it's illegal to have visible on only one overflow
      // value.
      if (boxHasNonVisibleOverflow)
      {
        // FIXME: Repaint the overflow that's erased, once there's painting.
        SetHasNonVisibleOverflow();
      }
    }
    SetHasTransformRelatedProperty(ComputeHasTransformRelatedProperty(styleToUse));
    // FIXME: Reflections (`-webkit-box-reflect` was dropped).
  }

  bool RenderBox::ComputeHasTransformRelatedProperty(const RenderStyle &styleToUse) const noexcept
  {
    if (styleToUse.HasTransformRelatedProperty())
    {
      return true;
    }

    // FIXME: The rest only applies with `css3DTransformBackfaceVisibilityInteroperabilityEnabled`, a preview
    // setting that's off by default.
    return false;
  }

  LayoutUnit RenderBox::ContentBoxWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxHeight() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxLogicalWidth() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContentBoxLogicalHeight() const noexcept
  {
    krys_unreachable();
  }

  LayoutRect RenderBox::BorderBoxRect() const noexcept
  {
    krys_unreachable();
  }

  LayoutRect RenderBox::ComputedCSSContentBoxRect() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::MarginTop() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::MarginRight() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::MarginBottom() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::MarginLeft() const noexcept
  {
    krys_unreachable();
  }

  bool RenderBox::HasTrimmedMargin(Maybe<style::MarginTrimSide>) const noexcept
  {
    krys_unreachable();
  }

  const RenderBox &RenderBox::EnclosingClippingBoxForStickyPosition() const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContainingBlockRangeSizeForPositioned(const RenderBoxModelObject &,
                                                              BoxAxis) const noexcept
  {
    krys_unreachable();
  }

  LayoutUnit RenderBox::ContainingBlockLogicalHeightForContent(AvailableLogicalHeightType) const noexcept
  {
    krys_unreachable();
  }
}
