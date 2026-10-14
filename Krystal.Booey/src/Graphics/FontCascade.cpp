#include "Krystal.Booey/Graphics/FontCascade.hpp"

namespace krys::boo
{
  namespace
  {
    uint32 LastFontCascadeGeneration = 0;
  }

  FontCascade::FontCascade(FontCascadeDescription &&description) noexcept
      : _fontDescription(krys::move(description)), _generation(++LastFontCascadeGeneration),
        _enableKerning(ComputeEnableKerning()), _requiresShaping(ComputeRequiresShaping())
  {
    _fontDescription.SetShouldDisableLigaturesForSpacing(false);
  }

  FontCascade::FontCascade(FontCascadeDescription &&description, const FontCascade &other) noexcept
      : _fontDescription(krys::move(description)), _spacing(other._spacing),
        _generation(++LastFontCascadeGeneration), _enableKerning(ComputeEnableKerning()),
        _requiresShaping(ComputeRequiresShaping())
  {
  }

  bool FontCascade::operator==(const FontCascade &other) const noexcept
  {
    return _fontDescription == other._fontDescription && _spacing == other._spacing;
  }

  const FontMetrics &FontCascade::MetricsOfPrimaryFont() const noexcept
  {
    auto size = _fontDescription.ComputedSize();
    FontMetrics metrics;
    metrics.SetAscent(size);
    metrics.SetLineSpacing(size * 1.2f);
    metrics.SetIdeogramWidth(size);
    _primaryFontMetrics = metrics;
    return _primaryFontMetrics;
  }

  float FontCascade::ZeroWidth() const noexcept
  {
    // This represents the advance measure of the glyph 0 (zero, the Unicode character U+0030) in the
    // element's font. In cases where it is impossible or impractical to determine the measure of the 0 glyph,
    // it must be assumed to be 0.5em.
    auto defaultZeroWidthValue = GetFontDescription().ComputedSize() / 2;
    // FIXME: Use the metrics of the font that has the `0` glyph, once there's a font backend.
    return MetricsOfPrimaryFont().ZeroWidth().value_or(defaultZeroWidthValue);
  }
}
