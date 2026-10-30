#pragma once

#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTextLineBoxes.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributes.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class SVGInlineTextBox;

  /// @brief The text of an SVG text content element, laid out by the SVG text layout.
  class RenderSVGInlineText final : public RenderText
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGInlineText);

  private:
    float _scalingFactor {1};
    FontCascade _scaledFont;
    SVGTextLayoutAttributes _layoutAttributes;
    RenderTextLineBoxes _legacyLineBoxes;

  public:
    RenderSVGInlineText(dom::Text &, const utf8_string &) noexcept;
    ~RenderSVGInlineText() noexcept override;

    KRYS_NODISCARD dom::Text &TextNode() const noexcept
    {
      return Downcast<dom::Text>(NodeForNonAnonymous());
    }

    KRYS_NODISCARD bool CharacterStartsNewTextChunk(int32 position) const noexcept;
    KRYS_NODISCARD SVGTextLayoutAttributes *LayoutAttributes() noexcept
    {
      return &_layoutAttributes;
    }
    KRYS_NODISCARD const SVGTextLayoutAttributes *LayoutAttributes() const noexcept
    {
      return &_layoutAttributes;
    }

    /// @brief The font-size scaling factor, ignoring the text-rendering mode.
    /// @note ScalingFactor() takes it into account, and thus returns 1 whenever text-rendering is set to
    /// 'geometricPrecision'. Therefore if you need access to the vanilla scaling factor, use this method
    /// directly (e.g. for non-scaling-stroke).
    KRYS_NODISCARD static float ComputeScalingFactorForRenderer(const RenderObject &) noexcept;

    KRYS_NODISCARD float ScalingFactor() const noexcept
    {
      return _scalingFactor;
    }
    KRYS_NODISCARD const FontCascade &ScaledFont() const noexcept
    {
      return _scaledFont;
    }
    void UpdateScaledFont() noexcept;
    static bool ComputeNewScaledFontForStyle(const RenderObject &, const RenderStyle &, float &scalingFactor,
                                             FontCascade &scaledFont) noexcept;

    /// @brief Preserves floating point precision for the use in dumps. It knows how to round and does a
    /// better job than EnclosingIntRect.
    KRYS_NODISCARD FloatRect FloatLinesBoundingBox() const noexcept;

    void RemoveTextBox(LegacyInlineTextBox &box) noexcept
    {
      _legacyLineBoxes.Remove(box);
    }
    LegacyInlineTextBox *CreateInlineTextBox() noexcept
    {
      return _legacyLineBoxes.CreateAndAppendLineBox(*this);
    }
    void DeleteLegacyLineBoxes() noexcept;
    KRYS_NODISCARD LegacyInlineTextBox *FirstLegacyTextBox() const noexcept
    {
      return _legacyLineBoxes.First();
    }
    void RemoveAndDestroyLegacyTextBoxes() noexcept;
    KRYS_NODISCARD UniquePtr<LegacyInlineTextBox> CreateTextBox() noexcept;

  private:
    void WillBeDestroyed() noexcept final;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGInlineText";
    }

    KRYS_NODISCARD utf8_string OriginalText() const noexcept override;
    void StyleDidChange(style::Difference, const RenderStyle *) noexcept override;

    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept override
    {
      return FloatLinesBoundingBox();
    }

    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &,
                                                              HitTestSource) noexcept override;
    KRYS_NODISCARD IntRect LinesBoundingBox() const noexcept override;

    void SetTextInternal(utf8_string, bool force) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderSVGInlineText)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGInlineText();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
