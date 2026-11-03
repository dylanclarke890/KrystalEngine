#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::html
{
  /// @brief The dimensions of a piece of text measured by a 2D context's `MeasureText`, relative to where it
  /// would be drawn.
  /// @see https://html.spec.whatwg.org/#textmetrics
  class TextMetrics final : public RefCounted<TextMetrics>
  {
  private:
    double _width {0};
    double _actualBoundingBoxLeft {0};
    double _actualBoundingBoxRight {0};
    double _fontBoundingBoxAscent {0};
    double _fontBoundingBoxDescent {0};
    double _actualBoundingBoxAscent {0};
    double _actualBoundingBoxDescent {0};
    double _emHeightAscent {0};
    double _emHeightDescent {0};
    double _hangingBaseline {0};
    double _alphabeticBaseline {0};
    double _ideographicBaseline {0};

  public:
    KRYS_NODISCARD static Ref<TextMetrics> Create() noexcept
    {
      return AdoptRef(*new TextMetrics);
    }

    KRYS_NODISCARD double Width() const noexcept
    {
      return _width;
    }
    void SetWidth(double width) noexcept
    {
      _width = width;
    }

    KRYS_NODISCARD double ActualBoundingBoxLeft() const noexcept
    {
      return _actualBoundingBoxLeft;
    }
    void SetActualBoundingBoxLeft(double value) noexcept
    {
      _actualBoundingBoxLeft = value;
    }

    KRYS_NODISCARD double ActualBoundingBoxRight() const noexcept
    {
      return _actualBoundingBoxRight;
    }
    void SetActualBoundingBoxRight(double value) noexcept
    {
      _actualBoundingBoxRight = value;
    }

    KRYS_NODISCARD double FontBoundingBoxAscent() const noexcept
    {
      return _fontBoundingBoxAscent;
    }
    void SetFontBoundingBoxAscent(double value) noexcept
    {
      _fontBoundingBoxAscent = value;
    }

    KRYS_NODISCARD double FontBoundingBoxDescent() const noexcept
    {
      return _fontBoundingBoxDescent;
    }
    void SetFontBoundingBoxDescent(double value) noexcept
    {
      _fontBoundingBoxDescent = value;
    }

    KRYS_NODISCARD double ActualBoundingBoxAscent() const noexcept
    {
      return _actualBoundingBoxAscent;
    }
    void SetActualBoundingBoxAscent(double value) noexcept
    {
      _actualBoundingBoxAscent = value;
    }

    KRYS_NODISCARD double ActualBoundingBoxDescent() const noexcept
    {
      return _actualBoundingBoxDescent;
    }
    void SetActualBoundingBoxDescent(double value) noexcept
    {
      _actualBoundingBoxDescent = value;
    }

    KRYS_NODISCARD double EmHeightAscent() const noexcept
    {
      return _emHeightAscent;
    }
    void SetEmHeightAscent(double value) noexcept
    {
      _emHeightAscent = value;
    }

    KRYS_NODISCARD double EmHeightDescent() const noexcept
    {
      return _emHeightDescent;
    }
    void SetEmHeightDescent(double value) noexcept
    {
      _emHeightDescent = value;
    }

    KRYS_NODISCARD double HangingBaseline() const noexcept
    {
      return _hangingBaseline;
    }
    void SetHangingBaseline(double value) noexcept
    {
      _hangingBaseline = value;
    }

    KRYS_NODISCARD double AlphabeticBaseline() const noexcept
    {
      return _alphabeticBaseline;
    }
    void SetAlphabeticBaseline(double value) noexcept
    {
      _alphabeticBaseline = value;
    }

    KRYS_NODISCARD double IdeographicBaseline() const noexcept
    {
      return _ideographicBaseline;
    }
    void SetIdeographicBaseline(double value) noexcept
    {
      _ideographicBaseline = value;
    }

  private:
    TextMetrics() noexcept = default;
  };
}
