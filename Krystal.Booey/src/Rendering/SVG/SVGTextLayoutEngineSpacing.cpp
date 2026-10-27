#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutEngineSpacing.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"

namespace krys::boo
{
  SVGTextLayoutEngineSpacing::SVGTextLayoutEngineSpacing(const FontCascade &font) noexcept : _font(font)
  {
  }

  float SVGTextLayoutEngineSpacing::CalculateCSSSpacing(char32 currentCharacter) noexcept
  {
    float spacing = _font.LetterSpacing();
    if (_font.WordSpacing() && FontCascade::TreatAsSpace(currentCharacter)
        && !FontCascade::TreatAsSpace(_lastCharacter))
    {
      spacing += _font.WordSpacing();
    }

    _lastCharacter = currentCharacter;
    return spacing;
  }
}
