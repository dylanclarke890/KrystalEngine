#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief The renderer of `<math>`: a row, centred in its line when it's displayed as a block.
  class RenderMathMLMath final : public RenderMathMLRow
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLMath);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLMath);

  public:
    RenderMathMLMath(mathml::MathMLRowElement &element, RenderStyle &&style) noexcept;
    ~RenderMathMLMath() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return u8"RenderMathMLMath";
    }

  private:
    void CenterChildren(LayoutUnit contentWidth) noexcept;
    void LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight = 0_lu) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLMath)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLMath();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
