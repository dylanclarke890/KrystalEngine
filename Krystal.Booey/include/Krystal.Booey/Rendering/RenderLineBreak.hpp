#pragma once

#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace html
  {
    class HTMLElement;
  }

  /// @brief The line break of a `<br>`, or the line break opportunity of a `<wbr>`.
  // FIXME: The lines' bounding box, the zero margins, the offset sizes, caret offsets, hit testing and
  // selection come with layout.
  class RenderLineBreak final : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderLineBreak);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLineBreak);

  public:
    RenderLineBreak(html::HTMLElement &element, RenderStyle &&style) noexcept;
    ~RenderLineBreak() noexcept override;

    // FIXME: The lies here keep render tree dump based test results unchanged.
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept final
    {
      return IsWBR() ? u8"RenderWordBreak" : u8"RenderBR";
    }

    KRYS_NODISCARD bool IsBR() const noexcept
    {
      return !HasWBRLineBreakFlag();
    }
    KRYS_NODISCARD bool IsWBR() const noexcept
    {
      return HasWBRLineBreakFlag();
    }
    KRYS_NODISCARD bool IsLineBreakOpportunity() const noexcept
    {
      return IsWBR();
    }

  private:
    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return false;
    }

    void UpdateFromStyle() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderLineBreak)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderLineBreak();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
