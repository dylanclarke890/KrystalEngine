#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLSpaceElement;
  }

  /// @brief The renderer of `<mspace>`: an empty box of the element's width, height and depth.
  class RenderMathMLSpace final : public RenderMathMLBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLSpace);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLSpace);

  public:
    RenderMathMLSpace(mathml::MathMLSpaceElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLSpace() noexcept override;

    KRYS_NODISCARD mathml::MathMLSpaceElement &GetElement() const noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLSpace";
    }

  private:
    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &, const RenderStyle &) const noexcept final
    {
      return false;
    }
    void ComputePreferredLogicalWidths() noexcept final;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;

    KRYS_NODISCARD LayoutUnit SpaceWidth() const noexcept;
    void GetSpaceHeightAndDepth(LayoutUnit &height, LayoutUnit &depth) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLSpace)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLSpace();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
