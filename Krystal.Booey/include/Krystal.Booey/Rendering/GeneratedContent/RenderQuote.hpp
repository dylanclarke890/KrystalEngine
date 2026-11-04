#pragma once

#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief An `open-quote`, `close-quote`, `no-open-quote` or `no-close-quote` in generated content. Its
  /// text depends on its nesting depth, so the quotes are updated in tree order after the render tree
  /// changes.
  class RenderQuote final : public RenderInline
  {
    KRYS_NON_COPY_MOVABLE(RenderQuote);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderQuote);

  private:
    const QuoteType _type;
    int32 _depth {-1};
    utf8_string _text;

    bool _needsTextUpdate {false};

  public:
    RenderQuote(dom::Document &document, RenderStyle &&style, QuoteType quote) noexcept;
    ~RenderQuote() noexcept override;

    void UpdateRenderer(RenderTreeBuilder &builder, RenderQuote *previousQuote) noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderQuote";
    }

  private:
    KRYS_NODISCARD bool IsOpen() const noexcept;
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    KRYS_NODISCARD utf8_string ComputeText() const noexcept;
    void UpdateTextRenderer(RenderTreeBuilder &builder) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderQuote)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderQuote();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
