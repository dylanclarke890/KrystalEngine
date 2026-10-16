#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  class RenderTextFragment;

  /// @brief A renderer that follows the CSS box model: a box or an inline box.
  // FIXME: Painting (backgrounds, borders, box shadows), continuations, sticky positioning and the geometry
  // come with layout and painting.
  class RenderBoxModelObject : public RenderLayerModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderBoxModelObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBoxModelObject);

  public:
    ~RenderBoxModelObject() noexcept override;

    KRYS_NODISCARD bool HasVisibleBoxDecorationStyle() const noexcept;

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
    KRYS_NODISCARD LayoutUnit OffsetLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetTop() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingTop() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingRight() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingLeft() const noexcept;
    KRYS_NODISCARD LayoutSize RelativePositionOffset() const noexcept;
    KRYS_NODISCARD LayoutUnit ContainingBlockLogicalWidthForContent() const noexcept;

    /// @brief For a `::first-letter` renderer, the text after the first letter.
    KRYS_NODISCARD RenderTextFragment *FirstLetterRemainingText() const noexcept;
    void SetFirstLetterRemainingText(RenderTextFragment &remainingText) noexcept;
    void ClearFirstLetterRemainingText() noexcept;

  protected:
    RenderBoxModelObject(Type type, dom::Element &element, RenderStyle &&style,
                         EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderBoxModelObject(Type type, dom::Document &document, RenderStyle &&style,
                         EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;

    void WillBeDestroyed() noexcept override;
    void UpdateFromStyle() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBoxModelObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBoxModelObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
