#pragma once

#include "Krystal.Booey/Graphics/FontCascadeDescription.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief The fonts a run of text is drawn with: a description, the spacing applied between letters and
  /// words, and the fonts that description resolves to.
  /// @note FIXME: There's no font backend yet, so a cascade holds only its description and spacing: no fonts,
  /// font selector, glyph, width, drawing or text layout APIs. Its primary font's metrics are a fallback
  /// derived from the computed size (see `MetricsOfPrimaryFont`).
  class FontCascade final
  {
  private:
    struct Spacing
    {
      float Letter {0};
      float Word {0};

      KRYS_NODISCARD constexpr bool operator==(const Spacing &) const noexcept = default;
    };

    mutable FontCascadeDescription _fontDescription;
    Spacing _spacing;
    mutable FontMetrics _primaryFontMetrics;
    mutable uint32 _generation {0};
    bool _enableKerning {false};   // Computed from _fontDescription.
    bool _requiresShaping {false}; // Computed from _fontDescription.

  public:
    FontCascade() noexcept = default;
    explicit FontCascade(FontCascadeDescription &&) noexcept;
    FontCascade(FontCascadeDescription &&, const FontCascade &) noexcept;

    FontCascade(const FontCascade &) noexcept = default;
    FontCascade &operator=(const FontCascade &) noexcept = default;

    /// @note FIXME: Upstream also compares the fonts, the font selector and their versions, and two cascades
    /// with no fonts are never equal. With no fonts, the description and the spacing are compared.
    KRYS_NODISCARD bool operator==(const FontCascade &other) const noexcept;

    KRYS_NODISCARD const FontCascadeDescription &GetFontDescription() const noexcept
    {
      return _fontDescription;
    }
    KRYS_NODISCARD FontCascadeDescription &MutableFontDescription() const noexcept
    {
      return _fontDescription;
    }

    KRYS_NODISCARD float Size() const noexcept
    {
      return GetFontDescription().ComputedSize();
    }

    KRYS_NODISCARD bool IsSmallCaps() const noexcept
    {
      return _fontDescription.VariantCaps() == FontVariantCaps::Small;
    }

    KRYS_NODISCARD float LetterSpacing() const noexcept
    {
      return _spacing.Letter;
    }
    KRYS_NODISCARD float WordSpacing() const noexcept
    {
      return _spacing.Word;
    }
    void SetLetterSpacing(float spacing) noexcept
    {
      _spacing.Letter = spacing;
    }
    void SetWordSpacing(float spacing) noexcept
    {
      _spacing.Word = spacing;
    }
    KRYS_NODISCARD boo::TextSpacingTrim GetTextSpacingTrim() const noexcept
    {
      return _fontDescription.GetTextSpacingTrim();
    }
    KRYS_NODISCARD boo::TextAutospace GetTextAutospace() const noexcept
    {
      return _fontDescription.GetTextAutospace();
    }

    KRYS_NODISCARD bool EnableKerning() const noexcept
    {
      return _enableKerning;
    }
    KRYS_NODISCARD bool RequiresShaping() const noexcept
    {
      return _requiresShaping;
    }

    KRYS_NODISCARD const StringAtom &FirstFamily() const noexcept
    {
      return _fontDescription.FirstFamily();
    }
    KRYS_NODISCARD uint32 FamilyCount() const noexcept
    {
      return _fontDescription.FamilyCount();
    }
    KRYS_NODISCARD const StringAtom &FamilyAt(uint32 i) const noexcept
    {
      return _fontDescription.FamilyAt(i);
    }

    /// @brief A null return value indicates "font-style: normal".
    KRYS_NODISCARD Maybe<FontSelectionValue> FontStyleSlope() const noexcept
    {
      return _fontDescription.FontStyleSlope();
    }
    KRYS_NODISCARD FontSelectionValue Weight() const noexcept
    {
      return _fontDescription.Weight();
    }
    KRYS_NODISCARD FontWidthVariant WidthVariant() const noexcept
    {
      return _fontDescription.WidthVariant();
    }

    /// @brief The metrics of the primary font.
    /// @note FIXME: Until there's a font backend, there's no primary font: the metrics are an ascent of 1em,
    /// a line spacing of 1.2em and an ideogram width of 1em, with no descent, line gap, x-height, cap height
    /// or zero width (so `ex`, `cap` and `ch` use their CSS fallbacks).
    KRYS_NODISCARD const FontMetrics &MetricsOfPrimaryFont() const noexcept;

    /// @brief The advance of the `0` glyph, or 0.5em when it can't be determined.
    KRYS_NODISCARD float ZeroWidth() const noexcept;

    KRYS_NODISCARD constexpr static float SyntheticObliqueAngle() noexcept
    {
      return 14;
    }

    KRYS_NODISCARD uint32 Generation() const noexcept
    {
      return _generation;
    }

    void UpdateEnableKerning() noexcept
    {
      _enableKerning = ComputeEnableKerning();
    }
    void UpdateRequiresShaping() noexcept
    {
      _requiresShaping = ComputeRequiresShaping();
    }

  private:
    KRYS_NODISCARD bool AdvancedTextRenderingMode() const noexcept
    {
      return _fontDescription.GetTextRenderingMode() != TextRenderingMode::OptimizeSpeed;
    }

    KRYS_NODISCARD bool ComputeEnableKerning() const noexcept
    {
      auto kerning = _fontDescription.GetKerning();
      if (kerning == Kerning::Normal)
      {
        return true;
      }
      if (kerning == Kerning::NoShift)
      {
        return false;
      }
      return AdvancedTextRenderingMode();
    }

    KRYS_NODISCARD bool ComputeRequiresShaping() const noexcept
    {
      if (!_fontDescription.VariantSettings().IsAllNormal())
      {
        return true;
      }
      if (_fontDescription.FeatureSettings().size())
      {
        return true;
      }
      return AdvancedTextRenderingMode();
    }
  };
}
