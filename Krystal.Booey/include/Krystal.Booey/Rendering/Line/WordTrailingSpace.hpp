#pragma once

#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief The width of the space after a word, measured once and only when kerning makes it matter (a word
  /// is then measured with its trailing space, and the space's width taken off).
  struct WordTrailingSpace
  {
  private:
    enum class WordTrailingSpaceState : bool
    {
      Uninitialized,
      Initialized
    };

    const RenderStyle *_style;
    WordTrailingSpaceState _state {WordTrailingSpaceState::Uninitialized};
    Maybe<float> _width;

  public:
    explicit WordTrailingSpace(const RenderStyle &style,
                               bool measuringWithTrailingWhitespaceEnabled = true) noexcept
        : _style(&style)
    {
      if (!measuringWithTrailingWhitespaceEnabled || !_style->GetFontCascade().EnableKerning())
      {
        _state = WordTrailingSpaceState::Initialized;
      }
    }

    KRYS_NODISCARD Maybe<float> Width(HashSet<RefPtr<const Font>> &fallbackFonts) noexcept
    {
      if (_state == WordTrailingSpaceState::Initialized)
      {
        return _width;
      }

      auto &font = _style->GetFontCascade();
      _width = font.Width(RenderBlock::ConstructTextRun(u8" ", *_style), &fallbackFonts) + font.WordSpacing();
      _state = WordTrailingSpaceState::Initialized;
      return _width;
    }
  };
}
