#pragma once

#include "Krystal.Booey/Rendering/MathML/RenderMathMLBlock.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace mathml
  {
    class MathMLTokenElement;
  }

  /// @brief The renderer of a token element (`<mi>`, `<mn>`, `<ms>` and `<mtext>`) and the base of the
  /// operator renderer: a block of its text.
  class RenderMathMLToken : public RenderMathMLBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderMathMLToken);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMathMLToken);

  public:
    RenderMathMLToken(Type type, mathml::MathMLTokenElement &element, RenderStyle &&style) noexcept;
    RenderMathMLToken(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderMathMLToken() noexcept override;

    KRYS_NODISCARD mathml::MathMLTokenElement &GetElement() const noexcept;

    /// @brief Updates the renderer after the token's content changed.
    virtual void UpdateTokenContent() noexcept;
    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMathMLToken";
    }

  private:
    KRYS_NODISCARD bool IsChildAllowed(const RenderObject &, const RenderStyle &) const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMathMLToken)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMathMLToken();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
