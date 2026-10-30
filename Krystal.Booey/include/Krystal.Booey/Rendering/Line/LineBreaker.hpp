#pragma once

#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineIterator.hpp"
#include "Krystal.Booey/Rendering/Line/LineInfo.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"

namespace krys::boo
{
  class FontCascade;
  class RenderText;

  struct RenderTextInfo
  {
    CheckedPtr<RenderText> Text;
    CachedLineBreakIteratorFactory LineBreakIteratorFactory;
    const FontCascade *Font {nullptr};
  };

  /// @brief Breaks the legacy line layout's content into lines.
  class LineBreaker
  {
    friend class BreakingContext;

  private:
    CheckedRef<RenderBlockFlow> _block;

  public:
    explicit LineBreaker(RenderBlockFlow &block) noexcept : _block(ShareCheckedRef(block))
    {
    }

    KRYS_NODISCARD LegacyInlineIterator NextLineBreak(InlineBidiResolver &, LineInfo &,
                                                      RenderTextInfo &) noexcept;

  private:
    void SkipTrailingWhitespace(LegacyInlineIterator &, const LineInfo &) noexcept;
    void SkipLeadingWhitespace(InlineBidiResolver &, LineInfo &) noexcept;
  };
}
