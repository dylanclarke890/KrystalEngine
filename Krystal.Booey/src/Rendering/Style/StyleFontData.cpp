#include "Krystal.Booey/Rendering/Style/StyleFontData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{
  StyleFontData::StyleFontData() noexcept
      : LetterSpacing(style::ComputedStyle::InitialLetterSpacing()),
        WordSpacing(style::ComputedStyle::InitialWordSpacing())
  {
  }

  StyleFontData::StyleFontData(const StyleFontData &o) noexcept
      : LetterSpacing(o.LetterSpacing), WordSpacing(o.WordSpacing), FontCascade(o.FontCascade)
  {
  }

  Ref<StyleFontData> StyleFontData::Copy() const noexcept
  {
    return AdoptRef(*new StyleFontData(*this));
  }

  bool StyleFontData::operator==(const StyleFontData &o) const noexcept
  {
    return LetterSpacing == o.LetterSpacing && WordSpacing == o.WordSpacing && FontCascade == o.FontCascade;
  }
}
