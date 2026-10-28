#pragma once

#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Pattern;
  class SourceImage;
}

namespace krys::boo::html
{
  /// @brief An image a 2D context fills or strokes with, repeated.
  /// @see https://html.spec.whatwg.org/#canvaspattern
  class CanvasPattern : public RefCounted<CanvasPattern>
  {
  private:
    const Ref<Pattern> _pattern;

  public:
    KRYS_NODISCARD static Ref<CanvasPattern> Create(SourceImage &&, bool repeatX, bool repeatY) noexcept;

    ~CanvasPattern() noexcept;

    /// @brief Reads @p type as a pattern's repetition (`repeat`, `repeat-x`, `repeat-y` or `no-repeat`; empty
    /// is `repeat`).
    /// @return False if @p type isn't one.
    KRYS_NODISCARD static bool ParseRepetitionType(dom::DOMStringView type, bool &repeatX,
                                                   bool &repeatY) noexcept;

    KRYS_NODISCARD Pattern &GetPattern() noexcept
    {
      return *_pattern;
    }
    KRYS_NODISCARD const Pattern &GetPattern() const noexcept
    {
      return *_pattern;
    }

    /// @see https://html.spec.whatwg.org/#dom-canvaspattern-settransform
    dom::ExceptionOr<void> SetTransform(css::DOMMatrix2DInit && = {}) noexcept;

  private:
    CanvasPattern(SourceImage &&, bool repeatX, bool repeatY) noexcept;
  };
}
